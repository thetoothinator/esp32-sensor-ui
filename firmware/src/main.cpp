#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>

static const int SCREEN_WIDTH = 128;
static const int SCREEN_HEIGHT = 64;
static const uint8_t OLED_ADDR = 0x3C;   // from your I2C scanner

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

void setup() {
  Serial.begin(115200);
  delay(200);

  // Use your wired I2C pins
  Wire.begin(21, 22);

  // OLED init
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: OLED init failed (check address/wiring).");
    while (true) { delay(1000); }
  }

  // SHT31 init (your scanner showed it, most likely 0x44)
  if (!sht31.begin(0x44)) {
    Serial.println("ERROR: SHT31 init failed at 0x44.");
    Serial.println("If your scanner showed 0x45, change 0x44 to 0x45.");
    while (true) { delay(1000); }
  }

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("SHT31 + OLED OK");
  display.display();

  Serial.println("Setup complete.");
}

void loop() {
  float tC = sht31.readTemperature();
  float h = sht31.readHumidity();

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 0);

  if (isnan(tC) || isnan(h)) {
    display.println("Read");
    display.println("FAIL");
    Serial.println("Failed to read SHT31.");
  } else {
    // OLED
    display.print(tC, 1);
    display.println(" C");

    display.setTextSize(2);
    display.print(h, 1);
    display.println(" %");

    // Serial
    Serial.print("Temp: ");
    Serial.print(tC, 1);
    Serial.print(" C  Hum: ");
    Serial.print(h, 1);
    Serial.println(" %");
  }

  display.display();
  delay(1000);
}