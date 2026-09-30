#include "stm32f407xx.h"
#include "i2c.h"
#include "lcd.h"
#include "timer.h"

/******************** PRIVATE DEFINITIONS ********************/
#define LCD_BACKLIGHT_PIN  (1U << 3) // P3: Bật đèn nền
#define LCD_EN_PIN         (1U << 2) // P2: Chốt Enable
#define LCD_RS_PIN         (1U << 0) // P0: RS (0 = Lệnh, 1 = Dữ liệu)

static uint8_t s_backlight_state = LCD_BACKLIGHT_PIN;

static void LCD_I2C_Write_Byte(uint8_t data)
{
    //1. Kiem tra truoc khi gui.
    // Ban dau se cho kiem tra tren duong day co dang dinh voi th/bi nao dang lam viec khong
    uint32_t to = I2C_TIMEOUT;
    while ((I2C1->SR2 & SR2_BUSY) && --to);
    
    /*GUI GOI TIN*/
    //2. Gui bit START, nho la phai cho co start bat
    I2C1->CR1 |= CR1_START;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_START) && --to);
    //3. Gui 8 bit tiep theo. bao gom 7 bit addr, 1 bit W/R
    I2C1->DR = (uint8_t)(LCD_ADDR << 1); // 0x4E (Write mode)
    to = I2C_TIMEOUT;
    //4. Neu toi day gui xong nhan dc ACK thi se thoat while, con neu nhan NACK thi ADDR = 0
    // dinh cung o while nay.
    while (!(I2C1->SR1 & SR1_ADDR) && --to);
    //5. Xoa ADDR de nha SCL ra 
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    //6. Gui data
    I2C1->DR = data;
    //7. Cho co bao la da gui xong tren duong truyen o con gi nua, va bat ACK de bao la xong r.
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_BTF) && --to);
    //8. Ket thuc goi tin
    I2C1->CR1 |= CR1_STOP;
}

// nibble & 0xF0 lenh lay 4 bit cao.
//example:
// 'A' = 0100 0001 & (0xF0) = 0100 0000
static void LCD_Write_Nibble(uint8_t nibble, uint8_t rs)
{
    uint8_t data = (nibble & 0xF0) | rs | s_backlight_state;

    LCD_I2C_Write_Byte(data | LCD_EN_PIN);
    Delay_us_TIM2(50);

    LCD_I2C_Write_Byte(data & ~LCD_EN_PIN);
    Delay_us_TIM2(50);
}


void LCD_Send_Cmd(uint8_t cmd)
{
    LCD_Write_Nibble(cmd, 0);        
    LCD_Write_Nibble(cmd << 4, 0);
    Delay_us_TIM2(50);
}

void LCD_Send_Char(char data)
{
    LCD_Write_Nibble(((uint8_t)data), LCD_RS_PIN);        
    LCD_Write_Nibble((((uint8_t)data) << 4), LCD_RS_PIN); 
    Delay_us_TIM2(50);
}

void LCD_Init(void)
{
    Delay_ms_TIM2(50);
    LCD_Write_Nibble(0x30, 0);
    Delay_ms_TIM2(5);
    LCD_Write_Nibble(0x30, 0);
    Delay_us_TIM2(150);
    LCD_Write_Nibble(0x30, 0);
    Delay_us_TIM2(150);

    // Bước 3: Chuyển sang 4-bit mode
    LCD_Write_Nibble(0x20, 0);
    Delay_ms_TIM2(2);

    // Bước 4: Cấu hình hiển thị chuẩn
    LCD_Send_Cmd(0x28); // 4-bit mode, 2 dòng, font 5x8
    Delay_ms_TIM2(1);
    LCD_Send_Cmd(0x08); // Tắt hiển thị
    Delay_ms_TIM2(1);
    LCD_Send_Cmd(0x01); // Xóa màn hình
    Delay_ms_TIM2(2);   // Lệnh Clear tốn tối thiểu 1.52ms
    LCD_Send_Cmd(0x06); // Tự động tăng con trỏ sang phải
    Delay_ms_TIM2(1);
    LCD_Send_Cmd(0x0C); // Bật hiển thị, tắt con trỏ nhấp nháy
    Delay_ms_TIM2(1);
}

void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? (0x80 + col) : (0xC0 + col);
    LCD_Send_Cmd(addr);
}

void LCD_Send_String(const char *str)
{
    while (*str) {
        LCD_Send_Char(*str++);
    }
}

void LCD_Clear(void)
{
    LCD_Send_Cmd(0x01);
    Delay_ms_TIM2(2);
}

void LCD_Backlight(uint8_t state)
{
    if (state) {
        s_backlight_state = LCD_BACKLIGHT_PIN;
    } else {
        s_backlight_state = 0x00;
    }
    LCD_I2C_Write_Byte(s_backlight_state);
}

