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
const int LED_BRIGHTNESS = 150;  // adjust as needed: 0-255

// --- Timing ---
const uint32_t WORK_MS     = 45UL * 60UL * 1000UL;
const uint32_t BREAK_MS    =  5UL * 60UL * 1000UL;
const uint32_t IDLE_LED_MS = 10UL * 60UL * 1000UL;
const uint32_t LONG_PRESS_MS = 2000;

// --- States ---
enum State { IDLE, WORK, BREAK, ALL_OFF };
State state = IDLE;

uint32_t stateStartMs = 0;
bool idleLedsOff = false;

// --- Colors ---
const uint32_t COLOR_IDLE  = Adafruit_NeoPixel::Color(255, 80, 0);  // orange
const uint32_t COLOR_WORK  = Adafruit_NeoPixel::Color(0, 255, 0);   // green
const uint32_t COLOR_BREAK = Adafruit_NeoPixel::Color(255, 0, 0);   // red
uint32_t currentColor = 0;

// --- Transitions ---
enum Transition { SET_RING, RUNNING_LIGHT, RUNNING_FILL, RUNNING_LIGHT_CLEAR, FADE, FADE_OUT_IN };
Transition transition = FADE_OUT_IN;

// --- Transition timing ---
const int MIN_DELAY = 10;
const int MAX_DELAY = 500;
uint16_t transitionDelayMs = 25; // (10-500) ms per pixel, fast(10), pleasant(40), relaxed(100+)


void setRing(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();

  currentColor = color;
}


void runningLights(uint32_t color) {
  allLedsOff();

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.clear();

    strip.setPixelColor(i, color);

    // Small tail behind the running LED
    int tail1 = (i - 1 + NUM_LEDS) % NUM_LEDS;
    int tail2 = (i - 2 + NUM_LEDS) % NUM_LEDS;

    strip.setPixelColor(tail1, color);
    strip.setPixelColor(tail2, color);

    strip.show();
    delay(constrain(transitionDelayMs, MIN_DELAY, MAX_DELAY));
  }

  setRing(color);
}

void runningFillClear(uint32_t color) {
  allLedsOff();

  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(constrain(transitionDelayMs, MIN_DELAY, MAX_DELAY));
  }
}

void runningFill(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(constrain(transitionDelayMs, MIN_DELAY, MAX_DELAY));
  }
}

uint8_t blend8(uint8_t from, uint8_t to, uint8_t step, uint8_t steps) {
  return from + ((int16_t)(to - from) * step) / steps;
}

void fadeToColor(uint32_t targetColor) {
  uint8_t r1 = (currentColor >> 16) & 0xFF;
  uint8_t g1 = (currentColor >> 8)  & 0xFF;
  uint8_t b1 =  currentColor        & 0xFF;

  uint8_t r2 = (targetColor >> 16) & 0xFF;
  uint8_t g2 = (targetColor >> 8)  & 0xFF;
  uint8_t b2 =  targetColor        & 0xFF;

  const uint8_t fadeSteps = 40;
  uint16_t d = constrain(transitionDelayMs, MIN_DELAY, MAX_DELAY);

  for (uint8_t step = 0; step <= fadeSteps; step++) {
    uint32_t c = strip.Color(
      blend8(r1, r2, step, fadeSteps),
      blend8(g1, g2, step, fadeSteps),
      blend8(b1, b2, step, fadeSteps)
    );

    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, c);
    }

    strip.show();
    delay(d);
  }

  currentColor = targetColor;
}

