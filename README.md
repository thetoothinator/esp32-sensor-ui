# esp32-sensor-ui

An embedded "sensor UI" on an ESP32 that reads temperature and humidity from an SHT31 over I<sup>2</sup>C and displays live data on a .96" SSD1306 OLED. Two pushbuttons provide a simple user interface with screen navigation and long-press actions, including a Min/Max screen.

## Demo (Click the image to watch the demo)
[![ESP32 Sensor UI demo video](./docs/demo.jpg)](https://www.youtube.com/watch?v=t4kKeUxgJoU)

## Features
- SHT31 temperature and humidity sensor over **I<sup>2</sup>C**
- SSD1036 .96" OLED display over I<sup>2</sup>C
- Non-blocking update loop using `millis()`
- Button-driven UI with debounce and long-press detection:
    - **BTN32 short press:** next screen
    - **BTN33 short press:** previous screen
    - **Hold either button (~0.7s):** toggle C/F
- Screens:
    1. Temp and Humidity
    2. Temp only (large)
    3. Humidity only (large)
    4. Min/Max since boot

## Hardware
- ESP32 DevKit (CP2102N USB-UART)
- SHT31 breakout
- 0.96" SSD1306 OLED display
- 2x momentary pushbuttons
- Breadboard and jumper wires

## Wiring

### Power (3.3V)
| Module | Pin | ESP32 |
|--------|-----|-------|
| OLED   | VCC | 3V3   |
| OLED   | GND | GND   |
| SHT31  | VIN | 3V3   |
| SHT31  | GND | GND   |

### I<sup>2</sup>C (shared bus)
| Signal | ESP32 GPIO | OLED | SHT31 |
|--------|------------|------|-------|
| SDA    | GPIO21     | SDA  | SDA   |
| SCL    | GPIO22     | SCL  | SCL   |

### Buttons (wired using internal pull-ups)
- **not pressed = HIGH**
- **pressed = LOW** (connects GPIO to GND)

| Button | ESP32 GPIO | Other side |
|--------|------------|------------|
| BTN32  | GPIO32     | GND        |
| BTN33  | GPIO33     | GND        |

## Software / Tools
- VS Code + **PlatformIO**
- Framework: **Arduino** on ESP32

## Build & Upload (PlatformIO)
1. Open the PlatformIO project folder: `firmware/` (contains `platform.ini`)
2. Connect the ESP32 via USB
3. PlatformIO -> **Upload**
4. PlatformIO -> **Monitor**

## Project Structure
esp32-sensor-ui/
README.md
LICENSE
firmware/
platform.ini
src/
main.cpp

## Expected I<sup>2</sup>C Addresses
- OLED is commonly **0x3C**
- SHT31 is commonly **0x44**

## Troubleshooting
- Garbled Serial Monitor output: set `monitor_speed = 115200` in `platform.ini`
- OLED blank: verify VCC=3V3, GND, SDA/SCL not swapped; confirm OLED address
- SHT31 not detected: verify VIN=3V3 and SDA/SCL wiring; confirm address
- Buttons do nothing: ensure the button straddles the breadboard gap and connects the GPIO pin to GND when pressed

## Roadmap / Improvements
- Refractor into modules (`sensor/`, `ui/`, `buttons/`) for a production-style layout
- Add a Settings screen
- Add robust sensor error handling

## Resume bullets
- Built an ESP32-based sensor UI using I<sup>2</sup>C peripherals (SHT31 + SSD1306 OLED) with PlatformIO/Arduino.
- Implemented a button-driven multi-screen UI with debouncing, short/long-press detection, and C/F unit toggling.
- Desgined a non-blocking main loop using `millis()` scheduling and added min/max tracking for sensor telemetry.
- Documented wiring, build steps, and troubleshooting to support reproducible hardware bring-up.

## License
MIT (see `LICENSE`).