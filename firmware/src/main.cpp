#include <Arduino.h>
#include <Wire.h>

void setup() {
  Serial.begin(115200);
  delay(500);

  // I2C defaults for ESP32 are usually SDA=21, SCL=22
  Wire.begin(21, 22);

  Serial.println("\nI2C Scanner");
}

void loop() {
  byte error, address;
  int found = 0;

  Serial.println("Scanning...");

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C devices found.");
    Serial.println("Check: power (3V3/GND) + SDA(21) + SCL(22).");
  } else {
    Serial.print("Done. Devices found: ");
    Serial.println(found);
  }

  Serial.println();
  delay(2000);
}