// Display Library for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
//
// based on Demo Example from Good Display, available here: https://www.good-display.com/comp/xcompanyFile/downloadNew.do?appId=24&fid=1781&id=1133
// Panel: GDEY0579T93 : https://www.good-display.com/product/439.html
// Controller : SSD1683 : https://v4.cecdn.yun300.cn/100001_1909185148/SSD1683.PDF
//
// Author: Jean-Marc Zingg
//
// Version: see library.properties
//
// Library: https://github.com/ZinggJM/GxEPD2

#include "GxEPD2_579_GDEY0579T93.h"

GxEPD2_579_GDEY0579T93::GxEPD2_579_GDEY0579T93(int16_t cs, int16_t dc, int16_t rst, int16_t busy) :
  GxEPD2_EPD(cs, dc, rst, busy, HIGH, 10000000, WIDTH, HEIGHT, panel, hasColor, hasPartialUpdate, hasFastPartialUpdate)
{
  _refresh_mode = full_refresh;
  _init_4G_done = false;
}

void GxEPD2_579_GDEY0579T93::clearScreen(uint8_t value)
{
  // full refresh needed for all cases (previous != screen)
  _writeScreenBuffer(0x26, value); // set previous
  _writeScreenBuffer(0x24, value); // set current
  refresh(false); // full refresh
  _initial_write = false;
}

void GxEPD2_579_GDEY0579T93::writeScreenBuffer(uint8_t value)
{
  if (_initial_write) return clearScreen(value);
  _writeScreenBuffer(0x24, value); // set current
}

void GxEPD2_579_GDEY0579T93::writeScreenBufferAgain(uint8_t value)
{
  _writeScreenBuffer(0x24, value); // set current
  _writeScreenBuffer(0x26, value); // set previous
}

void GxEPD2_579_GDEY0579T93::_writeScreenBuffer(uint8_t command, uint8_t value)
{
  if (!_init_display_done) _InitDisplay();
  _setPartialRamAreaMaster(0, 0, WIDTH / 2, HEIGHT);
  _writeCommand(command);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH / 2) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
  _setPartialRamAreaMaster(0, HEIGHT / 2, WIDTH / 2, HEIGHT);
  _writeCommand(command);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH / 2) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
  _setPartialRamAreaSlave(0, 0, WIDTH / 2, HEIGHT / 2);
  _writeCommand(command | 0x80);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH / 2) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
  _setPartialRamAreaSlave(0, HEIGHT / 2, WIDTH / 2, HEIGHT);
  _writeCommand(command | 0x80);
  _startTransfer();
  for (uint32_t i = 0; i < uint32_t(WIDTH / 2) * uint32_t(HEIGHT) / 8; i++)
  {
    _transfer(value);
  }
  _endTransfer();
}

void GxEPD2_579_GDEY0579T93::writeImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_refresh_mode == grey_refresh) _writeImage(0x26, bitmap, x, y, w, h, invert, mirror_y, pgm);
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::writeImageForFullRefresh(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x26, bitmap, x, y, w, h, invert, mirror_y, pgm); // set previous
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm); // set current
}


void GxEPD2_579_GDEY0579T93::writeImageAgain(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImage(0x26, bitmap, x, y, w, h, invert, mirror_y, pgm); // set previous
  _writeImage(0x24, bitmap, x, y, w, h, invert, mirror_y, pgm); // set current
}

void GxEPD2_579_GDEY0579T93::_writeImage(uint8_t command, const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  //This is it!
  
  //Serial.print("writeImage("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", ");
  //Serial.print(w); Serial.print(", "); Serial.print(h); Serial.println(")");
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  int16_t wbb = (w + 7) / 8; // width bytes, bitmaps are padded
  x -= x >= 0 ? x % 8 : -x % 8; // byte boundary
  w = wbb * 8; // byte boundary
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx1 = x1 - x; // offset into bitmap
  int16_t dy1 = y1 - y; // offset into bitmap
  w1 -= dx1;
  h1 -= dy1;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
  _writeFromImage(command, bitmap, x1, y1, w1, h1, wbb, h, dx1, dy1, invert, mirror_y, pgm);
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  
}

