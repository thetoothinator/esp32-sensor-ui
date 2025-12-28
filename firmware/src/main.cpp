#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SHT31.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);
  delay(200);

  Wire.begin(21, 22); // SDA, SCL

  if (!sht31.begin(0x44)) { // most common address
    Serial.println("ERROR: SHT31 not found at 0x44.");
    Serial.println("Tip: Run I2C scanner and check address (0x44 or 0x45).");
    while (true) { delay(1000); }
  }

  Serial.println("SHT31 initialized.");
}

void loop() {
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read SHT31.");
  } else {
    Serial.print("Temp: ");
    Serial.print(t);
    Serial.print(" C,  Humidity: ");
    Serial.print(h);
    Serial.println(" %");
  }

  delay(1000);
}