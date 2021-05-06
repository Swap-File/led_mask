
#include <Adafruit_BMP280.h>  // altered to be faster, temp doesnt get re-read on pressure poll

#include "fastled.h"
#include "input.h"
#include "led.h"
#include "screen.h"
#include "sound.h"

#define PIN_TFT_BACKLIGHT 47  // Display backlight pin

Adafruit_BMP280 bmp;

static void fps_update(void) {
  static int fps = 0;
  fps++;

  static uint32_t fps_timer = 0;
  if (millis() > fps_timer) {
    Serial.print("FPS: ");
    Serial.println(fps);
    fps = 0;
    while (fps_timer <= millis()) {
      fps_timer += 1000;
    }
  }
}

#define GUI_MODE_LIGHT 2
#define GUI_MODE_SOUND 1
#define GUI_MODE_OFF 0
#define GUI_MODE_HORN 3
static uint8_t gui_mode = GUI_MODE_OFF;

#define HUE_MODE_RANDOM 0
#define HUE_MODE_WHITE 1
#define HUE_MODE_RAINBOW 2
#define HUE_MODE_PORTAL 3
static int hue_mode = HUE_MODE_RAINBOW;

static uint8_t hue = 0;

static uint16_t rgb565(CRGB color) {
  uint16_t temp = 0;
  temp |= (((uint16_t)(color.r >> 3 & 0b00011111)) << 11);
  temp |= (((uint16_t)(color.g >> 2 & 0b00111111)) << 5);
  temp |= (((uint16_t)(color.b >> 3 & 0b00011111)) << 0);
  return temp;
}

