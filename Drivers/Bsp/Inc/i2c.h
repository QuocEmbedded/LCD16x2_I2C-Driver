#ifndef I2C_H
#define I2C_H
#include "stm32f407xx.h"

#define CR1_START (1U<<8)
#define CR1_STOP  (1U<<9)
#define CR1_ACK   (1U<<10)

#define SR1_START (1U<<0)
#define SR1_ADDR  (1U<<1)
#define SR1_BTF   (1U<<2)
#define SR1_RXNE  (1U<<6)
#define SR1_TXE   (1U<<7)
#define SR1_AF    (1U<<10)

#define SR2_BUSY  (1U<<1)

#define I2C_TIMEOUT 50000U


void I2C1_Init();
void I2C1_Read_Byte(uint8_t saddr, uint8_t maddr, uint8_t* data);
void I2C1_Burst_Read(uint8_t saddr, uint8_t maddr, int n, uint8_t* data);
void I2C1_Write(uint8_t saddr, uint8_t maddr, uint8_t data);
uint8_t I2C1_Scan(void);
void Byte_To_Hex_String(uint8_t byte, char *str);
//void I2C1_Burst_Write(uint8_t saddr, uint8_t reg_start, const uint8_t *data, uint16_t length);
#endif