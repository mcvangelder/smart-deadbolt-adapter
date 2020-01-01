#include "AdapterOrchestrator.h"
#include <statemachine.h>
#include <nfc-mifarereader.h>

AdapterOrchestrator::AdapterOrchestrator(NFCMiFareReader *reader)
{
    m_stateMachine = StateMachine();
    m_nfcReader = reader;

    EEPROM.get(0, validUID);
}

void AdapterOrchestrator::initialize(
    void (*initializationHandler)(),
    void (*lockDoorHandler)(),
    void (*doorLockedHandler)(),
    void (*readCardHandler)(),
    void (*unlockDoorHandler)(),
    void (*doorUnlockedHandler)())
{
    eventHandlers[AdapterOrchestrator::AdapterStates::INITIALIZING] = initializationHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::LOCK_DOOR] = lockDoorHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::DOOR_LOCKED] = doorLockedHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::READ_CARD] = readCardHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::UNLOCK_DOOR] = unlockDoorHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::DOOR_UNLOCKED] = doorUnlockedHandler;

    m_nfcReader->initialize();
    initializeStateMachine();
    Serial.print("Initialization complete. Current State: "); Serial.println(m_stateMachine.getCurrentStateName());
}

void AdapterOrchestrator::run()
{
    if (m_stateMachine.getCurrentStateValue() == AdapterOrchestrator::AdapterStates::INITIALIZING)
    {
        (eventHandlers[AdapterOrchestrator::AdapterStates::INITIALIZING])();
    }
}

void AdapterOrchestrator::goToState(AdapterOrchestrator::AdapterStates nextState)
{
    m_stateMachine.transitionTo(nextState);
}

bool AdapterOrchestrator::readNextCard()
{
    auto success = m_nfcReader->read(readStatus);
    if (success && isSavedUID(readStatus.uidRaw, readStatus.uidLength))
    {
        Serial.println("Access Granted");
        return true;
    }
    else
    {
        Serial.print("Access Denied -->");Serial.println(success ? "Successful Read" : "Failed Read");
    }
    return false;
}

void AdapterOrchestrator::initializeStateMachine()
{
    StateData doorLockedState, unlockDoorState, doorUnlockedState, lockDoorState, readCardState, initializingState;

    StateData *allStates[] = {
        &(doorLockedState = StateData(AdapterStates::DOOR_LOCKED, "LOCKED")),
        &(unlockDoorState = StateData(AdapterStates::UNLOCK_DOOR, "UNLOCKING")),
        &(doorUnlockedState = StateData(AdapterStates::DOOR_UNLOCKED, "UNLOCKED")),
        &(lockDoorState = StateData(AdapterStates::LOCK_DOOR, "LOCKING")),
        &(readCardState = StateData(AdapterStates::READ_CARD, "WAITING ON CARD")),
        &(initializingState = StateData(AdapterStates::INITIALIZING, "INITIALIZING"))};

    StateData *doorLockedStateTransitions[] = {&readCardState};
    doorLockedState.setAllowedTransitions(doorLockedStateTransitions, 1);

    StateData *unlockDoorStateTransistions[] = {&doorUnlockedState};
    unlockDoorState.setAllowedTransitions(unlockDoorStateTransistions, 1);

    StateData *doorUnlockedStateTranistions[] = {&lockDoorState};
    doorUnlockedState.setAllowedTransitions(doorUnlockedStateTranistions, 1);

    StateData *lockDoorStateTransitions[] = {&doorLockedState};
    lockDoorState.setAllowedTransitions(lockDoorStateTransitions, 1);

    StateData *readCardStateTransitions[] = {&doorLockedState, &unlockDoorState};
    readCardState.setAllowedTransitions(readCardStateTransitions, 2);

    StateData *initializingStateTransitions[] = {&doorLockedState, &doorUnlockedState};
    initializingState.setAllowedTransitions(initializingStateTransitions, 2);

    m_stateMachine.initialize(allStates, NUM_ADAPTER_STATES, initializingState);
    m_stateMachine.setOnStateChangedListener(dynamic_cast<StateChangedListener *>(this));
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

void AdapterOrchestrator::onStateChanged(StateData *oldState, StateData *newState)
{
    Serial.print("Old State: ");
    Serial.println(oldState->getName());

    Serial.print("New State: ");
    Serial.println(newState->getName());
    auto evntHandlrIndex = newState->getValue() - 1;
    if (evntHandlrIndex < 7)
    {
        auto eventHandler = eventHandlers[newState->getValue()];
        (*eventHandler)();
    }
    else
    {
        (eventHandlers[0])();
    }
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