void fadeOutIn(uint32_t targetColor) {
  uint8_t r1 = (currentColor >> 16) & 0xFF;
  uint8_t g1 = (currentColor >> 8)  & 0xFF;
  uint8_t b1 =  currentColor        & 0xFF;

  uint8_t r2 = (targetColor >> 16) & 0xFF;
  uint8_t g2 = (targetColor >> 8)  & 0xFF;
  uint8_t b2 =  targetColor        & 0xFF;

  const uint8_t fadeSteps = 40;
  uint16_t d = constrain(transitionDelayMs, MIN_DELAY, MAX_DELAY);

  // Fade out to off
  for (uint8_t step = 0; step <= fadeSteps; step++) {
    uint32_t c = strip.Color(
      blend8(r1, 0, step, fadeSteps),
      blend8(g1, 0, step, fadeSteps),
      blend8(b1, 0, step, fadeSteps)
    );

    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, c);
    }

    strip.show();
    delay(d);
  }

  // Fade in to new color
  for (uint8_t step = 0; step <= fadeSteps; step++) {
    uint32_t c = strip.Color(
      blend8(0, r2, step, fadeSteps),
      blend8(0, g2, step, fadeSteps),
      blend8(0, b2, step, fadeSteps)
    );

    for (int i = 0; i < NUM_LEDS; i++) {
      strip.setPixelColor(i, c);
    }

    strip.show();
    delay(d);
  }

  currentColor = targetColor;
}

void allLedsOff() {
  strip.clear();
  strip.show();
}

void applyTransition(uint32_t color) {
  switch (transition) {
    case SET_RING:
      setRing(color);
      break;

    case RUNNING_LIGHT:
      runningLights(color);
      break;

    case RUNNING_FILL:
      runningFill(color);
      currentColor = color;
      break;

    case RUNNING_LIGHT_CLEAR:
      runningFillClear(color);
      currentColor = 0;
      break;

    case FADE:
      fadeToColor(color);
      break;

    case FADE_OUT_IN:
      fadeOutIn(color);
      break;
  }
}

void enterState(State s) {
  state = s;
  stateStartMs = millis();
  idleLedsOff = false;

  switch (state) {
    case IDLE:
      applyTransition(COLOR_IDLE);
      break;

    case WORK:
      applyTransition(COLOR_WORK);
      break;

    case BREAK:
      applyTransition(COLOR_BREAK);
      break;

    case ALL_OFF:
      allLedsOff();
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

  bool pressHandled = false;

  uint32_t lastChangeMs = 0;
  uint32_t pressStartMs = 0;

  static const uint32_t DEBOUNCE_MS = 30;

  void begin() {
    pinMode(pin, INPUT_PULLUP);

    lastRaw = digitalRead(pin);
    stable = lastRaw;

    lastChangeMs = millis();
  }

  void update() {
    bool raw = digitalRead(pin);

    if (raw != lastRaw) {
      lastRaw = raw;
      lastChangeMs = millis();
    }

    if (millis() - lastChangeMs > DEBOUNCE_MS) {

      if (stable != raw) {
        stable = raw;

        if (stable == LOW) {
          // button pressed
          pressStartMs = millis();
          pressHandled = false;
        }
      }
    }
  }

  bool shortPressEvent() {
    if (stable == HIGH &&
        !pressHandled &&
        (millis() - pressStartMs) < LONG_PRESS_MS &&
        pressStartMs != 0) {

      pressHandled = true;
      return true;
    }

    return false;
  }

  bool longPressEvent() {
    if (stable == LOW &&
        !pressHandled &&
        (millis() - pressStartMs >= LONG_PRESS_MS)) {

      pressHandled = true;
      return true;
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

  delay(200); // small delay before initializing LEDs to reduce startup surge

  strip.begin();
  strip.setBrightness(LED_BRIGHTNESS);
  allLedsOff();

  delay(5);
  enterState(IDLE);
}

void loop() {
  delay(5); // timer accuracy will still be fine, and the CPU won’t spin flat-out constantly.

  uint32_t now = millis();

  button.update();

  // Long press -> ALL OFF mode
  if (button.longPressEvent()) {
    enterState(ALL_OFF);
    return;
  }

  // Short press cycles states
  if (button.shortPressEvent()) {

    if (state == ALL_OFF) {
      enterState(IDLE);
    } else {
      nextState();
    }

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