#ifndef _SSD1306_EXTEND_H_
#define _SSD1306_EXTEND_H_
#include <SSD1306Wire.h>

class SSD1306_EX : public SSD1306Wire {
  public:
    SSD1306_EX(uint8_t _address, uint8_t _sda, uint8_t _scl, OLEDDISPLAY_GEOMETRY g = GEOMETRY_128_64);
    void drawDouble(int16_t x, int16_t y, double num, int digits, int under_zero);
    void drawInt(int16_t x, int16_t y, int num, const char* format);
    void drawDotHLine(int16_t x, int16_t y, int16_t w);
    void drawDotVLine(int16_t x, int16_t y, int16_t h);
};
#endif
