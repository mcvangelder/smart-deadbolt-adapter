#ifndef AdapterOrchestrator_H
#define AdapterOrchestrator_H

#include <EEPROM.h>
#include <nfc-mifareclassic-spi.h>
#include <statemachine.h>

const uint8_t INITIALIZING = 0;
const uint8_t LOCK_DOOR = 1;
const uint8_t DOOR_LOCKED = 2;
const uint8_t UNLOCK_DOOR = 3;
const uint8_t DOOR_UNLOCKED = 4;

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
    uint8_t storedValue[MAX_UID_BYTES] = {0, 0, 0, 0, 0, 0, 0};

    void initializeStateMachine();
    ReadStatus readCard();
    bool isSavedUID(uint8_t *uid, uint8_t uidLength);

    static void onStateChanged(StateData *oldState, StateData *newState);
    static void printHex(uint8_t *values, uint8_t length);
};
#endif