void GxEPD2_579_GDEY0579T93::_writeFromImage(uint8_t command, const uint8_t bitmap[], int16_t x1, int16_t y1, int16_t w1, int16_t h1,
    int16_t wbb, int16_t hb, int16_t dx, int16_t dy, bool invert, bool mirror_y, bool pgm)
{
  if ((x1 < WIDTH / 2) && (y1 < HEIGHT / 2) && true) // upper left, near connector
  { // memory is on slave controller
    int16_t ws = x1 + w1 <= WIDTH / 2 ? w1 : WIDTH / 2 - x1;
    int16_t hs = y1 + h1 <= HEIGHT / 2 ? h1 : HEIGHT / 2 - y1;
    //Serial.print("upper left ws="); Serial.print(ws); Serial.print(" hs="); Serial.print(hs); Serial.print(" dx="); Serial.print(dx); Serial.print(" dy="); Serial.println(dy);
    _setPartialRamAreaSlave(x1, y1, ws, hs, 0x03);
    _writeCommand(command | 0x80);
    _writeDataFromImage(bitmap, ws, hs, wbb, hb, dx, dy, invert, mirror_y, pgm);
  }

  if (((x1 + w1) > WIDTH / 2) && (y1 < HEIGHT / 2) && true) // upper right, opposite to connector
  { // memory is on master controller
    int16_t wm = x1 >= WIDTH / 2 ? w1 : (x1 + w1) - WIDTH / 2;
    int16_t hm = y1 + h1 <= HEIGHT / 2 ? h1 : HEIGHT / 2 - y1;
    int16_t xm = x1 < WIDTH / 2 ? WIDTH / 2 - wm : WIDTH - x1 - wm;
    int16_t ym = y1 < HEIGHT / 2 ? HEIGHT / 2 - hm : HEIGHT - y1 - hm;
    //Serial.print("upper right wm="); Serial.print(wm); Serial.print(" hm="); Serial.print(hm); Serial.print(" dx="); Serial.print(dx); Serial.print(" dy="); Serial.println(dy);
    _setPartialRamAreaMaster(xm, y1, wm, hm, 0x02);
    _writeCommand(command);
    _writeDataFromImage(bitmap, wm, hm, wbb, hb, dx + w1 - wm, dy, invert, mirror_y, pgm);
  }

  if ((x1 < WIDTH / 2) && ((y1 + h1) > HEIGHT / 2) && true) // lower left, near connector
  { // memory is on slave controller
    int16_t ws = x1 + w1 <= WIDTH / 2 ? w1 : WIDTH / 2 - x1;
    int16_t hs = y1 >= HEIGHT / 2 ? h1 : (y1 + h1) - HEIGHT / 2;
    int16_t ys = y1 >= HEIGHT / 2 ? y1 : HEIGHT / 2;
    //Serial.print("lower left ws="); Serial.print(ws); Serial.print(" hs="); Serial.print(hs); Serial.print(" dx="); Serial.print(dx); Serial.print(" dy="); Serial.println(dy);
    _setPartialRamAreaSlave(x1, ys, ws, hs, 0x03);
    _writeCommand(command | 0x80);
    _writeDataFromImage(bitmap, ws, hs, wbb, hb, dx, dy + h1 - hs, invert, mirror_y, pgm);
  }

  if (((x1 + w1) > WIDTH / 2) && ((y1 + h1) > HEIGHT / 2) && true) // lower right, opposite to connector
  { // memory is on master controller
    int16_t wm = x1 >= WIDTH / 2 ? w1 : (x1 + w1) - WIDTH / 2;
    int16_t hm = y1 >= HEIGHT / 2 ? h1 : (y1 + h1) - HEIGHT / 2;
    int16_t xm = x1 < WIDTH / 2 ? WIDTH / 2 - wm : WIDTH - x1 - wm;
    int16_t ym = y1 >= HEIGHT / 2 ? y1 : HEIGHT / 2;
    //Serial.print("lower right wm="); Serial.print(wm); Serial.print(" hm="); Serial.print(hm); Serial.print(" dx="); Serial.print(dx); Serial.print(" dy="); Serial.println(dy);
    _setPartialRamAreaMaster(xm, ym, wm, hm, 0x02);
    _writeCommand(command);
    _writeDataFromImage(bitmap, wm, hm, wbb, hb, dx + w1 - wm, dy + h1 - hm, invert, mirror_y, pgm);
  }
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_579_GDEY0579T93::_writeDataFromImage(const uint8_t bitmap[], int16_t w, int16_t h, int16_t wbb, int16_t hb, int16_t dx, int16_t dy, bool invert, bool mirror_y, bool pgm)
{
  _startTransfer();
  for (int16_t i = 0; i < h; i++)
  {
    for (int16_t j = 0; j < w; j += 8)
    {
      uint8_t data;
      // use wbb, h of bitmap for index!
      int16_t idx = mirror_y ? ((j + dx) / 8 + ((hb - 1 - (i + dy))) * wbb) : ((j + dx) / 8 + (i + dy) * wbb);
      if (pgm)
      {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
        data = pgm_read_byte(&bitmap[idx]);
#else
        data = bitmap[idx];
#endif
      }
      else
      {
        data = bitmap[idx];
      }
      if (invert) data = ~data;
      _transfer(data);
    }
  }
  _endTransfer();
}

void GxEPD2_579_GDEY0579T93::writeImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (_refresh_mode == grey_refresh) _writeImagePart(0x26, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  _writeImagePart(0x24, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::writeImagePartAgain(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  _writeImagePart(0x24, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  _writeImagePart(0x26, bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::_writeImagePart(uint8_t command, const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  //Serial.print("writeImagePart("); Serial.print(x_part); Serial.print(", "); Serial.print(y_part); Serial.print(", ");
  //Serial.print(w_bitmap); Serial.print(", "); Serial.print(h_bitmap); Serial.print(", ");
  //Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", ");
  //Serial.print(w); Serial.print(", "); Serial.print(h); Serial.println(")");
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;
  int16_t wb_bitmap = (w_bitmap + 7) / 8; // width bytes, bitmaps are padded
  x_part -= x_part % 8; // byte boundary
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
  x -= x % 8; // byte boundary
  w = 8 * ((w + 7) / 8); // byte boundary, bitmaps are padded
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  int16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  int16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_init_display_done) _InitDisplay();
  if (_initial_write) writeScreenBuffer(); // initial full screen buffer clean
  _writeFromImage(command, bitmap, x1, y1, w1, h1, wb_bitmap, h_bitmap, dx + x_part, dy + y_part, invert, mirror_y, pgm);
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

// Added 4G (4 grayscale) methods
void GxEPD2_579_GDEY0579T93::writeImage_4G(const uint8_t bitmap[], uint8_t bpp, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  uint16_t ppb = (bpp == 2 ? 4 : (bpp == 4 ? 2 : (bpp == 8 ? 1 : 0)));
  uint8_t mask = (bpp == 2 ? 0xC0 : (bpp == 4 ? 0xF0 : 0xFF));
  uint8_t grey1 = (bpp == 2 ? 0x80 : 0xA0); // demo limit for 4bpp
  if (ppb == 0) return;
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  int16_t wbc = (w + 7) / 8; // width bytes on controller
  x -= x >= 0 ? x % 8 : -x % 8; // byte boundary on controller
  w = wbc * 8; // byte boundary on controller
  int16_t wb = (w + ppb - 1) / ppb; // width bytes of bitmap, bitmaps are padded
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  uint16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  uint16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  
  if (!_init_4G_done) _Init_4G();
  
  _writeImage_4G_FromBitmap(0x26, bitmap, bpp, x1, y1, w1, h1, wb, h, dx, dy, invert, mirror_y, pgm, mask, grey1, ppb);
  _writeImage_4G_FromBitmap(0x24, bitmap, bpp, x1, y1, w1, h1, wb, h, dx, dy, invert, mirror_y, pgm, mask, grey1, ppb);
  
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  
}

void GxEPD2_579_GDEY0579T93::_writeImage_4G_FromBitmap(uint8_t command, const uint8_t bitmap[], uint8_t bpp, int16_t x, int16_t y, int16_t w, int16_t h, 
                                                    int16_t wb, int16_t hb, int16_t dx, int16_t dy, bool invert, bool mirror_y, bool pgm,
                                                    uint8_t mask, uint8_t grey1, uint16_t ppb)
{
  if ((x < WIDTH / 2) && (y < HEIGHT / 2)) // upper left, near connector
  { // memory is on slave controller
    int16_t ws = x + w <= WIDTH / 2 ? w : WIDTH / 2 - x;
    int16_t hs = y + h <= HEIGHT / 2 ? h : HEIGHT / 2 - y;
    _setPartialRamAreaSlave(x, y, ws, hs, 0x03);
    _writeCommand(command | 0x80);
    _startTransfer();
    for (uint16_t i = 0; i < hs; i++) // lines
    {
      for (uint16_t j = 0; j < ws / ppb; j += bpp) // out bytes
      {
        uint8_t out_byte = 0;
        for (uint16_t k = 0; k < bpp; k++) // in bytes (bpp per out byte)
        {
          uint8_t in_byte;
          // use wb, hb of bitmap for index!
          uint32_t idx = mirror_y ? j + k + dx / ppb + uint32_t((hb - 1 - (i + dy))) * wb : j + k + dx / ppb + uint32_t(i + dy) * wb;
          if (pgm)
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            in_byte = pgm_read_byte(&bitmap[idx]);
#else
            in_byte = bitmap[idx];
#endif
          }
          else
          {
            in_byte = bitmap[idx];
          }
          if (invert) in_byte = ~in_byte;
          for (uint16_t n = 0; n < ppb; n++) // bits, nibbles (ppb per in byte)
          {
            out_byte <<= 1;
            uint8_t nibble = in_byte & mask;
            if (nibble == mask) out_byte |= 0x01;//white
            else if (nibble == 0x00) out_byte |= 0x00;  //black
            else if (nibble >= grey1) out_byte |= (command == 0x26 ? 0x01 : 0x00);  //gray1
            else out_byte |= (command == 0x26 ? 0x00 : 0x01); //gray2
            in_byte <<= bpp;
          }
        }
        _transfer(~out_byte);
      }
    }
    _endTransfer();
  }

  if (((x + w) > WIDTH / 2) && (y < HEIGHT / 2)) // upper right, opposite to connector
  { // memory is on master controller
    int16_t wm = x >= WIDTH / 2 ? w : (x + w) - WIDTH / 2;
    int16_t hm = y + h <= HEIGHT / 2 ? h : HEIGHT / 2 - y;
    int16_t xm = x < WIDTH / 2 ? WIDTH / 2 - wm : WIDTH - x - wm;
    int16_t ym = y < HEIGHT / 2 ? HEIGHT / 2 - hm : HEIGHT - y - hm;
    _setPartialRamAreaMaster(xm, y, wm, hm, 0x02);
    _writeCommand(command);
    _startTransfer();
    for (uint16_t i = 0; i < hm; i++) // lines
    {
      for (uint16_t j = 0; j < wm / ppb; j += bpp) // out bytes
      {
        uint8_t out_byte = 0;
        for (uint16_t k = 0; k < bpp; k++) // in bytes (bpp per out byte)
        {
          uint8_t in_byte;
          // use wb, hb of bitmap for index!
          uint32_t idx = mirror_y ? j + k + (dx + w - wm) / ppb + uint32_t((hb - 1 - (i + dy))) * wb : j + k + (dx + w - wm) / ppb + uint32_t(i + dy) * wb;
          if (pgm)
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            in_byte = pgm_read_byte(&bitmap[idx]);
#else
            in_byte = bitmap[idx];
#endif
          }
          else
          {
            in_byte = bitmap[idx];
          }
          if (invert) in_byte = ~in_byte;
          for (uint16_t n = 0; n < ppb; n++) // bits, nibbles (ppb per in byte)
          {
            out_byte <<= 1;
            uint8_t nibble = in_byte & mask;
            if (nibble == mask) out_byte |= 0x01;//white
            else if (nibble == 0x00) out_byte |= 0x00;  //black
            else if (nibble >= grey1) out_byte |= (command == 0x26 ? 0x01 : 0x00);  //gray1
            else out_byte |= (command == 0x26 ? 0x00 : 0x01); //gray2
            in_byte <<= bpp;
          }
        }
        _transfer(~out_byte);
      }
    }
    _endTransfer();
  }

  if ((x < WIDTH / 2) && ((y + h) > HEIGHT / 2)) // lower left, near connector
  { // memory is on slave controller
    int16_t ws = x + w <= WIDTH / 2 ? w : WIDTH / 2 - x;
    int16_t hs = y >= HEIGHT / 2 ? h : (y + h) - HEIGHT / 2;
    int16_t ys = y >= HEIGHT / 2 ? y : HEIGHT / 2;
    _setPartialRamAreaSlave(x, ys, ws, hs, 0x03);
    _writeCommand(command | 0x80);
    _startTransfer();
    for (uint16_t i = 0; i < hs; i++) // lines
    {
      for (uint16_t j = 0; j < ws / ppb; j += bpp) // out bytes
      {
        uint8_t out_byte = 0;
        for (uint16_t k = 0; k < bpp; k++) // in bytes (bpp per out byte)
        {
          uint8_t in_byte;
          // use wb, hb of bitmap for index!
          uint32_t idx = mirror_y ? j + k + dx / ppb + uint32_t((hb - 1 - (i + dy + h - hs))) * wb : j + k + dx / ppb + uint32_t(i + dy + h - hs) * wb;
          if (pgm)
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            in_byte = pgm_read_byte(&bitmap[idx]);
#else
            in_byte = bitmap[idx];
#endif
          }
          else
          {
            in_byte = bitmap[idx];
          }
          if (invert) in_byte = ~in_byte;
          for (uint16_t n = 0; n < ppb; n++) // bits, nibbles (ppb per in byte)
          {
            out_byte <<= 1;
            uint8_t nibble = in_byte & mask;
            if (nibble == mask) out_byte |= 0x01;//white
            else if (nibble == 0x00) out_byte |= 0x00;  //black
            else if (nibble >= grey1) out_byte |= (command == 0x26 ? 0x01 : 0x00);  //gray1
            else out_byte |= (command == 0x26 ? 0x00 : 0x01); //gray2
            in_byte <<= bpp;
          }
        }
        _transfer(~out_byte);
      }
    }
    _endTransfer();
  }

  if (((x + w) > WIDTH / 2) && ((y + h) > HEIGHT / 2)) // lower right, opposite to connector
  { // memory is on master controller
    int16_t wm = x >= WIDTH / 2 ? w : (x + w) - WIDTH / 2;
    int16_t hm = y >= HEIGHT / 2 ? h : (y + h) - HEIGHT / 2;
    int16_t xm = x < WIDTH / 2 ? WIDTH / 2 - wm : WIDTH - x - wm;
    int16_t ym = y >= HEIGHT / 2 ? y : HEIGHT / 2;
    _setPartialRamAreaMaster(xm, ym, wm, hm, 0x02);
    _writeCommand(command);
    _startTransfer();
    for (uint16_t i = 0; i < hm; i++) // lines
    {
      for (uint16_t j = 0; j < wm / ppb; j += bpp) // out bytes
      {
        uint8_t out_byte = 0;
        for (uint16_t k = 0; k < bpp; k++) // in bytes (bpp per out byte)
        {
          uint8_t in_byte;
          // use wb, hb of bitmap for index!
          uint32_t idx = mirror_y ? j + k + (dx + w - wm) / ppb + uint32_t((hb - 1 - (i + dy + h - hm))) * wb : j + k + (dx + w - wm) / ppb + uint32_t(i + dy + h - hm) * wb;
          if (pgm)
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            in_byte = pgm_read_byte(&bitmap[idx]);
#else
            in_byte = bitmap[idx];
#endif
          }
          else
          {
            in_byte = bitmap[idx];
          }
          if (invert) in_byte = ~in_byte;
          for (uint16_t n = 0; n < ppb; n++) // bits, nibbles (ppb per in byte)
          {
            out_byte <<= 1;
            uint8_t nibble = in_byte & mask;
            if (nibble == mask) out_byte |= 0x01;//white
            else if (nibble == 0x00) out_byte |= 0x00;  //black
            else if (nibble >= grey1) out_byte |= (command == 0x26 ? 0x01 : 0x00);  //gray1
            else out_byte |= (command == 0x26 ? 0x00 : 0x01); //gray2
            in_byte <<= bpp;
          }
        }
        _transfer(~out_byte);
      }
    }
    _endTransfer();
  }
  
}

void GxEPD2_579_GDEY0579T93::writeImagePart_4G(const uint8_t bitmap[], uint8_t bpp, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  uint16_t ppb = (bpp == 2 ? 4 : (bpp == 4 ? 2 : (bpp == 8 ? 1 : 0)));
  uint8_t mask = (bpp == 2 ? 0xC0 : (bpp == 4 ? 0xF0 : 0xFF));
  uint8_t grey1 = (bpp == 2 ? 0x80 : 0xA0); // demo limit for 4bpp
  if (ppb == 0) return;
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
  if ((w_bitmap < 0) || (h_bitmap < 0) || (w < 0) || (h < 0)) return;
  if ((x_part < 0) || (x_part >= w_bitmap)) return;
  if ((y_part < 0) || (y_part >= h_bitmap)) return;
  int16_t wb_bitmap = (w_bitmap + ppb - 1) / ppb; // width bytes, bitmaps are padded
  x_part -= x_part % ppb; // byte boundary
  w = w_bitmap - x_part < w ? w_bitmap - x_part : w; // limit
  h = h_bitmap - y_part < h ? h_bitmap - y_part : h; // limit
  x -= x % ppb; // byte boundary
  w = ppb * ((w + ppb - 1) / ppb); // byte boundary, bitmaps are padded
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  uint16_t w1 = x + w < int16_t(WIDTH) ? w : int16_t(WIDTH) - x; // limit
  uint16_t h1 = y + h < int16_t(HEIGHT) ? h : int16_t(HEIGHT) - y; // limit
  int16_t dx = x1 - x;
  int16_t dy = y1 - y;
  w1 -= dx;
  h1 -= dy;
  if ((w1 <= 0) || (h1 <= 0)) return;
  if (!_init_4G_done) _Init_4G();

  _writeImagePart_4G_FromBitmap(0x26, bitmap, bpp, x_part, y_part, w_bitmap, h_bitmap, x1, y1, w1, h1, wb_bitmap, h_bitmap, dx, dy, invert, mirror_y, pgm, mask, grey1, ppb);
  _writeImagePart_4G_FromBitmap(0x24, bitmap, bpp, x_part, y_part, w_bitmap, h_bitmap, x1, y1, w1, h1, wb_bitmap, h_bitmap, dx, dy, invert, mirror_y, pgm, mask, grey1, ppb);
  
  delay(1); // yield() to avoid WDT on ESP8266 and ESP32
}

void GxEPD2_579_GDEY0579T93::_writeImagePart_4G_FromBitmap(uint8_t command, const uint8_t bitmap[], uint8_t bpp, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, int16_t wb_bitmap, int16_t hb, int16_t dx, int16_t dy, bool invert, bool mirror_y, bool pgm,
    uint8_t mask, uint8_t grey1, uint16_t ppb)
{
  if ((x < WIDTH / 2) && (y < HEIGHT / 2)) // upper left, near connector
  { // memory is on slave controller
    int16_t ws = x + w <= WIDTH / 2 ? w : WIDTH / 2 - x;
    int16_t hs = y + h <= HEIGHT / 2 ? h : HEIGHT / 2 - y;
    _setPartialRamAreaSlave(x, y, ws, hs, 0x03);
    _writeCommand(command | 0x80);
    _startTransfer();
    for (uint16_t i = 0; i < hs; i++) // lines
    {
      for (uint16_t j = 0; j < ws / ppb; j += bpp) // out bytes
      {
        uint8_t out_byte = 0;
        for (uint16_t k = 0; k < bpp; k++) // in bytes (bpp per out byte)
        {
          uint8_t in_byte;
          // use wb_bitmap, h_bitmap of bitmap for index!
          uint32_t idx = mirror_y ? x_part / ppb + j + k + dx / ppb + uint32_t((hb - 1 - (y_part + i + dy))) * wb_bitmap : x_part / ppb + j + k + dx / ppb + uint32_t(y_part + i + dy) * wb_bitmap;
          if (pgm)
          {
#if defined(__AVR) || defined(ESP8266) || defined(ESP32)
            in_byte = pgm_read_byte(&bitmap[idx]);
#else
            in_byte = bitmap[idx];
#endif
          }
          else
          {
            in_byte = bitmap[idx];
          }
          if (invert) in_byte = ~in_byte;
          for (uint16_t n = 0; n < ppb; n++) // bits, nibbles (ppb per in byte)
          {
            out_byte <<= 1;
            uint8_t nibble = in_byte & mask;
            if (nibble == mask) out_byte |= 0x01;//white
            else if (nibble == 0x00) out_byte |= 0x00;  //black
            else if (nibble >= grey1) out_byte |= (command == 0x26 ? 0x01 : 0x00);  //gray1
            else out_byte |= (command == 0x26 ? 0x00 : 0x01); //gray2
            in_byte <<= bpp;
          }
        }
        _transfer(~out_byte);
      }
    }
    _endTransfer();
  }

  // Complete similar implementations for the other three quadrants as needed
  // ...
}

void GxEPD2_579_GDEY0579T93::writeImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  /*
  if (black)
  {
    writeImage(black, x, y, w, h, invert, mirror_y, pgm);
  }
  */

  writeImage_4G(black, 4, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::writeImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (black)
  {
    writeImagePart(black, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_579_GDEY0579T93::writeNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1)
  {
    writeImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_579_GDEY0579T93::drawImage(const uint8_t bitmap[], int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage(bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
  writeImageAgain(bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::drawImage_4G(const uint8_t bitmap[], uint8_t bpp, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImage_4G(bitmap, bpp, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_579_GDEY0579T93::drawImagePart(const uint8_t bitmap[], int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart(bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
  writeImagePartAgain(bitmap, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
}

void GxEPD2_579_GDEY0579T93::drawImagePart_4G(const uint8_t bitmap[], uint8_t bpp, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  writeImagePart_4G(bitmap, bpp, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  refresh(x, y, w, h);
}

void GxEPD2_579_GDEY0579T93::drawImage(const uint8_t* black, const uint8_t* color, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (black)
  {
    drawImage(black, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_579_GDEY0579T93::drawImagePart(const uint8_t* black, const uint8_t* color, int16_t x_part, int16_t y_part, int16_t w_bitmap, int16_t h_bitmap,
    int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (black)
  {
    drawImagePart(black, x_part, y_part, w_bitmap, h_bitmap, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_579_GDEY0579T93::drawNative(const uint8_t* data1, const uint8_t* data2, int16_t x, int16_t y, int16_t w, int16_t h, bool invert, bool mirror_y, bool pgm)
{
  if (data1)
  {
    drawImage(data1, x, y, w, h, invert, mirror_y, pgm);
  }
}

void GxEPD2_579_GDEY0579T93::refresh(bool partial_update_mode)
{
  if (partial_update_mode) refresh(0, 0, WIDTH, HEIGHT);
  else
  {
    if (_refresh_mode == forced_full_refresh) _refresh_mode = full_refresh;
    if (_refresh_mode == fast_refresh) _InitDisplay();
    if (_refresh_mode == grey_refresh) _Update_4G();
    else
    {
      _setPartialRamAreaSlave(0, 0, WIDTH / 2, HEIGHT);
      _setPartialRamAreaMaster(0, 0, WIDTH / 2, HEIGHT);
      _Update_Full();
    }
    _initial_refresh = false; // initial full update done
  }
}

void GxEPD2_579_GDEY0579T93::refresh(int16_t x, int16_t y, int16_t w, int16_t h)
{
  if (_initial_refresh) return refresh(false); // initial update needs be full update
  if (_refresh_mode == forced_full_refresh) return refresh(false);
  // intersection with screen
  int16_t w1 = x < 0 ? w + x : w; // reduce
  int16_t h1 = y < 0 ? h + y : h; // reduce
  int16_t x1 = x < 0 ? 0 : x; // limit
  int16_t y1 = y < 0 ? 0 : y; // limit
  w1 = x1 + w1 < int16_t(WIDTH) ? w1 : int16_t(WIDTH) - x1; // limit
  h1 = y1 + h1 < int16_t(HEIGHT) ? h1 : int16_t(HEIGHT) - y1; // limit
  if ((w1 <= 0) || (h1 <= 0)) return;
  // make x1, w1 multiple of 8
  w1 += x1 % 8;
  if (w1 % 8 > 0) w1 += 8 - w1 % 8;
  x1 -= x1 % 8;
  if (_refresh_mode == grey_refresh) _Update_4G();
  else
  {
    _setPartialRamAreaSlave(0, 0, WIDTH / 2, HEIGHT);
    _setPartialRamAreaMaster(0, 0, WIDTH / 2, HEIGHT);
    _Update_Part();
  }
}

void GxEPD2_579_GDEY0579T93::powerOff()
{
  _PowerOff();
}

void GxEPD2_579_GDEY0579T93::hibernate()
{
  _PowerOff();
  if (_rst >= 0)
  {
    _writeCommand(0x10); // deep sleep mode
    _writeData(0x1);     // enter deep sleep
    _hibernating = true;
    _init_display_done = false;
    _init_4G_done = false;
  }
}

void GxEPD2_579_GDEY0579T93::_setPartialRamAreaMaster(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode)
{
  //Serial.print("_setPartialRamAreaMaster("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", ");
  //Serial.print(w); Serial.print(", "); Serial.print(h); Serial.println(")");
  _setPartialRamArea(x, y, w, h, mode, 0x00);
}

void GxEPD2_579_GDEY0579T93::_setPartialRamAreaSlave(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode)
{
  //Serial.print("_setPartialRamAreaSlave("); Serial.print(x); Serial.print(", "); Serial.print(y); Serial.print(", ");
  //Serial.print(w); Serial.print(", "); Serial.print(h); Serial.println(")");
  _setPartialRamArea(x, y, w, h, mode, 0x80);
}

void GxEPD2_579_GDEY0579T93::_setPartialRamArea(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t mode, uint8_t target)
{
  _writeCommand(0x11 | target); // set ram entry mode
  _writeData(mode);
  switch (mode)
  {
    case 0x00: // x decrement, y decrement
      _writeCommand(0x44 | target);
      _writeData((x + w - 1) / 8);
      _writeData(x / 8);
      _writeCommand(0x45 | target);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      _writeData(y % 256);
      _writeData(y / 256);
      _writeCommand(0x4e | target);
      _writeData((x + w - 1) / 8);
      _writeCommand(0x4f | target);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      break;
    case 0x01: // x increment, y decrement
      _writeCommand(0x44 | target);
      _writeData(x / 8);
      _writeData((x + w - 1) / 8);
      _writeCommand(0x45 | target);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      _writeData(y % 256);
      _writeData(y / 256);
      _writeCommand(0x4e | target);
      _writeData(x / 8);
      _writeCommand(0x4f | target);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      break;
    case 0x02: // x decrement, y increment
      _writeCommand(0x44 | target);
      _writeData((x + w - 1) / 8);
      _writeData(x / 8);
      _writeCommand(0x45 | target);
      _writeData(y % 256);
      _writeData(y / 256);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      _writeCommand(0x4e | target);
      _writeData((x + w - 1) / 8);
      _writeCommand(0x4f | target);
      _writeData(y % 256);
      _writeData(y / 256);
      break;
    case 0x03: // x increment, y increment
      _writeCommand(0x44 | target);
      _writeData(x / 8);
      _writeData((x + w - 1) / 8);
      _writeCommand(0x45 | target);
      _writeData(y % 256);
      _writeData(y / 256);
      _writeData((y + h - 1) % 256);
      _writeData((y + h - 1) / 256);
      _writeCommand(0x4e | target);
      _writeData(x / 8);
      _writeCommand(0x4f | target);
      _writeData(y % 256);
      _writeData(y / 256);
      break;
  }
  delay(2);
}

void GxEPD2_579_GDEY0579T93::_PowerOn()
{
  if (!_power_is_on)
  {
    _writeCommand(0x22);
    _writeData(0xe0);
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOn", power_on_time);
  }
  _power_is_on = true;
}

void GxEPD2_579_GDEY0579T93::_PowerOff()
{
  if (_power_is_on)
  {
    _writeCommand(0x22);
    _writeData(0x83);
    _writeCommand(0x20);
    _waitWhileBusy("_PowerOff", power_off_time);
  }
  _power_is_on = false;
  _using_partial_mode = false;
}

void GxEPD2_579_GDEY0579T93::_InitDisplay()
{
  if (_hibernating) _reset();
  _writeCommand(0x12);  //SWRESET
  delay(10); // 10ms according to specs, 4ms measured
  _writeCommand(0x18); // Temperature Sensor Control
  _writeData(0x80);    // Internal temperature sensor
  _writeCommand(0x22); // Display Update Control 2
  //_writeData(0xB1);    // Enable clock signal, Load temperature value, Load LUT (3-color mode), Disable clock signal
  _writeData(0xB1);    // Enable clock signal, Load temperature value, Load LUT (black/white mode), Disable clock signal
  _writeCommand(0x20); // Master Activation
  delay(10); // 4ms measured
  _writeCommand(0x1A); // Write to temperature register
  _writeData(0x64);
  _writeData(0x00);
  _writeCommand(0x22); // Display Update Control 2
  _writeData(0x91);    // Load temperature value, Load LUT (black/white mode), Disable clock signal
  _writeCommand(0x20); // Master Activation
  delay(10); // 4ms measured
  _init_display_done = true;
  _init_4G_done = false;
  _refresh_mode = full_refresh;
}

// full screen update LUT 0~3 gray
const unsigned char GxEPD2_579_GDEY0579T93::lut_4G[] PROGMEM =
{
  0x01,  0x0A, 0x1B, 0x0F, 0x03, 0x01, 0x01,
  0x05, 0x0A, 0x01, 0x0A, 0x01, 0x01, 0x01,
  0x05, 0x08, 0x03, 0x02, 0x04, 0x01, 0x01,
  0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x0A, 0x1B, 0x0F, 0x03, 0x01, 0x01,
  0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
  0x05, 0x48, 0x03, 0x82, 0x84, 0x01, 0x01,
  0x01, 0x84, 0x84, 0x82, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x0A, 0x1B, 0x8F, 0x03, 0x01, 0x01,
  0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
  0x05, 0x48, 0x83, 0x82, 0x04, 0x01, 0x01,
  0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x8A, 0x1B, 0x8F, 0x03, 0x01, 0x01,
  0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
  0x05, 0x48, 0x83, 0x02, 0x04, 0x01, 0x01,
  0x01, 0x04, 0x04, 0x02, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x8A, 0x9B, 0x8F, 0x03, 0x01, 0x01,
  0x05, 0x4A, 0x01, 0x8A, 0x01, 0x01, 0x01,
  0x05, 0x48, 0x03, 0x42, 0x04, 0x01, 0x01,
  0x01, 0x04, 0x04, 0x42, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x07, 0x17, 0x41, 0xA8,
  0x32, 0x30,
};

void GxEPD2_579_GDEY0579T93::_Init_4G()
{
  if (_hibernating) _reset();
  delay(10); // 10ms according to specs
  _writeCommand(0x12);  //SWRESET
  delay(10); // 10ms according to specs
  _writeCommand(0x0C); //set soft start
  _writeData(0x8B);
  _writeData(0x9C);
  _writeData(0xA4);
  _writeData(0x0F);
  _writeCommand(0x21);
  _writeData(0x00);
  _writeData(0x00);
  _writeCommand(0x3C); // Border setting
  _writeData(0x03);
  
  // Set both master and slave RAM areas
  _setPartialRamAreaMaster(0, 0, WIDTH / 2, HEIGHT);
  _setPartialRamAreaSlave(0, 0, WIDTH / 2, HEIGHT);
  
  _writeCommand(0x32);
  _writeDataPGM(lut_4G, 227);
  _writeCommand(0x3F);
  //_writeData(lut_4G[227]); //incriminating?
  _writeCommand(0x03);
  _writeData(lut_4G[228]);//VGH
  _writeCommand(0x04);
  _writeData(lut_4G[229]);//VSH1
  _writeData(lut_4G[230]);//VSH2
  _writeData(lut_4G[231]);//VSL
  _writeCommand(0x2c);
  _writeData(lut_4G[232]);//VCOM
  
  // Clear both master and slave buffers
  _writeScreenBuffer(0x24, 0x00); // set current
  _writeScreenBuffer(0x26, 0x00); // set previous
  
  _initial_write = false;
  _init_display_done = false;
  _init_4G_done = true;
  _refresh_mode = grey_refresh;
}

void GxEPD2_579_GDEY0579T93::_Update_Full()
{
  _writeCommand(0x21); // Display Update Control
  _writeData(0x40);    // bypass RED as 0
  _writeData(0x10);    // cascade application
  if (useFastFullUpdate)
  {
    _writeCommand(0x1A); // Write to temperature register
    _writeData(0x64);
    _writeData(0x00);
    _writeCommand(0x22); // Display Update Control 2
    _writeData(0xd7);
  }
  else
  {
    _writeCommand(0x22); // Display Update Control 2
    _writeData(0xF7);
  }
  _writeCommand(0x20); // Master Activation
  _waitWhileBusy("_Update_Full", full_refresh_time);
  _power_is_on = false;
}

void GxEPD2_579_GDEY0579T93::_Update_4G()
{
  _writeCommand(0x21); // Display Update Control
  _writeData(0x88);    // b/w inverted, RED inverted
  _writeData(0x10);    // cascade application
  _writeCommand(0x22);
  _writeData(0xcf);
  _writeCommand(0x20);
  _waitWhileBusy("_Update_4G", full_refresh_time);
  _power_is_on = false;
}

void GxEPD2_579_GDEY0579T93::_Update_Part()
{
  _writeCommand(0x3C); // Border Waveform Control
  _writeData(0x80);    // VCOM
  _writeCommand(0x21); // Display Update Control
  _writeData(0x00);    // RED normal
  _writeData(0x10);    // cascade application
  _writeCommand(0x22); // Display Update Control 2
  _writeData(0xFF);
  _writeCommand(0x20);
  _waitWhileBusy("_Update_Part", partial_refresh_time);
  _power_is_on = false;
}

void GxEPD2_579_GDEY0579T93::drawGreyLevels()
{
  if (!_init_4G_done) _Init_4G();
  
  // Draw 4 blocks with different gray levels
  _writeCommand(0x24);
  
  // Draw first quarter with black (0x00)
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0x00);
  }
  
  // Draw second quarter with dark gray (0xFF for 0x24 and 0x00 for 0x26)
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0xFF);
  }
  
  // Draw third quarter with light gray (0x00 for 0x24 and 0xFF for 0x26)
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0x00);
  }
  
  // Draw fourth quarter with white (0xFF)
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0xFF);
  }
  
  _writeCommand(0x26);
  
  // Black area - both buffers are 0x00
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0x00);
  }
  
  // Dark gray - 0x24 buffer is 0xFF, 0x26 buffer is 0x00
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0x00);
  }
  
  // Light gray - 0x24 buffer is 0x00, 0x26 buffer is 0xFF
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0xFF);
  }
  
  // White area - both buffers are 0xFF
  for (uint32_t i = 0; i < uint32_t(WIDTH) * uint32_t(HEIGHT) / 32; i++)
  {
    _writeData(0xFF);
  }
  
  _Update_4G();
}