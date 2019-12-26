#include <nfc-mifareclassic-spi.h>
#include <statemachine.h>

NFCMiFareClassicSpi nfcSpi = NFCMiFareClassicSpi();
StateMachine machine;

const uint8_t LOCK_DOOR = 0;
const uint8_t DOOR_LOCKED = 0b001;
const uint8_t UNLOCK_DOOR = 0b010;
const uint8_t DOOR_UNLOCKED = 0b011;
const uint8_t INITIALIZING = 0b100;

#define BUTTON_PIN (8)
#define BUZZER_PIN (6)
#define UNLOCKED_LED (7)
#define LOCKED_LED LED_BUILTIN

void setup()
{
    nfcSpi.initialize();
    initializeStateMachine();
    pinMode(LOCKED_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(UNLOCKED_LED, OUTPUT);
}

void loop()
{
    auto currentState = machine.getCurrentStateValue();
    switch (currentState)
    {
        case INITIALIZING : {
            Serial.println("Initializing... Detecting status of door.");
            machine.transitionTo(DOOR_LOCKED);
            break;
        }
        case DOOR_LOCKED : {
            digitalWrite(LOCKED_LED, HIGH);
            if(readCard())
            {
                machine.transitionTo(UNLOCK_DOOR);
            }
            break;
        }
        case UNLOCK_DOOR : {
            digitalWrite(LOCKED_LED, LOW);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(500);
            machine.transitionTo(DOOR_UNLOCKED);
            break;
        }
        case DOOR_UNLOCKED : {
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(UNLOCKED_LED, HIGH);
            delay(1000);
            machine.transitionTo(LOCK_DOOR);
            break;
        }
        case LOCK_DOOR : {
            digitalWrite(UNLOCKED_LED, LOW);
            machine.transitionTo(DOOR_LOCKED);
            break;
        }
        default:
            Serial.print("Unknown state: ");
            Serial.print(currentState);
            Serial.print(" Name: ");
            Serial.println(machine.getCurrentStateName());
    }
}

void initializeStateMachine()
{
    StateData doorLockedState, unlockDoorState, doorUnlockedState, lockDoorState, initializingState;

    StateData *allStates[] = {
                &(doorLockedState = StateData(DOOR_LOCKED, "LOCKED")),
                &(unlockDoorState = StateData(UNLOCK_DOOR, "UNLOCKING")),
                &(doorUnlockedState = StateData(DOOR_UNLOCKED, "UNLOCKED")),
                &(lockDoorState = StateData(LOCK_DOOR, "LOCKING")),
                &(initializingState = StateData(INITIALIZING, "INITIALIZING"))
              };
    
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

    machine = StateMachine(allStates, 5, initializingState);
    machine.setOnTransitionCallback(onStateChange);
}

void onStateChange(StateData* oldState, StateData* newState) 
{
    Serial.print("Old State: ");
    Serial.println(oldState->getName());

    Serial.print("New State: ");
    Serial.println(newState->getName());
}

bool readCard()
{
    ReadStatus status;
    status = nfcSpi.read();

    Serial.print("[Sandbox] Status (1 success, 0 fail): ");
    Serial.println(status.success);
    if(status.success) {
        Serial.print("Mifare Classic Card Id: ");
        Serial.println(status.cardId);
        return true;
    }
    return false;
}