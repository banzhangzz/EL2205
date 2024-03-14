#include "mpu6050.h"
using namespace std::chrono;

Timer mputmr;

//校准值
static signed short ax_cl = 0;
static signed short ay_cl = 0;
static signed short az_cl = 0;
static signed short gx_cl = 0;
static signed short gy_cl = 0;
static signed short gz_cl = 0;

/**
 * @brief 初始化MPU6050
 * @param 无
 * @return 状态 0成功 其他失败
 */
uint8_t MPU_Init(void)
{
    mputmr.start();
    mpu6050i2cinit();
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X80); //复位MPU6050
	HAL_Delay(100);//delay 100ms
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X00); //唤醒MPU6050
	MPU_Set_Gyro_Fsr(3);					 //陀螺仪传感器,±2000dps
	MPU_Set_Accel_Fsr(0);					 //加速度传感器,±2g
	MPU_Set_Rate(50);						 //设置采样率50Hz
	MPU_Write_Byte(MPU_INT_EN_REG, 0X00);	 //关闭所有中断
	MPU_Write_Byte(MPU_USER_CTRL_REG, 0X00); // I2C主模式关闭
	MPU_Write_Byte(MPU_FIFO_EN_REG, 0X00);	 //关闭FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG, 0X80); // INT引脚低电平有效
	MPU_Read_Byte(MPU_DEVICE_ID_REG);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG, 0X01); //设置CLKSEL,PLL X轴为参考
	MPU_Write_Byte(MPU_PWR_MGMT2_REG, 0X00); //加速度与陀螺仪都工作
	MPU_Set_Rate(200);						 //设置采样率为50Hz
	return 0;
}
//设置MPU6050陀螺仪传感器满量程范围
// fsr:0,±250dps;1,±500dps;2,±1000dps;3,±2000dps
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Gyro_Fsr(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG, fsr << 3); //设置陀螺仪满量程范围
}
//设置MPU6050加速度传感器满量程范围
// fsr:0,±2g;1,±4g;2,±8g;3,±16g
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Accel_Fsr(uint8_t fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG, fsr << 3); //设置加速度传感器满量程范围
}
//设置MPU6050的数字低通滤波器
// lpf:数字低通滤波频率(Hz)
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_LPF(uint16_t lpf)
{
	uint8_t data = 0;
	if (lpf >= 188)
		data = 1;
	else if (lpf >= 98)
		data = 2;
	else if (lpf >= 42)
		data = 3;
	else if (lpf >= 20)
		data = 4;
	else if (lpf >= 10)
		data = 5;
	else
		data = 6;
	return MPU_Write_Byte(MPU_CFG_REG, data); //设置数字低通滤波器
}
//设置MPU6050的采样率(假定Fs=1KHz)
// rate:4~1000(Hz)
//返回值:0,设置成功
//    其他,设置失败
uint8_t MPU_Set_Rate(uint16_t rate)
{
	uint8_t data;
	if (rate > 1000)
		rate = 1000;
	if (rate < 4)
		rate = 4;
	data = 1000 / rate - 1;
	data = MPU_Write_Byte(MPU_SAMPLE_RATE_REG, data); //设置数字低通滤波器
	return MPU_Set_LPF(rate / 2);					  //自动设置LPF为采样率的一半
}

//得到温度值
//返回值:温度值(扩大了100倍)
short MPU_Get_Temperature(void)
{
	uint8_t buf[2];
	short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR, MPU_TEMP_OUTH_REG, 2, buf);
	raw = ((uint16_t)buf[0] << 8) | buf[1];
	temp = 36.53 + ((double)raw) / 340;
	return temp * 100;
	;
}
//得到陀螺仪值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Gyroscope(short *gx, short *gy, short *gz)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_ADDR, MPU_GYRO_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*gx = ((uint16_t)buf[0] << 8) | buf[1];
		*gy = ((uint16_t)buf[2] << 8) | buf[3];
		*gz = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
}
//得到加速度值(原始值)
// gx,gy,gz:陀螺仪x,y,z轴的原始读数(带符号)
//返回值:0,成功
//    其他,错误代码
uint8_t MPU_Get_Accelerometer(short *ax, short *ay, short *az)
{
	uint8_t buf[6], res;
	res = MPU_Read_Len(MPU_ADDR, MPU_ACCEL_XOUTH_REG, 6, buf);
	if (res == 0)
	{
		*ax = ((uint16_t)buf[0] << 8) | buf[1];
		*ay = ((uint16_t)buf[2] << 8) | buf[3];
		*az = ((uint16_t)buf[4] << 8) | buf[5];
	}
	return res;
	;
}
/*
 *获取加速度值 (g)
 */
uint8_t MPU_Get_Acceleration(float *ax, float *ay, float *az)
{
    short ax1,ay1,az1;
    if(MPU_Get_Accelerometer(&ax1, &ay1, &az1)==0){
       ax1 -= ax_cl;
       ay1 -= ay_cl;
       az1 -= az_cl;              
    }
    *ax=(float)ax1/(0xffff/4);
    *ay=(float)ay1/(0xffff/4);
    *az=(float)az1/(0xffff/4); 
    return 0;   
}
/*
 *获取角速度值 (deg/s)
 */
uint8_t MPU_Get_AngularVelocity (float *gx, float *gy, float *gz)
{
    short gx1,gy1,gz1;
    if(MPU_Get_Gyroscope(&gx1, &gy1, &gz1)==0){
       gx1 -= gx_cl;
       gy1 -= gy_cl;
       gz1 -= gz_cl;              
    }
    *gx=(float)gx1/(0xffff/500);
    *gy=(float)gy1/(0xffff/500);
    *gz=(float)gz1/(0xffff/500); 
    return 0;   
}
/*
MPU6050校准函数
将IMU水平放置，z轴向上时，启动校准
计算N个周期的平均值，得到校准参数
*/
#define	CL_cnt	128
void MPU6050_calibrate()
{
	unsigned short i;
    short ax,ay,az;
    short gx,gy,gz;	
	signed int temp[6] = {0};
	for(i=0; i<CL_cnt; i++)
	{
		HAL_Delay(20);
		MPU_Get_Accelerometer(&ax,&ay,&az);
        MPU_Get_Gyroscope(&gx, &gy, &gz);
		temp[0] += ax;
		temp[1] += ay;
		temp[2] += az;
		temp[3] += gx;
		temp[4] += gy;
		temp[5] += gz;
	}	
	ax_cl = temp[0]/CL_cnt;
	ay_cl = temp[1]/CL_cnt;
	az_cl = temp[2]/CL_cnt - (0xffff>>2); //平放时z轴有重力加速度g，减去g值
	gx_cl = temp[3]/CL_cnt;
	gy_cl = temp[4]/CL_cnt;
	gz_cl = temp[5]/CL_cnt;
}
//
uint32_t GetMpumillis(void)
{
    return duration_cast<milliseconds>(mputmr.elapsed_time()).count();    
}

