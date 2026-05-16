#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEAdvertising.h>
#include <BLEScan.h>

// ================= OBJECTS =================
BLEScan* pBLEScan;

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  BLEDevice::init("ZONE_A");

  // ===== ZONE ADVERTISEMENT =====
  BLEAdvertising *pAdvertising =
      BLEDevice::getAdvertising();

  BLEAdvertisementData advData;

  advData.setName("ZONE_A");

  pAdvertising->setAdvertisementData(advData);

  pAdvertising->setScanResponse(false);

  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  pAdvertising->start();

  // ===== BLE SCAN =====
  pBLEScan = BLEDevice::getScan();

  pBLEScan->setActiveScan(true);

  Serial.println("ZONE BEACON READY");
}

// ================= LOOP =================
void loop() {

  BLEScanResults* results =
      pBLEScan->start(2, false);

  for (int i = 0; i < results->getCount(); i++) {

    BLEAdvertisedDevice d =
        results->getDevice(i);

    String name = d.getName().c_str();

    // ===== ENTRY =====
    if (name == "VISHAL_IN") {

      Serial.println("State:IN");
    }

    // ===== EXIT =====
    else if (name == "VISHAL_OUT") {

      Serial.println("State:OUT");
    }
  }

  pBLEScan->clearResults();

  delay(500);
}
