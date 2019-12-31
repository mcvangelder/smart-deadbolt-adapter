#ifndef AdapterOrchestrator_H
#define AdapterOrchestrator_H

#include <EEPROM.h>
#include <nfc-mifarereader-i2c.h>
#include <statemachine.h>

const uint8_t MAX_UID_BYTES = 7;


class AdapterOrchestrator : public virtual StateChangedListener
{
public:
    // These enum values correlate 1 to 1 to indexes in eventHandlers
    enum AdapterStates : byte
    {
        INITIALIZING,
        LOCK_DOOR,
        DOOR_LOCKED,
        UNLOCK_DOOR,
        DOOR_UNLOCKED
    };

    AdapterOrchestrator(NFCMiFareReader *nfcReader);
    AdapterOrchestrator() {}
    void initialize(
        void (*initializationHandler)(),
        void (*lockDoorHandler)(),
        void (*doorLockedHandler)(),
        void (*unlockDoorHandler)(),
        void (*doorUnlockedHandler)()
    );
    void run();
    void goToState(AdapterOrchestrator::AdapterStates nextState);
    bool readNextCard();

private:
    StateMachine m_stateMachine;
    NFCMiFareReader *m_nfcReader;
    uint8_t validUID[MAX_UID_BYTES] = {0, 0, 0, 0, 0, 0, 0};
    ReadStatus readStatus;
    // These indexes correlate 1 to 1 to the AdapterStates enum values
    void (*eventHandlers[6])() = {};

    void onStateChanged(StateData *oldState, StateData *newState);
    void initializeStateMachine();
    bool isSavedUID(uint8_t *uid, uint8_t uidLength);

    static void printHex(uint8_t *values, uint8_t length);
};
#endif