#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertising.h>

#include <Wire.h>
#include <MPU6050.h>

// ================= CONFIG =================
#define ZONE_NAME "ZONE_A"

const int RSSI_THRESHOLD = -80;
const int ENTRY_DELAY = 3000;
const int EXIT_DELAY = 5000;

#define SAMPLE_SIZE 15
const int VAR_THRESHOLD = 200;

const unsigned long NO_MOTION_EXIT = 180000;
const unsigned long SCAN_INTERVAL = 4000;

// ================= OBJECTS =================
BLEScan* pBLEScan;
BLEAdvertising *pAdvertising;

MPU6050 mpu;

// ================= STATE =================
bool inside = false;
bool zoneDetected = false;

unsigned long lastSeen = 0;
unsigned long stableStart = 0;
unsigned long entryTime = 0;
unsigned long lastMotionTime = 0;
unsigned long lastScanTime = 0;

// MPU buffers
int axBuf[SAMPLE_SIZE];
int ayBuf[SAMPLE_SIZE];
int azBuf[SAMPLE_SIZE];

int idx = 0;

// ================= VARIANCE =================
int calcVar(int arr[]) {

  int minV = arr[0];
  int maxV = arr[0];

  for (int i = 1; i < SAMPLE_SIZE; i++) {

    if (arr[i] < minV) minV = arr[i];
    if (arr[i] > maxV) maxV = arr[i];
  }

  return maxV - minV;
}

bool isHuman() {

  int v =
    calcVar(axBuf) +
    calcVar(ayBuf) +
    calcVar(azBuf);

  return v > VAR_THRESHOLD;
}

// ================= MPU =================
void updateMPU() {

  int16_t ax, ay, az;

  mpu.getAcceleration(&ax, &ay, &az);

  axBuf[idx] = ax;
  ayBuf[idx] = ay;
  azBuf[idx] = az;

  idx = (idx + 1) % SAMPLE_SIZE;

  if (isHuman()) {
    lastMotionTime = millis();
  }
}

// ================= ADVERTISE =================
void advertiseState(String stateName) {

  pAdvertising->stop();

  BLEAdvertisementData advData;

  advData.setName(stateName);

  pAdvertising->setAdvertisementData(advData);

  pAdvertising->start();

  Serial.print("Advertising: ");
  Serial.println(stateName);
}

// ================= BLE =================
void scanBLE() {

  bool foundNow = false;

  BLEScanResults* results =
      pBLEScan->start(2, false);

  for (int i = 0; i < results->getCount(); i++) {

    BLEAdvertisedDevice d =
        results->getDevice(i);

    String name = d.getName().c_str();

    if (name == ZONE_NAME) {

      int rssi = d.getRSSI();

      Serial.print("ZONE RSSI: ");
      Serial.println(rssi);

      if (rssi > RSSI_THRESHOLD) {

        foundNow = true;

        lastSeen = millis();
      }
    }
  }

  pBLEScan->clearResults();

  // persistent state logic
  if (foundNow) {

    zoneDetected = true;
  }

  else {

    if (millis() - lastSeen > EXIT_DELAY) {

      zoneDetected = false;
    }
  }
}

// ================= ENTRY =================
void handleEntry() {

  if (zoneDetected && isHuman()) {

    if (!inside) {

      if (stableStart == 0)
        stableStart = millis();

      if (millis() - stableStart > ENTRY_DELAY) {

        inside = true;

        entryTime = millis();

        Serial.println("\n=== ENTRY ===");

        advertiseState("VISHAL_IN");
      }
    }
  }

  else {

    if (!inside)
      stableStart = 0;
  }
}

// ================= EXIT =================
void handleExit() {

  // Zone lost
  if (inside &&
      millis() - lastSeen > EXIT_DELAY) {

    inside = false;

    unsigned long dur =
        (millis() - entryTime) / 1000;

    Serial.println("\n=== EXIT ===");

    Serial.print("Duration: ");
    Serial.println(dur);

    advertiseState("VISHAL_OUT");
  }

  // No human
  if (inside &&
      millis() - lastMotionTime >
      NO_MOTION_EXIT) {

    inside = false;

    Serial.println("\n=== EXIT (NO HUMAN) ===");

    advertiseState("VISHAL_OUT");
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  delay(2000);

  // MPU
  Wire.begin(21, 22);

  Wire.setClock(100000);

  mpu.initialize();

  mpu.setSleepEnabled(false);

  Serial.println("MPU READY");

  // BLE
  BLEDevice::init("");

  // SCAN
  pBLEScan = BLEDevice::getScan();

  pBLEScan->setActiveScan(true);

  // ADVERTISE
  pAdvertising =
      BLEDevice::getAdvertising();

  advertiseState("VISHAL_OUT");

  lastMotionTime = millis();

  Serial.println("ATAG ENGINE READY");
}

// ================= LOOP =================
void loop() {

  // MPU always running
  updateMPU();

  // BLE scan
  if (millis() - lastScanTime >
      SCAN_INTERVAL) {

    lastScanTime = millis();

    scanBLE();
  }

  // Presence logic
  handleEntry();

  handleExit();

  delay(300);
}
