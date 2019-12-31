#include "AdapterOrchestrator.h"

#define BUTTON_PIN (8)
#define BUZZER_PIN (6)
#define UNLOCKED_LED (7)
#define LOCKED_LED LED_BUILTIN

auto nfcReader = NFCMiFareReaderI2C();
auto orchestrator = AdapterOrchestrator(&nfcReader);

void setup()
{
#ifndef ESP8266
    while (!Serial)
        ; // for Leonardo/Micro/Zero
#endif
    Serial.begin(115200);
    Serial.println("Hello!");

    pinMode(LOCKED_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(UNLOCKED_LED, OUTPUT);

    digitalWrite(UNLOCKED_LED, HIGH);
    orchestrator.initialize(&initializationHandler, &lockDoorHandler, &doorLockedHandler, &readCardHandler, &unlockDoorHandler, &doorUnlockedHandler);
    digitalWrite(LOCKED_LED, HIGH);
    delay(500);
}

void loop()
{
    orchestrator.run();
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
    orchestrator.goToState(AdapterOrchestrator::AdapterStates::READ_CARD);
}

void readCardHandler(){
    auto success = orchestrator.readNextCard();
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
    // for prototyping purposes, delaying 1 sec, then automatically resetting back to the locked state
    // final device implementation will require manual deadbolt manipulation (or button press to actuate the deadbolt)
    delay(1000);
    orchestrator.goToState(AdapterOrchestrator::AdapterStates::LOCK_DOOR);
}
