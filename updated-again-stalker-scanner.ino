#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

TaskHandle_t Task1;

const int beepPin = 0;
const int flashPin = 12;
const int powerLEDPin = 18;

int beepFlashDelay = 1000; // Default delay0
bool device_found = false;
int rssi = -100;
const int RSSI_THRESHOLD = -100; // Threshold for proximity
const int scanTime = 1; // Scan duration in seconds
BLEScan* pBLEScan;

String knownBLEAddresses[] = {"", ""};

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks { //Could use logic for filtering out any found device with lower than highest RSSI for the use case of having 2 BLE devices within the threshold proximity.
    void onResult(BLEAdvertisedDevice advertisedDevice) override {
        for (const String& addr : knownBLEAddresses) {
            if (strcmp(advertisedDevice.getAddress().toString().c_str(), addr.c_str()) == 0) {
                rssi = advertisedDevice.getRSSI();
                device_found = rssi > RSSI_THRESHOLD;
                Serial.printf("Known Device Found: %s, RSSI: %d\n", addr.c_str(), rssi);
                return;
            }
        }
    }
};

void beepFlash(void* parameter) {
    for (;;) {
        if (device_found) {
            analogWrite(beepPin, 20);
            digitalWrite(flashPin, HIGH);
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            analogWrite(beepPin, 0);
            digitalWrite(flashPin, LOW);
            digitalWrite(LED_BUILTIN, LOW);
            vTaskDelay(beepFlashDelay / portTICK_PERIOD_MS); // Use FreeRTOS delay
        } else {
            vTaskDelay(100 / portTICK_PERIOD_MS); // Wait briefly if no device is found
        }
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Scanning...");

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(beepPin, OUTPUT);
    pinMode(flashPin, OUTPUT);
    pinMode(powerLEDPin, OUTPUT);

    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true);
    pBLEScan->setInterval(100);
    pBLEScan->setWindow(99);

    xTaskCreatePinnedToCore(beepFlash, "Beep Flash Task", 4096, NULL, 1, &Task1, 1);
}

void loop() {
    digitalWrite(powerLEDPin, LOW);

    BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);

    Serial.printf("Number of devices: %d\n", foundDevices->getCount());

    if (!device_found) {
        beepFlashDelay = 1000; // Default delay if no known device is found
        Serial.println("No known device found.");
    } else {
        beepFlashDelay = map(constrain(rssi, RSSI_THRESHOLD, 0), RSSI_THRESHOLD, 0, 200, 0.5); //Determines beepFlashDelay based on proximity to detected devices. 
        //Changing the last 2 values will affect the frequency when within range.
        Serial.printf("Beep Flash Delay: %d ms (RSSI: %d)\n", beepFlashDelay, rssi);
    }

    pBLEScan->clearResults(); // Clear results for the next scan
    delay(100); // Small delay between scans
}