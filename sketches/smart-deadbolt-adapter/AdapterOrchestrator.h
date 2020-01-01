#ifndef AdapterOrchestrator_H
#define AdapterOrchestrator_H

#include <EEPROM.h>
#include <nfc-mifarereader-i2c.h>
#include <statemachine.h>

#define ADPT_ORCSTR_DEBUG

const uint8_t MAX_UID_BYTES = 7;

const uint8_t NUM_ADAPTER_STATES = 7;

class AdapterOrchestrator : public virtual StateChangedListener
{
public:
    enum AdapterStates : byte
    {
        INITIALIZING,
        LOCK_DOOR,
        DOOR_LOCKED,
        READ_CARD,
        UNLOCK_DOOR,
        DOOR_UNLOCKED,
        UNSET = 255
    };

    AdapterOrchestrator(NFCMiFareReader *nfcReader);
    AdapterOrchestrator() {}
    void initialize(
        void (*initializationHandler)(),
        void (*lockDoorHandler)(),
        void (*doorLockedHandler)(),
        void (*readCardHandler)(),
        void (*unlockDoorHandler)(),
        void (*doorUnlockedHandler)()
    );
    void goToState(AdapterOrchestrator::AdapterStates nextState);
    bool readCard();
    void activateCardReader(void (*cardReadHandler)());
    void cardDetected();

private:
    StateMachine m_stateMachine;
    NFCMiFareReader *m_nfcReader;
    uint8_t validUID[MAX_UID_BYTES] = {0, 0, 0, 0, 0, 0, 0};
    ReadStatus readStatus;
    // These indexes correlate 1 to 1 to the AdapterStates enum values
    void (*eventHandlers[NUM_ADAPTER_STATES])() = {};

    void onStateChanged(StateData *oldState, StateData *newState);
    void initializeStateMachine();
    bool isSavedUID(uint8_t *uid, uint8_t uidLength);

    static void printHex(uint8_t *values, uint8_t length);
};
#endif