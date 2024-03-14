#include "oled.h"
#include <cstdio>
#include <cstring>
#include <string.h>

static unsigned int HZ=0;
static const int OLED_ADDRESS = 0x78;      // 7 bit I2C address
//const int addr8bit = 0x48 << 1; // 8bit I2C address, 0x90

I2C oledi2c(PB_9 , PB_8 );
//oledi2c.read( addr8bit, cmd, 2);

//写命令
void Write_IIC_Command(uint8_t IIC_Command)
{
    char cmd[2];
    cmd[0] = 0x00;
    cmd[1] = IIC_Command;
	oledi2c.write(OLED_ADDRESS,cmd,2);//OLED地址
}

//写数据
void Write_IIC_Data(uint8_t IIC_Data)
{
    char cmd[2];
    cmd[0] = 0x40;
    cmd[1] = IIC_Data;
	oledi2c.write(OLED_ADDRESS,cmd,2);//OLED地址
}

//OLED全屏填充
void OLED_Fill(uint8_t fill_Data)
{
	int m,n;
	for(m=0;m<8;m++)
	{
		Write_IIC_Command(0xb0+m);		//page0-page1
		Write_IIC_Command(0x00);		//low column start address
		Write_IIC_Command(0x10);		//high column start address
		for(n=0;n<130;n++)
		{
			Write_IIC_Data(fill_Data);
		}
	}
}
//清屏
void OLED_Clear(void)
{
	OLED_Fill(0x00);
}
//设置起始点坐标
void OLED_Set_Pos(uint8_t x, uint8_t y) 
{ 
	Write_IIC_Command(0xb0+y);
	Write_IIC_Command((((x)&0xf0)>>4)|0x10);
	Write_IIC_Command(((x)&0x0f)|0x01);
}
//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr)
{
  uint8_t c = 0, i = 0;
  c = chr - ' '; //得到偏移后的值
  if(x > 130 - 1)
  {
    x = 0;
    y = y + 2;
  }
	OLED_Set_Pos(x, y);
	for(i = 0; i < 8; i++)
		Write_IIC_Data(zf[c * 16 + i]);
	OLED_Set_Pos(x, y + 1);
	for(i = 0; i < 8; i++)
		Write_IIC_Data(zf[c * 16 + i + 8]);

}
//m^n函数
uint32_t oled_pow(uint8_t m, uint8_t n)
{
  uint32_t result = 1;
  while(n--)result *= m;
  return result;
}
//显示中英文字符和数字0-9
//x:0~127, y:0~7
void OLED_ShowCH(uint8_t x, uint8_t y,char *chs)
{
  uint32_t i=0;
	uint32_t j;
	char* m;
	while (*chs != '\0')
	{
		if (*chs > 0xa0)				//汉字内码都是大于0xa0
		{
			for (i=0 ;i < HZ;i++)
			{	
				if(x>112)
				{
					x=0;
					y=y+2;
				}
				if ((*chs == hz_index[i*2]) && (*(chs+1) == hz_index[i*2+1]))
				{
					OLED_Set_Pos(x, y);
					for(j=0;j<16;j++)
						Write_IIC_Data(hz[i*32+j]);
					OLED_Set_Pos(x,y+1);
					for(j=0;j<16;j++)
						Write_IIC_Data(hz[i*32+j+16]);
					x +=16;
					break;
				}
			}
			chs+=2;
		}
		else
		{
			if(x>122)
			{
				x=0;
				y=y+2;
			}
			m=strchr((char*)zf_index,*chs);
			if (m!=NULL)
			{
				OLED_Set_Pos(x, y);
				for(j = 0; j < 8; j++)
					Write_IIC_Data(zf[((uint8_t)*m-' ') * 16 + j]);
				OLED_Set_Pos(x, y + 1);
				for(j = 0; j < 8; j++)
					Write_IIC_Data(zf[((uint8_t)*m-' ') * 16 + j + 8]);
				x += 8;
			}
			chs++;
		}
	}
}
//显示个数字
//x,y :起点坐标
//len :数字的位数
//num:数值(0~4294967295);
//mode:   为1:显示0   为0:显示空格
void OLED_ShowNum(uint8_t x, uint8_t y, uint32_t num, uint8_t len,uint8_t mode)
{
  uint8_t t, temp;
  for(t = 0; t < len; t++)
  {
    temp = (num / oled_pow(10, len - t - 1)) % 10;
		if(temp == 0)
		{
			if(mode)
				OLED_ShowChar(x + 8*t, y, '0');
			else
				OLED_ShowChar(x + 8*t, y, ' ');
			continue;
		}
		else
			OLED_ShowChar(x + 8*t, y, temp + '0');
  }
}
// x0,y0 -- 起始点坐标(x0:0~127, y0:0~7); x1,y1 -- 起点对角线(结束点)的坐标(x1:1~128,y1:1~8)
void OLED_DrawBMP(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[])
{
	uint16_t j=0;
	uint8_t x,y;

  if(y1%8==0)
		y = y1/8;
  else
		y = y1/8 + 1;
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
    for(x=x0;x<x1;x++)
		{
			Write_IIC_Data(BMP[j++]);
		}
	}
}
//初始化SSD1306
void OLED_Init(void)
{
  oledi2c.frequency(400000);
  Write_IIC_Command(0xAE); //--display off
  Write_IIC_Command(0x00); //---set low column address
  Write_IIC_Command(0x10); //---set high column address
  Write_IIC_Command(0x40); //--set start line address
  Write_IIC_Command(0xB0); //--set page address
  Write_IIC_Command(0x81); // contract control
  Write_IIC_Command(0xFF); //--128
  Write_IIC_Command(0xA1); //set segment remap
  Write_IIC_Command(0xA6); //--normal / reverse
  Write_IIC_Command(0xA8); //--set multiplex ratio(1 to 64)
  Write_IIC_Command(0x3F); //--1/32 duty
  Write_IIC_Command(0xC8); //Com scan direction
  Write_IIC_Command(0xD3); //-set display offset
  Write_IIC_Command(0x00); //
  Write_IIC_Command(0xD5); //set osc division
  Write_IIC_Command(0x80); //
  Write_IIC_Command(0xD8); //set area color mode off
  Write_IIC_Command(0x05); //
  Write_IIC_Command(0xD9); //Set Pre-Charge Period
  Write_IIC_Command(0xF1); //
  Write_IIC_Command(0xDA); //set com pin configuartion
  Write_IIC_Command(0x12); //
  Write_IIC_Command(0xDB); //set Vcomh
  Write_IIC_Command(0x30); //
  Write_IIC_Command(0x8D); //set charge pump enable
  Write_IIC_Command(0x14); //
  Write_IIC_Command(0xAF); //--turn on oled panel
  OLED_Clear();
}
//显示温度
void dispTemp(int temp)
{
    char t[20];
    snprintf(t, 16, "Temp:% 4d C     ",temp);
    OLED_ShowCH(0,2,t);
}
//显示加速度
void dispAcc(int acc)
{
    char t[20];
    snprintf(t, 16, " Acc:%4d ",acc);
    OLED_ShowCH(0,4,t);
}
//显示心率
void dispHR(int hr)
{
    char t[20];
    snprintf(t, 16, "  HR:% 4d BMP   ",hr);  
    OLED_ShowCH(0,6,t);
}
