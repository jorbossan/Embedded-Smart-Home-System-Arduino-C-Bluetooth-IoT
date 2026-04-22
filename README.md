# Embedded-Smart-Home-System-Arduino-C-Bluetooth-IoT

Arduino-based smart home system that integrates sensors, actuators and Bluetooth communication to enable basic home automation and remote control. The system reacts to environmental conditions and allows manual interaction through a custom command-based protocol displayed on an I2C LCD.

Project Preview

Features
- Automatic lighting control using LDR (PWM)
- Temperature & humidity monitoring (DHT11)
- Climate control based on temperature threshold
- RGB LED control (manual + preset modes)
- Servo-based door simulation
- Relay control for external devices
- Bluetooth communication (HM-10)
- Command-based control system

Wiring

- LDR → A15
- LED (main) → Pin 2
- PWM LED → Pin 3
- RGB LED → Pins 11, 12, 13
- Motor driver → Pins 5, 6
- DHT11 → Pin 9
- Servo → Pin 22
- Relay → Pin 30
- LCD I2C → SDA (20), SCL (21)
- Bluetooth HM-10 → Serial1

How it works

The system continuously runs a main loop that:

- Reads commands from Serial or Bluetooth
- Updates sensor values periodically
- Executes automatic behaviors when enabled

Lighting and climate systems can operate in both manual and automatic modes.

The user interacts with the system through a simple command protocol:

- Single-character commands trigger actions
- Optional numeric values allow precise control (PWM, servo position)

Examples:

- L / l → Light ON / OFF
- S + value (0–255) → LED brightness
- R/G/B + value → RGB control
- O / W → Open / close door
- P + value (0–180) → Servo position
