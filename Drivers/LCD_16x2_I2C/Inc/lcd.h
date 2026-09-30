#ifndef LCD_H_
#define LCD_H_
#include "stm32f407xx.h"

#define LCD_ADDR 0x27
void LCD_Init(void);

void LCD_Send_Cmd(uint8_t cmd);
void LCD_Send_Char(char data);
void LCD_Send_String(const char *str);
void LCD_Set_Cursor(uint8_t row, uint8_t col);
void LCD_Clear(void);
void LCD_Backlight(uint8_t state);

#endif 