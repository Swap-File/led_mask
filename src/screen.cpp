#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

Adafruit_ST7789 tft = Adafruit_ST7789(&SPI1, 44, 45, 46);
struct Point {
  int x, y;
};

int fix_index(int index) {
  while (index < -(tft.width() + tft.height() - 2)) {
    index += tft.width() + tft.height() + tft.width() + tft.height() - 3;
  }

  while (index > (tft.width() + tft.height() - 2)) {
    index -= tft.width() + tft.height() + tft.width() + tft.height() - 3;
  }
  return index;
}
Point xy(int index) {
  index = fix_index(index);

  struct Point p1;

  if (index <= tft.height() / 2 && index > -tft.height() / 2) {
    p1.x = (tft.width() - 1);
    p1.y = tft.height() - (tft.height() / 2 + index);
  } else if (index > 0 && index < tft.height() / 2 + tft.width()) {
    p1.x = (tft.width() - 1) - (index - (tft.height() / 2));
    p1.y = 0;
  } else if (index < 0 && index > -(tft.height() / 2 + tft.width() - 1)) {
    p1.x = (tft.width() - 1) - (-index - tft.height() / 2 + 1);
    p1.y = tft.height() - 1;
  } else if (index > 0) {
    p1.x = 0;
    p1.y = index - (tft.width() - 1) - tft.height() / 2;
  } else if (index < 0) {
    p1.x = 0;
    p1.y = tft.height() - ((-index) - tft.width() - tft.height() / 2) - 3;
  }
  return p1;
}

void screen_update(double gravity_double, int value, uint16_t color1) {
  static int value_previous = 0;

  int gravity = gravity_double * (tft.width() + tft.height() - 1) / PI;
  // bottom line
  static struct Point p1;
  static struct Point p2;
  struct Point p3 = xy(gravity - (tft.height() - 1) - value);
  struct Point p4 = xy(gravity + (tft.height()) + value);
  tft.drawLine(p1.x, p1.y, p2.x, p2.y, ST77XX_BLACK);
  tft.drawLine(p3.x, p3.y, p4.x, p4.y, ST77XX_WHITE);
  p1 = p3;
  p2 = p4;

  //middle line
  static struct Point p1c;
  static struct Point p2c;
  struct Point p3c = xy(gravity - (tft.height()));
  struct Point p4c = xy(gravity + (tft.height()));
  tft.drawLine(p1c.x, p1c.y, p2c.x, p2c.y, ST77XX_BLACK);
  tft.drawLine(p3c.x, p3c.y, p4c.x, p4c.y, ST77XX_WHITE);
  p1c = p3c;
  p2c = p4c;

  //top line
  static struct Point p1b;
  static struct Point p2b;
  struct Point p3b = xy(gravity - (tft.height() + 1) + value);
  struct Point p4b = xy(gravity + (tft.height() + 2) - value);
  tft.drawLine(p1b.x, p1b.y, p2b.x, p2b.y, ST77XX_BLACK);
  tft.drawLine(p3b.x, p3b.y, p4b.x, p4b.y, ST77XX_WHITE);
  p1b = p3b;
  p2b = p4b;

  //mouth horizontal
  if (value > value_previous) {
    for (int i = value_previous; i < value; i++) {
      tft.drawLine(0, tft.height() / 2 - 1 - i, tft.width() - 1, tft.height() / 2 - 1 - i, color1);
      tft.drawLine(0, tft.height() / 2 + i, tft.width() - 1, tft.height() / 2 + i, color1);
    }
  } else if (value < value_previous) {
    for (int i = value_previous; i >= value; i--) {
      tft.drawLine(0, tft.height() / 2 - 1 - i, tft.width() - 1, tft.height() / 2 - 1 - i, ST77XX_BLACK);
      tft.drawLine(0, tft.height() / 2 + i, tft.width() - 1, tft.height() / 2 + i, ST77XX_BLACK);
    }
  }

  value_previous = value;
}
void screen_init() {
  tft.init(240, 240);  // Initialize ST7789 screen
  tft.fillScreen(ST77XX_BLACK);
}
