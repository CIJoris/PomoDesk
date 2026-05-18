# PomoDesk - Physical Pomodoro Timer
**ESP32-based Pomodoro timer with API options**

V1.0:

// --- Pins for ESP32-S3-WROOM-1 ---
const int BTN_STATE = 1;       // IO1, button to GND
const int LED_DATA  = 48;      // IO48, WS2812B data in
const int PWR_LED   = 18;      // IO18, sink power LED to GND

// --- LEDs ---
const int NUM_LEDS = 19;

// --- Timing ---
const uint32_t WORK_MS     = 45 mins;
const uint32_t BREAK_MS    =  5 mins;
const uint32_t IDLE_LED_MS = 10 mins;

**Focus for 45 minutes, break for 5, then continue.**



---

*Feel free to send me constructive criticism or things you would like me to implement, or even if you would like to help me develop this system!*
