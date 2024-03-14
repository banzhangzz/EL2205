#ifndef _OLED_H
#define _OLED_H

#include <cstdio>
#include "mbed.h"
#include "oledfont.h"

void OLED_Init(void);
void OLED_Fill(uint8_t fill_Data);
void OLED_Clear(void);
void OLED_ShowCH(uint8_t x, uint8_t y,char *chs);
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len,uint8_t mode);
void OLED_DrawBMP(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);
void dispTemp(int temp);
void dispAcc(int acc);
void dispHR(int hr);

#endif