void setup(void) {
  Serial.begin(115200);
  input_init();
  screen_init();
  led_init();
  sound_init();

  pinMode(PIN_TFT_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_TFT_BACKLIGHT, HIGH);  // Backlight on

  pinMode(A1, INPUT);  //LIGHT

  if (!bmp.begin(BMP280_ADDRESS_ALT, BMP280_CHIPID)) {
    Serial.println("I2C Disconnected!");
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X1,   /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

  hue = random(256);  // random first color on boot
}


void loop() {

  static CRGB color1 = CHSV(hue, 255, 255);
  static bool cheek_on = false;
  static bool mouth_on = false;
  static bool mouth_last = false;

  if (hue_mode == HUE_MODE_RAINBOW) {
    if (mouth_on || cheek_on || gui_mode == GUI_MODE_LIGHT) {
      hue++;
      color1 = CHSV(hue, 255, 255);
    }
  } else if (hue_mode == HUE_MODE_RANDOM) {
    if (mouth_last == true && mouth_on == false) {
      hue = random(256);
      color1 = CHSV(hue, 255, 255);
    }
  } else if (hue_mode == HUE_MODE_WHITE) {
    if (mouth_last == true && mouth_on == false) {
      color1 = CRGB(128, 128, 128);
    }
  } else if (hue_mode == HUE_MODE_PORTAL) {
    if (mouth_last == true && mouth_on == false) {
      if (color1 != CRGB(0, 30, 224)) {
        color1 = CRGB(0, 30, 224);
      } else {
        color1 = CRGB(214, 40, 0);
      }
    }
  }

  //pressure read
  static float mask_pressure_avg_slow = 0;
  static float mask_pressure_avg_slow_last = 0;
  static float mask_pressure_avg_change = 0;

  bmp.readTemperature();
  float mask_pressure = bmp.readPressure();

  if (mask_pressure_avg_slow == 0) {  //load init on boot
    mask_pressure_avg_slow = mask_pressure;
    mask_pressure_avg_slow_last = mask_pressure_avg_slow;
  }

  mask_pressure_avg_slow = mask_pressure_avg_slow * .98 + mask_pressure * .02;
  float change = mask_pressure_avg_slow - mask_pressure_avg_slow_last;
  mask_pressure_avg_change = mask_pressure_avg_change * .98 + change * 0.02;

  //gravity, brightness, and sound
  double gravity = input_get_gravity();
  //int ambient_brightness = analogRead(A1);
  int value = sound_read();

  screen_update(gravity, value, rgb565(color1));

  //calculate LED state
  if (mask_pressure_avg_change < -.08)
    cheek_on = true;
  else
    cheek_on = false;

  mouth_last = mouth_on;
  if (change > .5) mouth_on = true;
  if (mask_pressure_avg_change < .05) mouth_on = false;
  //do led state
  static uint8_t touchbuttons_last = 0;
  uint8_t touchbuttons = input_update();

  CRGB left = CRGB(0, 0, 0);
  CRGB right = CRGB(0, 0, 0);

  if (touchbuttons == 0x00) gui_mode = GUI_MODE_OFF;
  static bool speakeron = false;

  if (touchbuttons == 0b0000011 || touchbuttons == 0b0001100) {
    sound_mute(INT_MAX);
    gui_mode = GUI_MODE_HORN;
  } else {
    if (!speakeron)
      sound_mute(0);
    else
      sound_mute(value);
  }

  if (gui_mode == GUI_MODE_OFF) {
    if (touchbuttons == 0b00000001) {
      gui_mode = GUI_MODE_SOUND;
    }

    if (touchbuttons == 0b00001000) {
      gui_mode = GUI_MODE_LIGHT;
    }
  }

  if (gui_mode == GUI_MODE_LIGHT) {
    right = CRGB(0, 0, 0);

    if (hue_mode == HUE_MODE_RANDOM) {
      static uint32_t random_timeout = 0;
      if (millis() - random_timeout > 1000) {
        hue = random(256);
        random_timeout = millis();
      }
      left = CHSV(hue, 255, 255);
    } else if (hue_mode == HUE_MODE_WHITE)
      left = CRGB(64, 64, 64);
    else if (hue_mode == HUE_MODE_RAINBOW)
      left = CHSV(hue, 255, 255);
    else if (hue_mode == HUE_MODE_PORTAL) {
      static uint32_t random_timeout = 0;
      if (millis() - random_timeout > 1000) {
        if (color1 != CRGB(0, 30, 224)) {
          color1 = CRGB(0, 30, 224);
        } else {
          color1 = CRGB(214, 40, 0);
        }
        random_timeout = millis();
      }

      left = color1;
    }

    if (touchbuttons_last == 0b00001000 && touchbuttons == 0b00001001)
      if (++hue_mode > 3) hue_mode = 0;
  }

  if (gui_mode == GUI_MODE_SOUND) {
    left = CRGB(0, 0, 0);

    if (speakeron)
      right = CRGB(0, 255, 0);
    else
      right = CRGB(255, 0, 0);

    if (touchbuttons_last == 0b00000001 && touchbuttons == 0b00001001)
      speakeron = !speakeron;
  }

  static bool soft_off = false;
  if (touchbuttons == 0b00000110) {
       soft_off = true;
    }
    
  uint8_t tap_val = input_get_tap(soft_off);
  if (tap_val != 0) {
    if (touchbuttons == 0)
      soft_off = false;
    else
      soft_off = true;
  }

  if (soft_off) {
    led_cheek_update(value, cheek_on, CRGB(0, 0, 0), 0, CRGB(0, 0, 0), CRGB(0, 0, 0));
    led_mouth_update(value, mouth_on, CRGB(0, 0, 0));
    digitalWrite(PIN_TFT_BACKLIGHT, LOW);
  } else {
    led_cheek_update(value, cheek_on, color1, tap_val, left, right);
    led_mouth_update(value, mouth_on, color1);
    digitalWrite(PIN_TFT_BACKLIGHT, HIGH);
  }

  fps_update();

  mask_pressure_avg_slow_last = mask_pressure_avg_slow;
  touchbuttons_last = touchbuttons;
}
