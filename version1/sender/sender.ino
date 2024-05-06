#include "BluetoothSerial.h"
#define motion_sensor 19

int motion;

BluetoothSerial SerialBT;

String MACadd = "78:21:84:87:AF:9E"; // Write Drone side MAC address
uint8_t address[6] = {0x78, 0x21, 0x84, 0x87, 0xAF, 0x9E}; // Write Drone side MAC address in HEX
bool connected;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test", true);
  Serial.println("The device started in master mode, make sure remote BT device is on!");

  connected = SerialBT.connect(address);

  if (connected) {
    Serial.println("Connected Successfully!");
  } else {
    while (!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure the remote device is available and in range, then restart the app.");
    }
  }

  pinMode(motion_sensor, INPUT);
}

uint8_t calculate_checksum(uint8_t *data) {
  uint8_t checksum = 0;
  for (int i = 1; i < 3; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

void loop() {
  uint8_t send_data[3];

  motion = digitalRead(motion_sensor);
  
  // Print PIR motion sensor value
  Serial.print("Motion value: ");
  Serial.println(motion);
  
  send_data[0] = 'T';
  send_data[1] = motion;
  send_data[2] = calculate_checksum(send_data);
  
  // Print the data being sent via Bluetooth
  Serial.print("Sending data: ");
  Serial.print(send_data[0]);
  Serial.print(" ");
  Serial.print(send_data[1]);
  Serial.print(" ");
  Serial.println(send_data[2]);
  
  SerialBT.write(send_data, 3);

  delay(20);
}
