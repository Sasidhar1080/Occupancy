#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

// MAC address of the target BLE device (Raspberry Pi)
#define TARGET_DEVICE_MAC "68:27:19:A8:11:92" // Replace with the actual MAC address

bool deviceFound = false;
BLEAdvertisedDevice* targetDevice = nullptr;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().equals(BLEAddress(TARGET_DEVICE_MAC))) {
      deviceFound = true;
      targetDevice = new BLEAdvertisedDevice(advertisedDevice);
    }
  }
};

void setup() {
  Serial.begin(115200);
  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30); // Scan for 30 seconds or adjust as needed
}

void loop() {
  if (deviceFound) {
    // Connect to the target device
    BLEClient* pClient = BLEDevice::createClient();
    pClient->connect(targetDevice);

    if (pClient->isConnected()) {
      Serial.println("Connected to the target device!");

      // Perform operations with the BLE device
      // Example: Read data from characteristics

      // For demonstration purposes, let's assume data is received
      String receivedData = "Sample data from device";
      Serial.println("Received data: " + receivedData);
    } else {
      Serial.println("Failed to connect to the target device!");
    }

    // Disconnect when done
    pClient->disconnect();
    deviceFound = false;
  }
  delay(1000); // Delay between connection attempts
}
