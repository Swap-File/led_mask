#include <Adafruit_MSA301.h>
#include <Arduino.h>
#include <CapacitiveSensor.h>
#include <fastled.h>

Adafruit_MSA301 msa;

CapacitiveSensor cs_A2 = CapacitiveSensor(48, A2);
CapacitiveSensor cs_A3 = CapacitiveSensor(48, A3);
CapacitiveSensor cs_A4 = CapacitiveSensor(48, A4);
CapacitiveSensor cs_A5 = CapacitiveSensor(48, A5);

void input_init() {
  msa.begin();
  msa.setDataRate(MSA301_DATARATE_125_HZ);
  msa.setPowerMode(MSA301_NORMALMODE);
  msa.setBandwidth(MSA301_BANDWIDTH_31_25_HZ);
  msa.setRange(MSA301_RANGE_2_G);
  msa.setResolution(MSA301_RESOLUTION_14);

  msa.setClick(false, false, MSA301_TAPDUR_250_MS, 25);
  msa.enableInterrupts(true, true);  // enable single/double tap
}

uint8_t input_update() {
  static long total1_last = 0;
  static long total2_last = 0;
  static long total3_last = 0;
  static long total4_last = 0;

  static long total1 = 0;
  static long total2 = 0;
  static long total3 = 0;
  static long total4 = 0;

  static int round_robin = 1;
  if (round_robin == 1) total1 = cs_A2.capacitiveSensor(30);
  if (round_robin == 2) total2 = cs_A3.capacitiveSensor(30);
  if (round_robin == 3) total3 = cs_A4.capacitiveSensor(30);
  if (round_robin == 4) total4 = cs_A5.capacitiveSensor(30);
  if (++round_robin > 4) round_robin = 1;

  static bool button1 = false;
  static bool button2 = false;
  static bool button3 = false;
  static bool button4 = false;

  if (total1 > 4000 && total1_last <= 4000) button1 = true;
  if (total2 > 4000 && total2_last <= 4000) button2 = true;
  if (total3 > 4000 && total3_last <= 4000) button3 = true;
  if (total4 > 4000 && total4_last <= 4000) button4 = true;
  if (total1 < 4000 && total1_last > 4000) button1 = false;
  if (total2 < 4000 && total2_last > 4000) button2 = false;
  if (total3 < 4000 && total3_last > 4000) button3 = false;
  if (total4 < 4000 && total4_last > 4000) button4 = false;

  total1_last = total1;
  total2_last = total2;
  total3_last = total3;
  total4_last = total4;

  uint8_t button_result = 0;
  if (button1) button_result |= 0b00000001;
  if (button2) button_result |= 0b00000010;
  if (button3) button_result |= 0b00000100;
  if (button4) button_result |= 0b00001000;

  return button_result;
}

double input_get_gravity(void) {
  msa.read();
  return (atan2(-msa.y, msa.x));
}

uint8_t input_get_tap(void) {
  static uint32_t tap_time = 0;
  static uint8_t tap = 0;
  uint8_t motionstat = msa.getMotionInterruptStatus();
  if (motionstat) {
    if (motionstat & (1 << 5)) {
      tap_time = millis();
      tap = 32;
    }
    if (motionstat & (1 << 4)) {
      tap_time = millis() + 5000;
      tap = 128;
    }
  }
  if (millis() > tap_time)
    tap = qsub8(tap, 1);

  return tap;
}