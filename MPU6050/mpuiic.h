#ifndef __MPUIIC_H
#define __MPUIIC_H
#include "mbed.h"

// IIC所有操作函数
void mpu6050i2cinit(void);
uint8_t MPU_Read_Byte(uint8_t reg);
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data);
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf);
#endif
