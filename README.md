# stalker-echo-detector
Arduino files for running the Stalker Echo Detector from https://www.printables.com/model/823279-stalker-echo-detector-printable-model

This is still a WIP repo, but I was able to use this code to compile and run the detector for a convention using newer versions of the SDKs required for compilation.

This script was made for the LOLIN32 (v1) board, not the newer ones. Modification may be required for different ESP32 boards, updated to support ESP 3.x.x via https://docs.espressif.com/projects/arduino-esp32/en/latest/migration_guides/2.x_to_3.0.html

Required Libraries:
* https://github.com/espressif/arduino-esp32 (tested with 3.0.3)

Future Updates:
* Adjustable RSSI sensitivity is kind of working in a WIP feature branch I have, but causes the device to crash after 5 minutes. Still working on a fix.