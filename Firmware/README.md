# PomoDesk - Physical Pomodoro Timer
**ESP32-based Pomodoro timer with API options**


V1.0:
---

## Features

- Physical one-button control
- Idle, Focus, Break states
- Multiple LED transition modes
- Settings saved in ESP32 Preferences memory

* *Pins for ESP32-S3-WROOM-1*
BTN_STATE = 1;       // IO1, button to GND
LED_DATA  = 48;      // IO48, WS2812B data in
PWR_LED   = 18;      // IO18, sink power LED to GND

* *LEDs*
NUM_LEDS = 19;

* *Timing*
WORK_MS     = 45 mins;
BREAK_MS    =  5 mins;
IDLE_LED_MS = 10 mins;

**Focus for 45 minutes, break for 5, then continue.**


V1.1:
---

**Now with a configurable NeoPixel LED ring and a built-in local configuration webserver.**

## Features

- Config state
- Adjustable state colors
- Adjustable Focus, Break, and Idle times
- Adjustable transition speed
- Brightness control with ECO mode
- Local Wi-Fi configuration portal
- Deep sleep mode with button wake-up

## Controls

- Short press: step through states
- Hold 1.2 seconds: enter or exit config mode
- Hold 3 seconds: go to sleep

## Config Mode

When config mode is active, PomoDesk creates its own Wi-Fi network:

- Wi-Fi: `PomoDesk-Config`
- Password: `pomodesk`
- Open: `http://pomodesk.local`
- Fallback: `http://10.10.10.1`

The web interface lets you customize colors, timers, transitions, speed, brightness, reset defaults, exit config mode, or put the device to sleep.


---

*Feel free to send me constructive criticism or things you would like me to implement, or even if you would like to help me develop this system!*
