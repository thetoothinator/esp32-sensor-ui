#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>

// ---------- Pins ----------
static const int PIN_BTN_NEXT = 32;   // Button 1
static const int PIN_BTN_TOGGLE = 33; // Button 2

// ---------- I2C ----------
static const int I2C_SDA = 21;
static const int I2C_SCL = 22;
static const uint8_t OLED_ADDR = 0x3C;
static const uint8_t SHT31_ADDR = 0x44;

// ---------- OLED ----------
static const int SCREEN_WIDTH = 128;
static const int SCREEN_HEIGHT = 64;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------- Sensor ----------
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ---------- App State ----------
enum Screen {
  SCREEN_TEMP_HUM = 0,
  SCREEN_TEMP_ONLY = 1,
  SCREEN_HUM_ONLY  = 2,
  SCREEN_COUNT
};

static Screen currentScreen = SCREEN_TEMP_HUM;
static bool useFahrenheit = false;

// ---------- Timing ----------
static const uint32_t SENSOR_PERIOD_MS = 1000;
static uint32_t lastSensorMs = 0;

// ---------- Latest readings ----------
static float tempC = NAN;
static float humPct = NAN;

// ---------- Simple debounce ----------
struct DebouncedButton {
  int pin;
  bool lastStable;          // last stable reading (HIGH/LOW)
  bool lastReading;         // most recent raw reading
  uint32_t lastChangeMs;    // when raw reading last changed
};

static const uint32_t DEBOUNCE_MS = 35;
DebouncedButton btnNext { PIN_BTN_NEXT, HIGH, HIGH, 0 };
DebouncedButton btnToggle { PIN_BTN_TOGGLE, HIGH, HIGH, 0 };

bool updateButtonPressed(DebouncedButton &b) {
  bool reading = digitalRead(b.pin);

  if (reading != b.lastReading) {
    b.lastReading = reading;
    b.lastChangeMs = millis();
  }

  // If the reading has been stable long enough, accept it
  if ((millis() - b.lastChangeMs) > DEBOUNCE_MS) {
    if (reading != b.lastStable) {
      b.lastStable = reading;

      // We consider a "press" when it becomes LOW (because of pull-up)
      if (b.lastStable == LOW) {
        return true;
      }
    }
  }

  return false;
}

float toF(float c) { return c * 9.0f / 5.0f + 32.0f; }

void drawScreen() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  // Header line
  display.setTextSize(1);
  display.print("Screen ");
  display.print((int)currentScreen + 1);
  display.print("/");
  display.print((int)SCREEN_COUNT);
  display.print("  Units: ");
  display.println(useFahrenheit ? "F" : "C");

  display.println("----------------");

  if (isnan(tempC) || isnan(humPct)) {
    display.setTextSize(2);
    display.println("NO DATA");
    display.setTextSize(1);
    display.println("Check sensor");
    display.display();
    return;
  }

  float tempToShow = useFahrenheit ? toF(tempC) : tempC;

  if (currentScreen == SCREEN_TEMP_HUM) {
    display.setTextSize(2);
    display.print(tempToShow, 1);
    display.print(useFahrenheit ? "F" : "C");
    display.println();

    display.setTextSize(2);
    display.print(humPct, 1);
    display.println("%");
  }
  else if (currentScreen == SCREEN_TEMP_ONLY) {
    display.setTextSize(3);
    display.print(tempToShow, 1);
    display.print(useFahrenheit ? "F" : "C");
  }
  else if (currentScreen == SCREEN_HUM_ONLY) {
    display.setTextSize(3);
    display.print(humPct, 1);
    display.print("%");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  // Buttons: internal pull-ups, press connects pin to GND (LOW)
  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_TOGGLE, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: OLED init failed.");
    while (true) delay(1000);
  }

  if (!sht31.begin(SHT31_ADDR)) {
    Serial.println("ERROR: SHT31 init failed (check address/wiring).");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Ready.");
  display.println("BTN32: Next screen");
  display.println("BTN33: Toggle C/F");
  display.display();

  Serial.println("Ready. BTN32 next screen, BTN33 toggle C/F.");
}

void loop() {
  // ---- Read buttons (debounced) ----
  if (updateButtonPressed(btnNext)) {
    currentScreen = (Screen)((currentScreen + 1) % SCREEN_COUNT);
    drawScreen(); // instant feedback
  }

  if (updateButtonPressed(btnToggle)) {
    useFahrenheit = !useFahrenheit;
    drawScreen(); // instant feedback
  }

  // ---- Update sensor on a schedule (non-blocking) ----
  uint32_t now = millis();
  if (now - lastSensorMs >= SENSOR_PERIOD_MS) {
    lastSensorMs = now;

    tempC = sht31.readTemperature();
    humPct = sht31.readHumidity();

    if (!isnan(tempC) && !isnan(humPct)) {
      Serial.print("TempC=");
      Serial.print(tempC, 2);
      Serial.print("  Hum=");
      Serial.println(humPct, 2);
    } else {
      Serial.println("SHT31 read failed.");
    }

    drawScreen();
  }
}