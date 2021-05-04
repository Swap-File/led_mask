
#include <Adafruit_NeoPixel.h>
#define FASTLED_FORCE_SOFTWARE_PINS
#include <FastLED.h>

#define PIN_ONBOARD_LED 13
#define PIN_CHEEK_LEFT 2
#define PIN_CHEEK_RIGHT 3
#define PIN_MOUTH 8

#define NUM_LEDS_MOUTH 4
#define NUM_LEDS_CHEEK 16
CRGB fastled_mouth[NUM_LEDS_MOUTH];

Adafruit_NeoPixel cheek_left = Adafruit_NeoPixel(NUM_LEDS_CHEEK, PIN_CHEEK_LEFT, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel cheek_right = Adafruit_NeoPixel(NUM_LEDS_CHEEK, PIN_CHEEK_RIGHT, NEO_GRBW + NEO_KHZ800);

static void onboard_blink(void) {
  static uint8_t cycle_counter = 0;
  static bool blink_mode = false;

  if (++cycle_counter > 10) {
   // digitalWrite(PIN_ONBOARD_LED, blink_mode);
    blink_mode = !blink_mode;
    cycle_counter = 0;
  }
}
void led_mouth_update(int voice_volume, int mask_pressure, CRGB color1) {
  //led update
  if (mask_pressure)
    fastled_mouth[0] = color1;
  else
    fastled_mouth[0] = CRGB(0, 0, 0);

  fastled_mouth[1] = fastled_mouth[0];
  fastled_mouth[2] = fastled_mouth[0];
  fastled_mouth[3] = fastled_mouth[0];

  FastLED.show();
}

void led_cheek_update(int voice_volume, int mask_pressure, CRGB color1, uint8_t tap, CRGB left, CRGB right) {
  static int cheek_index = 0;
  if (mask_pressure)
    cheek_index++;

  if (cheek_index > 15) cheek_index = 0;

  cheek_left.clear();
  cheek_right.clear();

  for (int i = 0; i < 16; i++) {
    cheek_left.setPixelColor(i, left.r, left.g, left.b, tap);
    cheek_right.setPixelColor(i, right.r, right.g, right.b, tap);
  }

  if (mask_pressure) {
    cheek_left.setPixelColor(cheek_index, color1.r, color1.g, color1.b, 0);
    int temp = (((NUM_LEDS_CHEEK - 1) - cheek_index) + 12) % NUM_LEDS_CHEEK;
    cheek_right.setPixelColor(temp, color1.r, color1.g, color1.b, 0);
  }

  cheek_left.show();
  cheek_right.show();

  onboard_blink();
}
void led_init() {
  FastLED.addLeds<NEOPIXEL, PIN_MOUTH>(fastled_mouth, NUM_LEDS_MOUTH);
  cheek_left.begin();
  cheek_right.begin();
  cheek_left.show();
  cheek_right.show();

  pinMode(PIN_ONBOARD_LED, OUTPUT);
}
