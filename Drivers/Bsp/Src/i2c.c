#include "i2c.h"
#include "uart.h"


void I2C1_Init()
{
    // I. Cau hinh chan PB6(SCL), PB7(SDA)
    RCC->AHB1ENR  |=  (1U<<1);

    GPIOB->MODER &= ~(3U<<12);
    GPIOB->MODER  |= (2U<<12);
    GPIOB->MODER &= ~(3U<<14);
    GPIOB->MODER  |= (2U<<14);

    GPIOB->OTYPER |=  (1U<<6);
    GPIOB->OTYPER |=  (1U<<7);

    GPIOB->PUPDR  |= (1U<<12);
    GPIOB->PUPDR  |= (1U<<14);
 
    GPIOB->AFR[0] |=  (4U<<24);
    GPIOB->AFR[0] |= (4U<<28);

    // II. Cau hinh I2C
    RCC->APB1ENR  |= (1U<<21);
    I2C1->CR1     |= (1U<<15); // bat reset I2C
    I2C1->CR1    &= ~(1U<<15); // xoa reset I2C
    
    I2C1->CR2      = (0x2A); // set 42Mhz
    I2C1->CCR      = (0xD2); // chay o 100kHZ
    I2C1->TRISE    = (0x2B); // chu ki nhip bus APB1

    I2C1->CR1     |= (1U<<0); // bat I2C
}

void I2C1_Read_Byte(uint8_t saddr, uint8_t maddr, uint8_t* data)
{
    uint32_t to = I2C_TIMEOUT;
    while ((I2C1->SR2 & (SR2_BUSY)) && --to);
    
    // 1. Start
    I2C1->CR1 |= (CR1_START);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (SR1_START)) && --to);

    // 2. Gui dia chi de ghi thanh ghi
    I2C1->DR = (saddr<<1);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_ADDR) && --to);
    (void)I2C1->SR1; // Doc SR1 roi SR2 de xoa sach co ADDR
    (void)I2C1->SR2;

    // 3. Gui dia chi thanh ghi can doc
    I2C1->DR = maddr;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_TXE) && --to);
    
    // 4. Repeated Start de doc 1 byte
    I2C1->CR1 |= (CR1_START);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & (SR1_START)) && --to);

    I2C1->DR = (saddr<<1) | 1;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_ADDR) && --to);

    // Voi 1 byte duy nhat: Tat ACK va bat STOP TRUOC KHI xoa co ADDR
    I2C1->CR1 &= ~(CR1_ACK);
    (void)I2C1->SR1;
    (void)I2C1->SR2;
    I2C1->CR1 |= CR1_STOP;

    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_RXNE) && --to);
    *data = (uint8_t)I2C1->DR;
}

void I2C1_Burst_Read(uint8_t saddr, uint8_t maddr, int n, uint8_t* data)
{
    uint32_t to = I2C_TIMEOUT;
    while ((I2C1->SR2 & SR2_BUSY) && --to);

    I2C1->CR1 |= (CR1_START);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_START) && --to);

    I2C1->DR = (saddr << 1);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_ADDR) && --to);
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = maddr;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_TXE) && --to);

    I2C1->CR1 |= (CR1_START);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_START) && --to);

    I2C1->DR = (saddr << 1) | 1;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_ADDR) && --to);

    // Xu ly rieng truong hop n == 1 de khong bao gio bi treo phan cung
    if (n == 1)
    {
        I2C1->CR1 &= ~(CR1_ACK);
        (void)I2C1->SR1;
        (void)I2C1->SR2;
        I2C1->CR1 |= CR1_STOP;

        to = I2C_TIMEOUT;
        while (!(I2C1->SR1 & SR1_RXNE) && --to);
        *data = (uint8_t)I2C1->DR;
    }
    else
    {
        I2C1->CR1 |= (CR1_ACK);
        (void)I2C1->SR1;
        (void)I2C1->SR2;

        while (n > 0U)
        {
            if (n == 1U)
            {
                I2C1->CR1 &= ~(CR1_ACK);
                I2C1->CR1 |= CR1_STOP;
                to = I2C_TIMEOUT;
                while (!(I2C1->SR1 & SR1_RXNE) && --to);
                *data++ = (uint8_t)I2C1->DR;
                break;
            }
            else
            {
                to = I2C_TIMEOUT;
                while (!(I2C1->SR1 & SR1_RXNE) && --to);
                *data++ = (uint8_t)I2C1->DR;
                n--;
            }
        }
    }
}

