#include "ssd1306_extend.h"

// コンストラクタ
SSD1306_EX::SSD1306_EX(uint8_t _address, uint8_t _sda, uint8_t _scl, OLEDDISPLAY_GEOMETRY g)
: SSD1306Wire(_address, _sda, _scl, g) {
}

// double型の値を表示する
void SSD1306_EX::drawDouble(int16_t x, int16_t y, double num, int digits, int under_zero) {
  char buf[20];
  dtostrf(num, digits, under_zero, buf);
  drawString(x, y, buf);  
}

// int型の値を表示する
void SSD1306_EX::drawInt(int16_t x, int16_t y, int num, const char* format) {
  char buf[20];
  sprintf(buf, format, num);
  drawString(x, y, buf);  
}

// 横の点線を表示する
void SSD1306_EX::drawDotHLine(int16_t x, int16_t y, int16_t w) {
  for (int i = 0; i < w; i += 2) {
    setPixel(x + i, y);
  }
}

// 縦の点線を表示する
void SSD1306_EX::drawDotVLine(int16_t x, int16_t y, int16_t h) {
  for (int i = 0; i < h; i += 2) {
    setPixel(x, y + i);
  }
}
