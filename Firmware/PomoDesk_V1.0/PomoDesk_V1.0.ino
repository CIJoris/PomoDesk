#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <esp_bt.h>

// --- Pins for ESP32-S3-WROOM-1 ---
const int BTN_STATE = 1;       // IO1, button to GND
const int LED_DATA  = 48;      // IO48, WS2812B data in
const int PWR_LED   = 18;      // IO18, sink power LED to GND

// --- LEDs ---
const int NUM_LEDS = 19;
Adafruit_NeoPixel strip(NUM_LEDS, LED_DATA, NEO_GRB + NEO_KHZ800);

// --- Timing ---
const uint32_t WORK_MS     = 45UL * 60UL * 1000UL;
const uint32_t BREAK_MS    =  5UL * 60UL * 1000UL;
const uint32_t IDLE_LED_MS = 10UL * 60UL * 1000UL;

// --- States ---
enum State { IDLE, WORK, BREAK };
State state = IDLE;

uint32_t stateStartMs = 0;
bool idleLedsOff = false;

// --- Colors ---
const uint32_t COLOR_IDLE  = Adafruit_NeoPixel::Color(255, 80, 0);  // orange
const uint32_t COLOR_WORK  = Adafruit_NeoPixel::Color(0, 255, 0);   // green
const uint32_t COLOR_BREAK = Adafruit_NeoPixel::Color(255, 0, 0);   // red

void setRing(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void allLedsOff() {
  strip.clear();
  strip.show();
}

void enterState(State s) {
  state = s;
  stateStartMs = millis();
  idleLedsOff = false;

  switch (state) {
    case IDLE:
      setRing(COLOR_IDLE);
      break;

    case WORK:
      setRing(COLOR_WORK);
      break;

    case BREAK:
      setRing(COLOR_BREAK);
      break;
  }
}

void nextState() {
  switch (state) {
    case IDLE:
      enterState(WORK);
      break;

    case WORK:
      enterState(BREAK);
      break;

    case BREAK:
      enterState(IDLE);
      break;
  }
}

// --- Debounced button ---
struct DebouncedButton {
  int pin;
  bool lastRaw = HIGH;
  bool stable = HIGH;
  uint32_t lastChangeMs = 0;
  static const uint32_t DEBOUNCE_MS = 30;

  void begin() {
    pinMode(pin, INPUT_PULLUP);   // button should connect IO1 to GND
    lastRaw = digitalRead(pin);
    stable = lastRaw;
    lastChangeMs = millis();
  }

  bool pressedEvent() {
    bool raw = digitalRead(pin);

    if (raw != lastRaw) {
      lastRaw = raw;
      lastChangeMs = millis();
    }

    if (millis() - lastChangeMs > DEBOUNCE_MS) {
      if (stable != raw) {
        stable = raw;
        if (stable == LOW) return true;   // active-low press
      }
    }

    return false;
  }
};

DebouncedButton button { BTN_STATE };

void setup() {
  // Power status LED: 3V3 -> resistor -> LED -> IO18
  // Drive IO18 LOW to sink current and turn it on.
  pinMode(PWR_LED, OUTPUT);
  digitalWrite(PWR_LED, HIGH); // OFF

  WiFi.mode(WIFI_OFF); // limit power consumption -> temperature gain
  btStop();
  setCpuFrequencyMhz(80); // limit power consumption -> temperature gain

  button.begin();

  strip.begin();
  strip.setBrightness(150);  // adjust as needed: 0-255
  allLedsOff();
  
  delay(5);
  enterState(IDLE);
}

void loop() {
  delay(5); // timer accuracy will still be fine, and the CPU won’t spin flat-out constantly.

  uint32_t now = millis();

  // Button cycles: IDLE -> WORK -> BREAK -> IDLE
  if (button.pressedEvent()) {
    nextState();
    return;
  }

  // Automatic transitions
  if (state == WORK) {
    if (now - stateStartMs >= WORK_MS) {
      enterState(BREAK);
    }
  }
  else if (state == BREAK) {
    if (now - stateStartMs >= BREAK_MS) {
      enterState(IDLE);
    }
  }
  else if (state == IDLE) {
    if (!idleLedsOff && now - stateStartMs >= IDLE_LED_MS) {
      allLedsOff();
      idleLedsOff = true;
    }
  }
}