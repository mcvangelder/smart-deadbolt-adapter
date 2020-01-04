#include "AdapterOrchestrator.h"

#define BUZZER_PIN (6)
#define UNLOCKED_LED (7)
#define LOCKED_LED LED_BUILTIN
#define TOGGLE_LOCK_BUTTON (3)
#define IRQ_PIN PN532_IRQ
#define RESET_PIN (5)

auto nfcReader = NFCMiFareReaderI2C(IRQ_PIN, RESET_PIN);
auto orchestrator = AdapterOrchestrator(&nfcReader, digitalPinToInterrupt(IRQ_PIN), digitalPinToInterrupt(TOGGLE_LOCK_BUTTON));
volatile AdapterOrchestrator::AdapterStates nextState = AdapterOrchestrator::AdapterStates::INITIALIZING;

void setup()
{
#ifndef ESP8266
    while (!Serial)
        ; // for Leonardo/Micro/Zero
#endif
    Serial.begin(115200);
    Serial.println("Hello!");

    pinMode(LOCKED_LED, OUTPUT);
    pinMode(TOGGLE_LOCK_BUTTON, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(UNLOCKED_LED, OUTPUT);

    digitalWrite(UNLOCKED_LED, HIGH);
    orchestrator.initialize(&initializationHandler, &lockDoorHandler, &doorLockedHandler, &readCardHandler, &unlockDoorHandler, &doorUnlockedHandler, &toggleButtonHandler);
    digitalWrite(LOCKED_LED, HIGH);
    delay(500);
}

void loop()
{
    if (nextState != AdapterOrchestrator::AdapterStates::UNSET)
    {
        orchestrator.goToState(nextState);
        nextState = AdapterOrchestrator::AdapterStates::UNSET;
    }
}

void initializationHandler()
{
    digitalWrite(UNLOCKED_LED, LOW);
    digitalWrite(LOCKED_LED, LOW);
    // This handler is where to put logic to figure out what the last position (locked or unlocked)
    // of the device was/is to properly initialize the state machine
    // for prototyping purposes, assuming the door is locked.
    orchestrator.goToState(AdapterOrchestrator::AdapterStates::DOOR_LOCKED);
}

void lockDoorHandler()
{
    digitalWrite(UNLOCKED_LED, LOW);
    // for prototyping purposes, delaying .5 sec to simulate actuator locking the door
    delay(500);
    orchestrator.goToState(AdapterOrchestrator::AdapterStates::DOOR_LOCKED);
}

void doorLockedHandler()
{
    digitalWrite(LOCKED_LED, HIGH);
    orchestrator.activateCardReader(cardDetected);
    Serial.println("Waiting for card detection");
}

void cardDetected()
{
    Serial.println("Card Detected");
    nextState = AdapterOrchestrator::AdapterStates::READ_CARD;
    orchestrator.cardDetected();
}

void readCardHandler()
{
    auto success = orchestrator.readCard();
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LOCKED_LED, LOW);
    delay(250);
    digitalWrite(BUZZER_PIN, LOW);
    if (success)
    {
        orchestrator.goToState(AdapterOrchestrator::AdapterStates::UNLOCK_DOOR);
    }
    else
    {
        orchestrator.goToState(AdapterOrchestrator::AdapterStates::DOOR_LOCKED);
    }
}

void unlockDoorHandler()
{
    digitalWrite(LOCKED_LED, LOW);
    // for prototyping purposes, delaying .5 sec to simulate the actuator unlocking the deadbolt
    delay(500);
    orchestrator.goToState(AdapterOrchestrator::AdapterStates::DOOR_UNLOCKED);
}

void doorUnlockedHandler()
{
    digitalWrite(UNLOCKED_LED, HIGH);
}

void toggleButtonHandler()
{
    // Arduino Uno doesn't support detecting "HIGH" mode
    // therefore must check if the interrupt was fired specifically due to being isPressed
    // ignoring the button release event
    auto isPressed = digitalRead(TOGGLE_LOCK_BUTTON);
    if(isPressed)
    {
        nextState = orchestrator.getNextToggleState();
    }
}
