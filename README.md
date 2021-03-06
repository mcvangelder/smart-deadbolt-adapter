# Smart Deadbolt Adapter
Smart Deadbolt Adapter is a piece of hardware which fits over a standard deadbolt enabling NFC/RFID 13.56MHz keyless access. You can program the device to accept any NFC/RFID 13.56MHz tag.

The device is built on Arduino platform and uses the [Adafruit PN532 breakout board](https://learn.adafruit.com/adafruit-pn532-rfid-nfc) to read your NFC card/tag.


The software uses a state machine to control what actions can happen when. The diagram below describes the states and their valid transitions.

![State Machine Diagram](documentation/AdapterStateMachine.png "State Machine Diagram")

***
Dependencies |
------------ |
[NFC MiFare Reader](https://github.com/mcvangelder/arduino-libraries/tree/develop/NFC-MiFareReader)|
[State Machine](https://github.com/mcvangelder/arduino-libraries/tree/develop/StateMachine)|

[Installing libraries for Arduino](http://www.arduino.cc/en/Guide/Libraries)

_Note:_
This is a work in progress, feel free to follow the project to get updates on its progress.
