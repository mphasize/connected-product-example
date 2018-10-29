/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristicRx = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

String value = "Short";

long longPressTime = 1200;
const int button1Pin = 0;
long button1Timer = 0;
boolean button1Active = false;
boolean longPress1Active = false;


// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_RX   "6E400002-B5A3-F393-E0A9-E50E24DCCA9E" // Receive data


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristicRead) {
      //Serial.println("BLE: received data");
      std::string rxValue = pCharacteristicRead->getValue();

      Serial.print("Received Value: ");
      for (int i = 0; i < rxValue.length(); i++)
        Serial.print(rxValue[i]);
      Serial.println();
    }
};


void setup() {
  Serial.begin(115200);
  pinMode(button1Pin, INPUT_PULLUP);


  // Create the BLE Device
  BLEDevice::init("MarcusBLE");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());


  // Create RX (Incoming Data) BLE Characteristic
  pCharacteristicRx = pService->createCharacteristic(
                        CHARACTERISTIC_RX,
                        BLECharacteristic::PROPERTY_WRITE
                      );
  pCharacteristicRx->setCallbacks(new MyCallbacks());
  

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  // notify changed value
  handleButtons();

  // disconnecting
  if (!deviceConnected && oldDeviceConnected) {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    Serial.println("start advertising");
    oldDeviceConnected = deviceConnected;
  }

  // connecting
  if (deviceConnected && !oldDeviceConnected) {
    // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
  }

  delay(10);
}

void onButton1Press() {
  Serial.println("BTN 1 press");
  if (deviceConnected) {
    value = "Short";
    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();
  }
}

void onButton1LongPress() {
  Serial.println("BTN 1 long press");
  if (deviceConnected) {
    value = "Long";
    pCharacteristic->setValue(value.c_str());
    pCharacteristic->notify();
  }
}

void handleButtons() {
  if (digitalRead(button1Pin) == LOW) {
    if (button1Active == false) {
      button1Active = true;
      button1Timer = millis();
    }
    if ((millis() - button1Timer > longPressTime) && (longPress1Active == false)) {
      longPress1Active = true;
      onButton1LongPress();
    }
  } else {
    if (button1Active == true) {
      if (longPress1Active == true) {
        longPress1Active = false;
      } else {
        onButton1Press();
      }
      button1Active = false;
    }
  }
}
