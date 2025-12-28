#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SHT31.h>

// ---------- Pins ----------
static const int PIN_BTN_NEXT = 32; // Button 1
static const int PIN_BTN_PREV = 33; // Button 2

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
  SCREEN_MINMAX    = 3,
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

// ---------- Min/Max tracking ----------
static bool haveMinMax = false;
static float minTempC = NAN, maxTempC = NAN;
static float minHum = NAN,   maxHum = NAN;

float toF(float c) { return c * 9.0f / 5.0f + 32.0f; }

// ---------- Debounced + long-press buttons ----------
struct Button {
  int pin;
  bool stable;           // stable state (HIGH/LOW)
  bool lastReading;      // last raw read
  uint32_t lastChangeMs; // raw change time

  bool pressed;          // currently pressed?
  uint32_t pressStartMs; // when press started
  bool longFired;        // long press already triggered?
};

static const uint32_t DEBOUNCE_MS = 35;
static const uint32_t LONGPRESS_MS = 700;

Button btnNext { PIN_BTN_NEXT, HIGH, HIGH, 0, false, 0, false };
Button btnPrev { PIN_BTN_PREV, HIGH, HIGH, 0, false, 0, false };

enum ButtonEvent { NONE, SHORT_PRESS, LONG_PRESS };

ButtonEvent updateButton(Button &b) {
  bool reading = digitalRead(b.pin);

  if (reading != b.lastReading) {
    b.lastReading = reading;
    b.lastChangeMs = millis();
  }

  // Accept new stable state after debounce
  if ((millis() - b.lastChangeMs) > DEBOUNCE_MS) {
    if (reading != b.stable) {
      b.stable = reading;

      if (b.stable == LOW) { // became pressed
        b.pressed = true;
        b.pressStartMs = millis();
        b.longFired = false;
      } else { // became released
        bool wasPressed = b.pressed;
        b.pressed = false;

        if (wasPressed && !b.longFired) {
          return SHORT_PRESS;
        }
      }
    }
  }

  // Long press detection while held
  if (b.pressed && !b.longFired) {
    if (millis() - b.pressStartMs >= LONGPRESS_MS) {
      b.longFired = true;
      return LONG_PRESS;
    }
  }

  return NONE;
}

void updateMinMax(float tC, float h) {
  if (!haveMinMax) {
    haveMinMax = true;
    minTempC = maxTempC = tC;
    minHum   = maxHum   = h;
    return;
  }

  if (tC < minTempC) minTempC = tC;
  if (tC > maxTempC) maxTempC = tC;
  if (h  < minHum)   minHum   = h;
  if (h  > maxHum)   maxHum   = h;
}

void drawHeader() {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);

  display.print("Scr ");
  display.print((int)currentScreen + 1);
  display.print("/");
  display.print((int)SCREEN_COUNT);
  display.print("  ");
  display.println(useFahrenheit ? "F" : "C");

  display.println("----------------");
}

void drawScreen() {
  display.clearDisplay();
  drawHeader();

  if (isnan(tempC) || isnan(humPct)) {
    display.setTextSize(2);
    display.println("NO DATA");
    display.setTextSize(1);
    display.println("Check sensor");
    display.display();
    return;
  }

  float tShow = useFahrenheit ? toF(tempC) : tempC;

  if (currentScreen == SCREEN_TEMP_HUM) {
    display.setTextSize(2);
    display.print(tShow, 1);
    display.print(useFahrenheit ? "F" : "C");
    display.println();

    display.print(humPct, 1);
    display.println("%");
  } else if (currentScreen == SCREEN_TEMP_ONLY) {
    display.setTextSize(3);
    display.print(tShow, 1);
    display.print(useFahrenheit ? "F" : "C");
  } else if (currentScreen == SCREEN_HUM_ONLY) {
    display.setTextSize(3);
    display.print(humPct, 1);
    display.print("%");
  } else if (currentScreen == SCREEN_MINMAX) {
    display.setTextSize(1);
    display.println("Min/Max since boot:");

    float minT = useFahrenheit ? toF(minTempC) : minTempC;
    float maxT = useFahrenheit ? toF(maxTempC) : maxTempC;

    display.print("T min: ");
    display.print(minT, 1);
    display.print(useFahrenheit ? "F" : "C");
    display.print("  max: ");
    display.print(maxT, 1);
    display.println(useFahrenheit ? "F" : "C");

    display.print("H min: ");
    display.print(minHum, 1);
    display.print("%");
    display.print("   max: ");
    display.print(maxHum, 1);
    display.println("%");

    display.println();
    display.println("Hold btn: toggle C/F");
  }

  display.display();
}

void nextScreen() {
  currentScreen = (Screen)((currentScreen + 1) % SCREEN_COUNT);
  drawScreen();
}

void prevScreen() {
  currentScreen = (Screen)((currentScreen + SCREEN_COUNT - 1) % SCREEN_COUNT);
  drawScreen();
}

void toggleUnits() {
  useFahrenheit = !useFahrenheit;
  drawScreen();
}

void setup() {
  Serial.begin(115200);
  delay(200);

  pinMode(PIN_BTN_NEXT, INPUT_PULLUP);
  pinMode(PIN_BTN_PREV, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("ERROR: OLED init failed.");
    while (true) delay(1000);
  }
  if (!sht31.begin(SHT31_ADDR)) {
    Serial.println("ERROR: SHT31 init failed.");
    while (true) delay(1000);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Ready!");
  display.println("BTN32: next");
  display.println("BTN33: prev");
  display.println("Hold: toggle C/F");
  display.display();

  Serial.println("Ready: BTN32 next, BTN33 prev, hold toggles C/F.");
}

void loop() {
  // --- Buttons ---
  ButtonEvent e1 = updateButton(btnNext);
  ButtonEvent e2 = updateButton(btnPrev);

  if (e1 == SHORT_PRESS) nextScreen();
  if (e2 == SHORT_PRESS) prevScreen();

  if (e1 == LONG_PRESS || e2 == LONG_PRESS) toggleUnits();

  // --- Sensor update (1 Hz) ---
  uint32_t now = millis();
  if (now - lastSensorMs >= SENSOR_PERIOD_MS) {
    lastSensorMs = now;

    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      tempC = t;
      humPct = h;
      updateMinMax(t, h);

      Serial.print("T=");
      Serial.print(t, 2);
      Serial.print("C  H=");
      Serial.print(h, 2);
      Serial.println("%");
    } else {
      Serial.println("SHT31 read failed.");
    }

    drawScreen();
  }
}