void I2C1_Write(uint8_t saddr, uint8_t maddr, uint8_t data)
{
    uint32_t to = I2C_TIMEOUT;
    while ((I2C1->SR2 & SR2_BUSY) && --to);

    I2C1->CR1 |= (CR1_START);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_START) && --to);
    
    I2C1->DR = (saddr << 1);
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_ADDR) && --to);
    (void)I2C1->SR1;
    (void)I2C1->SR2;

    I2C1->DR = maddr;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_TXE) && --to);

    I2C1->DR = data;
    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_TXE) && --to);

    to = I2C_TIMEOUT;
    while (!(I2C1->SR1 & SR1_BTF) && --to);
    I2C1->CR1 |= CR1_STOP;
}

/**
 * @brief Quét kiểm tra toàn bộ bus I2C từ 0x01 đến 0x7F
 * @return Số lượng thiết bị tìm thấy
 */
uint8_t I2C1_Scan(void)
{
    uint8_t found = 0;
    char hex_str[10];
    char num_str[12];
    uint32_t to;

    Send_String("\r\n--- BAT DAU QUET BUS I2C ---\r\n");

    for (uint8_t addr = 1; addr < 128; addr++)
    {
        to = I2C_TIMEOUT;
        while ((I2C1->SR2 & SR2_BUSY) && --to);
        if (to == 0)
        {
            I2C1->CR1 |= (1U << 15);
            for (volatile int i = 0; i < 500; i++);
            I2C1->CR1 &= ~(1U << 15);
            I2C1->CR1 |= (1U << 0); // Bật lại PE
            continue;
        }

        I2C1->CR1 |= CR1_START;
        to = I2C_TIMEOUT;
        while (!(I2C1->SR1 & SR1_START) && --to);
        if (to == 0) continue;

        I2C1->DR = (uint8_t)(addr << 1);

        to = I2C_TIMEOUT;
        while (!(I2C1->SR1 & (SR1_ADDR | SR1_AF)) && --to);

        if (I2C1->SR1 & SR1_ADDR)
        {
            found++;

            (void)I2C1->SR1;
            (void)I2C1->SR2;

            I2C1->CR1 |= CR1_STOP;

            // In thông báo qua UART
            Send_String("-> Tim thay thiet bi tai dia chi 7-bit: ");
            Byte_To_Hex_String(addr, hex_str);
            Send_String(hex_str);
            Send_String(" (8-bit: ");
            Byte_To_Hex_String((uint8_t)(addr << 1), hex_str);
            Send_String(hex_str);
            Send_String(")\r\n");
        }
        else
        {
            I2C1->CR1 |= CR1_STOP;
            I2C1->SR1 &= ~SR1_AF;
        }

        // Delay một nhịp ngắn để chân SDA/SCL kịp hồi lên mức cao sau STOP
        for (volatile int i = 0; i < 2000; i++);
    }

    Send_String("Tong so thiet bi tim thay: ");
    Int_To_String(found, num_str);
    Send_String(num_str);
    Send_String("\r\n----------------------------\r\n");

    return found;
}

void Byte_To_Hex_String(uint8_t byte, char *str)
{
    const char hex_chars[] = "0123456789ABCDEF";
    str[0] = '0';
    str[1] = 'x';
    str[2] = hex_chars[(byte >> 4) & 0x0F];
    str[3] = hex_chars[byte & 0x0F];
    str[4] = '\0';
}
