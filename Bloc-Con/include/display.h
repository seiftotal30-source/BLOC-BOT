#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>
#include "config.h"

// 128x64 oled, full frame buffer (1 KB)
#if USE_SSD1309
extern U8G2_SSD1309_128X64_NONAME0_F_HW_I2C u8g2;
#else
extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2;
#endif

#endif
