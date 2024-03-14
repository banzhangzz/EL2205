#include "mpuiic.h"
#include "mpu6050.h"
#include <cstring>

I2C mpu6050i2c(PC_1 , PC_0 ); //mpu6050
DigitalIn mpuINT(PB_0);

void mpu6050i2cinit(void)
{
    mpu6050i2c.frequency(400000);
}
/**
 * @brief MUP6050 I2C连续写
 * @param addr:器件地址8bit
 * @param reg:寄存器地址
 * @param len:长度
 * @param buf:缓冲区地址
 * @return 状态 0成功 其他失败
 */
uint8_t MPU_Write_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    char cmd[100];
    cmd[0] = reg;
    memcpy(&cmd[1],buf,len);
	return mpu6050i2c.write(addr<<1,cmd,len+1);
}
/**
 * @brief MUP6050 I2C连续读
 * @param addr:器件地址8bit
 * @param reg:寄存器地址
 * @param len:长度
 * @param buf:缓冲区地址
 * @return 状态 0成功 其他失败
 */
uint8_t MPU_Read_Len(uint8_t addr, uint8_t reg, uint8_t len, uint8_t *buf)
{
    char cmd[10];
    cmd[0] = reg;
    mpu6050i2c.write(addr<<1,cmd,1);
//	HAL_Delay(1);
    return mpu6050i2c.read(addr<<1,(char*)buf,len);
}
/**
 * @brief MUP6050 I2C写一个字节
 * @param reg:寄存器地址
 * @param data:数据
 * @return 状态 0成功 其他失败
 */
uint8_t MPU_Write_Byte(uint8_t reg, uint8_t data)
{
    char cmd[10];
    cmd[0] = reg;
	return mpu6050i2c.write(MPU_ADDR<<1,cmd,1);
}
/**
 * @brief MUP6050 I2C写一个字节
 * @param reg:寄存器地址
 * @return 读取到的数据
 */
uint8_t MPU_Read_Byte(uint8_t reg)
{
    char cmd[10];
    cmd[0] = reg;
    mpu6050i2c.write(MPU_ADDR<<1,cmd,1);
    mpu6050i2c.read(MPU_ADDR<<1,cmd,1);
	return cmd[0];
}
