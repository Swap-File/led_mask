
#include <Arduino.h>

#define PIN_SPEAKER_ENABLE 49
#define PIN_MIC A0

void sound_init(void) {
  pinMode(PIN_SPEAKER_ENABLE, OUTPUT);
  digitalWrite(PIN_SPEAKER_ENABLE, HIGH);

  pinMode(PIN_MIC, INPUT);  // ADC
  analogReadResolution(12);
}

int sound_read(void) {
  static int dac_peak = 4096;
  const int dac_min = 256;       // 1546 measured in silence, - 500 muteing the low end = 1000
  int dac_now = analogRead(A0);  //15.5 microseconds at 12 bit, 14.5 microseconds at 10 bit
  dac_peak = max(max(dac_peak * .98, dac_now), dac_min + 1);
  int value = map(dac_now, dac_min, dac_peak, 120, 0);  //0-4095  128x128
  value = constrain(value, 0, 120);                     // 0 is off.
  return value;
}

void volume_set(int i) {
  if (i == 0) {
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);
    pinMode(11, OUTPUT);
    digitalWrite(11, LOW);
  }
  if (i == 1) {
    pinMode(12, INPUT);
    pinMode(11, OUTPUT);
    digitalWrite(11, LOW);
  }
  if (i == 2) {
    pinMode(12, OUTPUT);
    digitalWrite(12, LOW);
    pinMode(11, INPUT);
  }
  if (i == 3) {
    pinMode(12, INPUT);
    pinMode(11, INPUT);
  }
}

void sound_mute(int value) {
  static int value_count = 0;
  if (value == INT_MAX) {
    volume_set(3);
    digitalWrite(PIN_SPEAKER_ENABLE, HIGH);
    value_count = 0;
    return;
  }

  volume_set(0);

  static uint32_t off_time = 0;

  if (millis() - off_time > 100) {
    if (value < 1) {
      if (value_count > 0) value_count--;
    } else {
      value_count = 10;
    }
  }

  if (value_count == 10) digitalWrite(PIN_SPEAKER_ENABLE, HIGH);
  if (value_count == 0) {
    digitalWrite(PIN_SPEAKER_ENABLE, LOW);
    off_time = millis();
    value_count--;
  }
}
