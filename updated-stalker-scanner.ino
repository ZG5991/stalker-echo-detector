#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// This script was made for the LOLIN32 (v1) board, not the newer ones. Modification may be required for different ESP32 boards.
// It has been modified from source to fix the breaking API changes that have been published in later ESP32 SDKs since original release.

TaskHandle_t Task1;

const int beepPin = 0;
const int flashPin = 12;
const int powerLEDPin = 18;  

int beepFlashDelay = 0;

bool failedScan = true;
int missed = 0;
String knownBLEAddresses[] = {"", "", "", ""};
const int RSSI_THRESHOLD = -70;
bool device_found;
const int scanTime = 1; //In seconds
int numberOfDevices = 0;
int rssi = 0;
int lowestRssi = -1000;
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      for (int i = 0; i < (sizeof(knownBLEAddresses) / sizeof(knownBLEAddresses[0])); i++)
      {
        //Uncomment to Enable Debug Information
        //Serial.println("*************Start**************");
        //Serial.println(sizeof(knownBLEAddresses));
        //Serial.println(sizeof(knownBLEAddresses[0]));
        //Serial.println(sizeof(knownBLEAddresses)/sizeof(knownBLEAddresses[0]));
        //Serial.println(advertisedDevice.getAddress().toString().c_str());
        //Serial.println(knownBLEAddresses[i].c_str());
        //Serial.println("*************End**************");
        if (strcmp(advertisedDevice.getAddress().toString().c_str(), knownBLEAddresses[i].c_str()) == 0)
                        {
          device_found = true;
                          break;
                        }
        else
          device_found = false;
      }
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};


void beepFlash( void * parameter) {
  for(;;) {
    if (failedScan == false) {
      analogWrite(beepPin, 20);
      // digitalWrite(LED_BUILTIN, HIGH); // Uncomment this line to enable the built-in LED to blink.
      digitalWrite(flashPin, HIGH);
      delay(100);
      analogWrite(beepPin, 0);
      //digitalWrite(LED_BUILTIN, LOW);
      digitalWrite(flashPin, LOW);
      delay(beepFlashDelay);
    } else {
//      Serial.println("no device found");
      delay(50);
    }
  }
}


void setup() {
  xTaskCreatePinnedToCore(
      beepFlash, /* Function to implement the task */
      "Task1", /* Name of the task */
      10000,  /* Stack size in words */
      NULL,  /* Task input parameter */
      0,  /* Priority of the task */
      &Task1,  /* Task handle. */
      1); /* Core where the task should run */


  Serial.begin(115200); //Enable UART on ESP32
  Serial.println("Scanning..."); // Print Scanning
  
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(beepPin, OUTPUT);
  pinMode(flashPin, OUTPUT);
  pinMode(powerLEDPin, OUTPUT);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Init Callback Function
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100); // set Scan interval
  pBLEScan->setWindow(99);  // less or equal setInterval value
}
void loop() {
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(powerLEDPin, HIGH);
  lowestRssi = -1000;
  BLEScanResults *foundDevices = pBLEScan->start(scanTime, false);
  
  numberOfDevices = foundDevices->getCount();
//  Serial.println(numberOfDevices);
  
  for (int i = 0; i < numberOfDevices; i++) {
    BLEAdvertisedDevice device = foundDevices->getDevice(i);
    rssi = device.getRSSI();
    Serial.print("RSSI: ");
    Serial.println(rssi);
    if (rssi > lowestRssi) {
      lowestRssi = rssi;
    }
  }
//  Serial.println(lowestRssi);
  if (numberOfDevices < 1) {
    if (missed > 2) {
      failedScan = true;
      Serial.println("scan failed");
    } else {
      missed++;
    }
    Serial.print("missed: ");
    Serial.println(missed);
  } else {
    beepFlashDelay = (((-(lowestRssi))-40)*5);
    missed = 0;
    failedScan = false;
  }
//  Serial.println(numberOfDevices);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
}