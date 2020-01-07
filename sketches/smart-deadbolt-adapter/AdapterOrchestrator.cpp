#include "AdapterOrchestrator.h"
#include <statemachine.h>
#include <nfc-mifarereader.h>

AdapterOrchestrator::AdapterOrchestrator(NFCMiFareReader *reader) : m_nfcReader(reader)
{
    m_stateMachine = StateMachine();

    EEPROM.get(0, m_validUID);
}

void AdapterOrchestrator::initialize(
    void (*initializationHandler)(),
    void (*lockDoorHandler)(),
    void (*doorLockedHandler)(),
    void (*readCardHandler)(),
    void (*unlockDoorHandler)(),
    void (*doorUnlockedHandler)(),
    InterruptProxy buttonInterrupt,
    InterruptProxy readerInterrupt)
{
    eventHandlers[AdapterOrchestrator::AdapterStates::INITIALIZING] = initializationHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::LOCK_DOOR] = lockDoorHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::DOOR_LOCKED] = doorLockedHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::READ_CARD] = readCardHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::UNLOCK_DOOR] = unlockDoorHandler;
    eventHandlers[AdapterOrchestrator::AdapterStates::DOOR_UNLOCKED] = doorUnlockedHandler;

    m_nfcReader->initialize();
    initializeStateMachine();

    toggleInterrupt = buttonInterrupt;
    toggleInterrupt.enable();

    cardInterrupt = readerInterrupt;

#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Initialization complete. Current State: ");
    Serial.println(m_stateMachine.getCurrentStateName());
#endif
}

void AdapterOrchestrator::goToState(AdapterOrchestrator::AdapterStates nextState)
{
    auto transitioned = m_stateMachine.transitionTo(nextState);
#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Transitioned to: ");
    Serial.print(nextState);
    Serial.print("? ");
    Serial.println(transitioned ? "true" : "false");
#endif
}

bool AdapterOrchestrator::readCard()
{
    auto success = m_nfcReader->read(m_readStatus);
    bool hasAccess = success && isSavedUID(m_readStatus.uidRaw, m_readStatus.uidLength);
#ifdef ADPT_ORCSTR_DEBUG
    Serial.println(success ? "Successful Read" : "Failed Read");
    Serial.print("Access ");
    Serial.println(hasAccess ? "Granted" : "Declined");
#endif
    return hasAccess;
}

void AdapterOrchestrator::activateCardReader()
{
    cardInterrupt.disable();
    if (m_nfcReader->activateCardReader())
    {
        cardInterrupt.enable();
    }
}

void AdapterOrchestrator::cardDetected()
{
    cardInterrupt.disable();
}

AdapterOrchestrator::AdapterStates AdapterOrchestrator::getNextToggleState()
{
    auto state = m_stateMachine.getCurrentStateValue();
    auto nextState = AdapterStates::UNSET;
#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Current State: ");
    Serial.println(m_stateMachine.getCurrentStateName());
#endif
    switch (state)
    {
    case AdapterStates::DOOR_LOCKED:
        nextState = AdapterStates::UNLOCK_DOOR;
        break;
    case AdapterStates::DOOR_UNLOCKED:
        nextState = AdapterStates::LOCK_DOOR;
        break;
    default:
        break;
    }

#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Next State Value: ");
    Serial.println(nextState);
#endif
    return nextState;
}

void AdapterOrchestrator::initializeStateMachine()
{
    StateData doorLockedState, unlockDoorState, doorUnlockedState, lockDoorState, readCardState, initializingState, unsetState;

    StateData *allStates[] = {
        &(doorLockedState = StateData(AdapterStates::DOOR_LOCKED, "LOCKED")),
        &(unlockDoorState = StateData(AdapterStates::UNLOCK_DOOR, "UNLOCKING")),
        &(doorUnlockedState = StateData(AdapterStates::DOOR_UNLOCKED, "UNLOCKED")),
        &(lockDoorState = StateData(AdapterStates::LOCK_DOOR, "LOCKING")),
        &(readCardState = StateData(AdapterStates::READ_CARD, "WAITING ON CARD")),
        &(initializingState = StateData(AdapterStates::INITIALIZING, "INITIALIZING")),
        &(unsetState = StateData(AdapterStates::UNSET, "UNSET"))};

    StateData *doorLockedStateTransitions[] = {&readCardState, &unlockDoorState};
    doorLockedState.setAllowedTransitions(doorLockedStateTransitions, 2);

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

    StateData *unsetStateTransitions[] = {&initializingState};
    unsetState.setAllowedTransitions(unsetStateTransitions, 1);

    m_stateMachine.initialize(allStates, NUM_ADAPTER_STATES, unsetState);
    m_stateMachine.setOnStateChangedListener(dynamic_cast<StateChangedListener *>(this));
}

// All UIDs are saved in fixed width of MAX_UID_BYTES
bool AdapterOrchestrator::isSavedUID(uint8_t *uid, uint8_t length)
{
    bool isSavedUID = false;

#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Comparing : ");
    printHex(uid, length);
    Serial.print("To : ");
    printHex(m_validUID, MAX_UID_BYTES);
#endif

    if (length > 0 && length <= MAX_UID_BYTES)
    {
        auto offset = MAX_UID_BYTES - length;
        auto i = 0;
        auto j = offset;
        isSavedUID = (uid[i++] == m_validUID[j++]);
        while (j < MAX_UID_BYTES && isSavedUID)
        {
            isSavedUID &= (uid[i++] == m_validUID[j++]);
        }
    }

    return isSavedUID;
}

void AdapterOrchestrator::onStateChanged(StateData *oldState, StateData *newState)
{
#ifdef ADPT_ORCSTR_DEBUG
    Serial.print("Old State: ");
    Serial.print(oldState->getName());
    Serial.print(" (");
    Serial.print(oldState->getValue());
    Serial.println(")");

    Serial.print("New State: ");
    Serial.print(newState->getName());
    Serial.print(" (");
    Serial.print(newState->getValue());
    Serial.println(")");

#endif

    auto evntHandlrIndex = newState->getValue() - 1;
    if (evntHandlrIndex < NUM_ADAPTER_STATES)
    {
        auto eventHandler = eventHandlers[newState->getValue()];
        (*eventHandler)();
    }
    else
    {
        (eventHandlers[AdapterOrchestrator::AdapterStates::INITIALIZING])();
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
