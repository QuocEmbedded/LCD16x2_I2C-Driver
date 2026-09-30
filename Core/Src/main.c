#include "uart.h"
#include "i2c.h"
#include "lcd.h"
#include "timer.h"

int main(void)
{
    Timer2_Init();  
    UART1_Init();
    I2C1_Init();
    
    Send_String("Dang khoi tao LCD...\r\n");
    LCD_Init();
    Send_String("LCD san sang! Dang in ky tu...\r\n");

    LCD_Set_Cursor(0, 2);
    LCD_Send_String("mai bao quoc");
    LCD_Set_Cursor(1, 0);
    LCD_Send_String("STM32 Bare-metal");

    while (1)
    {
    }
}