#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

static const int SCREEN_WIDTH = 128;
static const int SCREEN_HEIGHT = 64;

// Most 0.96" SSD1306 I2C OLEDs are 0x3C (yours scanned as 0x3C)
static const uint8_t OLED_ADDR = 0x3C;

// No reset pin used for many I2C modules (-1 means "not connected")
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(115200);
  delay(200);

  // Explicit I2C pins for ESP32 (matches your wiring)
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: SSD1306 init failed. Check wiring + address.");
    while (true) { delay(1000); }
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("HELLO!");
  display.setTextSize(1);
  display.println();
  display.println("ESP32 + OLED");
  display.display();

  Serial.println("OLED initialized and wrote HELLO!");
}

void loop() {
  static uint32_t last = 0;
  static int counter = 0;

  if (millis() - last >= 1000) {
    last = millis();
    counter++;

    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("HELLO!");
    display.setTextSize(1);
    display.println();
    display.print("Count: ");
    display.println(counter);
    display.display();

    Serial.print("Count: ");
    Serial.println(counter);
  }
}
