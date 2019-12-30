#include "AdapterOrchestrator.h"
#include <statemachine.h>
#include <nfc-mifareclassic-spi.h>

AdapterOrchestrator::AdapterOrchestrator(StateMachine *machine, NFCMiFareClassicSpi *nfcReader)
{
    m_stateMachine = machine;
    m_nfcReader = nfcReader;

    EEPROM.get(0, validUID);
}

void AdapterOrchestrator::initialize()
{
    m_nfcReader->initialize();
    initializeStateMachine();
}

void AdapterOrchestrator::initializeStateMachine()
{
    StateData *allStates[] = {
        &(doorLockedState = StateData(AdapterStates::DOOR_LOCKED, "LOCKED")),
        &(unlockDoorState = StateData(AdapterStates::UNLOCK_DOOR, "UNLOCKING")),
        &(doorUnlockedState = StateData(AdapterStates::DOOR_UNLOCKED, "UNLOCKED")),
        &(lockDoorState = StateData(AdapterStates::LOCK_DOOR, "LOCKING")),
        &(initializingState = StateData(AdapterStates::INITIALIZING, "INITIALIZING"))};

    StateData *doorLockedStateTransitions[] = {&unlockDoorState};
    doorLockedState.setAllowedTransitions(doorLockedStateTransitions, 1);

    StateData *unlockDoorStateTransistions[] = {&doorUnlockedState};
    unlockDoorState.setAllowedTransitions(unlockDoorStateTransistions, 1);

    StateData *doorUnlockedStateTranistions[] = {&lockDoorState};
    doorUnlockedState.setAllowedTransitions(doorUnlockedStateTranistions, 1);

    StateData *lockDoorStateTransitions[] = {&doorLockedState};
    lockDoorState.setAllowedTransitions(lockDoorStateTransitions, 1);

    StateData *initializingStateTransitions[] = {&doorLockedState, &doorUnlockedState};
    initializingState.setAllowedTransitions(initializingStateTransitions, 2);

    m_stateMachine->initialize(allStates, 5, initializingState);
    m_stateMachine->setOnTransitionCallback(AdapterOrchestrator::onStateChanged);
}

void AdapterOrchestrator::run()
{
    auto currentState = m_stateMachine->getCurrentStateValue();
    switch (currentState)
    {
    case INITIALIZING:
    {
        Serial.println("Initializing... Detecting status of door.");
        m_stateMachine->transitionTo(DOOR_LOCKED);
        break;
    }
    case DOOR_LOCKED:
    {
        digitalWrite(LOCKED_LED, HIGH);
        auto success = m_nfcReader->read(readStatus);
        if (success)
        {
            Serial.println("Read Success!!!");
            digitalWrite(BUZZER_PIN, HIGH);
            if (isSavedUID(readStatus.uidRaw, readStatus.uidLength))
            {
                Serial.println("Access Granted");
                m_stateMachine->transitionTo(UNLOCK_DOOR);
            }
            else
            {
                Serial.println("Access Denied");
            }
            delay(250);
            digitalWrite(BUZZER_PIN, LOW);
        }

        break;
    }
    case UNLOCK_DOOR:
    {
        digitalWrite(LOCKED_LED, LOW);
        delay(500);
        m_stateMachine->transitionTo(DOOR_UNLOCKED);
        break;
    }
    case DOOR_UNLOCKED:
    {
        digitalWrite(UNLOCKED_LED, HIGH);
        delay(1000);
        m_stateMachine->transitionTo(LOCK_DOOR);
        break;
    }
    case LOCK_DOOR:
    {
        digitalWrite(UNLOCKED_LED, LOW);
        m_stateMachine->transitionTo(DOOR_LOCKED);
        break;
    }
    default:
        Serial.print("Unknown state: ");
        Serial.print(currentState);
        Serial.print(" Name: ");
        Serial.println(m_stateMachine->getCurrentStateName());
    }
}

void AdapterOrchestrator::onStateChanged(StateData *oldState, StateData *newState)
{
    Serial.print("Old State: ");
    Serial.println(oldState->getName());

    Serial.print("New State: ");
    Serial.println(newState->getName());
}

// All UIDs are saved in fixed width of MAX_UID_BYTES
bool AdapterOrchestrator::isSavedUID(uint8_t *uid, uint8_t length)
{
    bool isSavedUID = false;

    Serial.print("Comparing : ");
    printHex(uid, length);
    Serial.print("To : ");
    printHex(validUID, MAX_UID_BYTES);

    if (length > 0 && length <= MAX_UID_BYTES)
    {
        auto offset = MAX_UID_BYTES - length;
        auto i = 0;
        auto j = offset;
        isSavedUID = (uid[i++] == validUID[j++]);
        while (j < MAX_UID_BYTES && isSavedUID)
        {
            isSavedUID &= (uid[i++] == validUID[j++]);
        }
    }

    return isSavedUID;
}

void AdapterOrchestrator::printHex(uint8_t *values, uint8_t length)
{
    for (auto i = 0; i < length; i++)
    {
        Serial.print("0x");
        Serial.print(values[i] < 16 ? "0" : "");
        Serial.print(values[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}