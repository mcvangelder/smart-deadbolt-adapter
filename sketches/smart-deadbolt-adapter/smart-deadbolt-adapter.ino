#include "AdapterOrchestrator.h"

auto nfcReader = NFCMiFareClassicSpi();
auto machine = StateMachine();
auto orchestrator = AdapterOrchestrator(&machine, &nfcReader);

void setup()
{
    pinMode(LOCKED_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(UNLOCKED_LED, OUTPUT);

    orchestrator.initialize();
}

void loop()
{
    orchestrator.run();
}