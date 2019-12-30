#ifndef AdapterOrchestrator_H
#define AdapterOrchestrator_H

#include <EEPROM.h>
#include <nfc-mifareclassic-spi.h>
#include <statemachine.h>

enum AdapterStates : byte { INITIALIZING, LOCK_DOOR, DOOR_LOCKED, UNLOCK_DOOR, DOOR_UNLOCKED };

const uint8_t MAX_UID_BYTES = 7;

#define BUTTON_PIN (8)
#define BUZZER_PIN (6)
#define UNLOCKED_LED (7)
#define LOCKED_LED LED_BUILTIN

class AdapterOrchestrator
{
public:
    AdapterOrchestrator(StateMachine *stateMachine, NFCMiFareClassicSpi *nfcReader);
    AdapterOrchestrator() {}
    void initialize();
    void run();

private:
    StateMachine *m_stateMachine;
    NFCMiFareClassicSpi *m_nfcReader;
    StateData doorLockedState, unlockDoorState, doorUnlockedState, lockDoorState, initializingState;
    uint8_t validUID[MAX_UID_BYTES] = {0, 0, 0, 0, 0, 0, 0};
    ReadStatus readStatus;

    void initializeStateMachine();
    bool isSavedUID(uint8_t *uid, uint8_t uidLength);

    static void onStateChanged(StateData *oldState, StateData *newState);
    static void printHex(uint8_t *values, uint8_t length);
};
#endif