#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <BLEUUID.h>

#define NAME "POWRLINK ZERO MOCK1"
#define DEV_INFO_UUID "0000180a-0000-1000-8000-00805f9b34fb"
#define MFG_NAME_STR "00002a29-0000-1000-8000-00805f9b34fb"
#define SER_NUM_STR "00002a25-0000-1000-8000-00805f9b34fb"
#define HW_REV_STR "00002a27-0000-1000-8000-00805f9b34fb"
#define FW_REV_STR "00002a26-0000-1000-8000-00805f9b34fb"
#define BATT_SVC_UUID "0000180f-0000-1000-8000-00805f9b34fb"
#define BATT_LVL "00002a19-0000-1000-8000-00805f9b34fb"

// Heart Rate Service
#define HR_SVC_UUID "0000180d-0000-1000-8000-00805f9b34fb"
#define HR_MEASURE "00002a37-0000-1000-8000-00805f9b34fb"
#define BODY_SENSOR_LOC "00002a38-0000-1000-8000-00805f9b34fb"

// Cycling Power Service
#define CPS_UUID "00001818-0000-1000-8000-00805f9b34fb"
#define CPS_MEASURE "00002A63-0000-1000-8000-00805f9b34fb"
#define CPS_FEATURE "00002A65-0000-1000-8000-00805f9b34fb"
#define CPS_LOCATION "00002A5D-0000-1000-8000-00805f9b34fb"
#define CPS_CTRL "00002A66-0000-1000-8000-00805f9b34fb"

BLEServer *pServer = NULL;
BLECharacteristic *batt_level;
BLECharacteristic *body_sensor_loc;
BLECharacteristic *hr_measure;
BLECharacteristic *mfg;
BLECharacteristic *sns;
BLECharacteristic *hrs;
BLECharacteristic *frs;
// CPS
BLECharacteristic *cps_sensor_loc;
BLECharacteristic *cps_feature;
BLECharacteristic *cps_measure;

bool deviceConnected = false;

uint8_t hr[] = { 0x16, 0x63, 0x83, 0x02 };
uint8_t batt = 0x64;
uint8_t sensor_loc = 0x01;
uint8_t mfg_name[] = { 0x57, 0x61, 0x68, 0x6f, 0x6f, 0x20, 0x46, 0x69, 0x74, 0x6e, 0x65, 0x73, 0x73 }; // Wahoo Fitness
uint8_t ser_str[] = { 0x32, 0x30, 0x30, 0x36, 0x30, 0x30, 0x31, 0x39, 0x36 };
uint8_t hw_str[] = { 0x32, 0x31 };
uint8_t fw_str[] = { 0x31, 0x2e, 0x39, 0x2e, 0x32 };
uint8_t cps_measure_str[] = { 0x2F, 0x00, 0x00, 0x00, 0x00, 0x7E, 0x00, 0x12, 0x00, 0x84, 0x27 };
uint16_t appearance = 0x0340;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
      deviceConnected = true;
      Serial.println("Client connected");
    };

    void onDisconnect(BLEServer *pServer) {
      deviceConnected = false;
      Serial.println("Client disconnected");
      BLEDevice::startAdvertising();
    }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init(NAME);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  
  // Create the BLE Services
  //BLEService *pService = pServer->createService(HR_SVC_UUID);
  BLEService *pService = pServer->createService(CPS_UUID);
  BLEService *pService2 = pServer->createService(BATT_SVC_UUID);
  BLEService *pService3 = pServer->createService(DEV_INFO_UUID);
  
  // Battery Level Service Chars
  batt_level = pService2->createCharacteristic(
                    BATT_LVL,
                    BLECharacteristic::PROPERTY_READ |
                    BLECharacteristic::PROPERTY_NOTIFY
                  );
  batt_level->setValue((uint8_t*)&batt, 1);  
  batt_level->addDescriptor(new BLE2902());

//   // Heart Rate Service Chars
//   body_sensor_loc = pService->createCharacteristic(BODY_SENSOR_LOC, BLECharacteristic::PROPERTY_READ);
//   body_sensor_loc->setValue((uint8_t*)&sensor_loc, 1);              
//   hr_measure = pService->createCharacteristic(HR_MEASURE, BLECharacteristic::PROPERTY_NOTIFY);               
//   hr_measure->addDescriptor(new BLE2902());

  // Cycling Power Service Chars
  cps_sensor_loc = pService->createCharacteristic(CPS_LOCATION, BLECharacteristic::PROPERTY_READ);
  cps_feature = pService->createCharacteristic(CPS_FEATURE, BLECharacteristic::PROPERTY_READ);
  cps_measure = pService->createCharacteristic(CPS_MEASURE, BLECharacteristic::PROPERTY_NOTIFY);               
  cps_measure->addDescriptor(new BLE2902());

  // Device Info Chars
  mfg = pService3->createCharacteristic(MFG_NAME_STR, BLECharacteristic::PROPERTY_READ);
  mfg->setValue((uint8_t*)&mfg_name, 13);
  sns = pService3->createCharacteristic(SER_NUM_STR, BLECharacteristic::PROPERTY_READ);
  sns->setValue((uint8_t*)&ser_str, 9);
  hrs = pService3->createCharacteristic(HW_REV_STR, BLECharacteristic::PROPERTY_READ);
  hrs->setValue((uint8_t*)&hw_str, 2);
  frs = pService3->createCharacteristic(FW_REV_STR, BLECharacteristic::PROPERTY_READ);
  frs->setValue((uint8_t*)&fw_str, 5);

  // Start the services
  pService2->start();
  pService3->start();
  pService->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  oAdvertisementData.setAppearance(appearance);
  oAdvertisementData.setFlags(0x06);
  pAdvertising->addServiceUUID(CPS_UUID);
  //pAdvertising->addServiceUUID(HR_SVC_UUID);
  pAdvertising->addServiceUUID(BATT_SVC_UUID);
  pAdvertising->addServiceUUID(DEV_INFO_UUID);
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    while(1) {
        cps_measure->setValue((uint8_t*)&cps_measure_str, 12);
        cps_measure->notify();
        delay(1000);
    }
  }
}