/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include <cstdint>
#include <cstdio>
#include <cstring>
#include "BufferedSerial.h"
#include "SerialBase.h"
#include "oled.h"
#include "mpu6050.h"
#include "MAX30102.h"

using namespace std;

// Blinking rate in milliseconds
#define BLINKING_RATE     500ms

typedef struct {
    int       HeartRt;
    uint32_t  steps;  
    float     Temper;   
    float     AccelX;  
    float     AccelY; 
    float     AccelZ;         
} message_t;
MemoryPool<message_t, 128> mpool;
Queue<message_t, 128> queue;
Thread Thread_led;
Thread Thread_oled;
DigitalIn button(BUTTON1);
// Initialise the digital pin
BufferedSerial BTserial(PA_0, PA_1);

uint8_t BTRdBuff[128],BTRdLength;
uint32_t getStep(float ax);

//thread led
void led_thread() {
    DigitalOut led(LED1);
    while (true) {
        led = !led;
        ThisThread::sleep_for(BLINKING_RATE);
    }
}
//thread oled
void oled_thread() {

    float ax=0,ay=0,az=0;
    char temp[100];
    float htratePm=0;
    float tempe=0;
    int page=1;
    uint32_t step=0;

    while(true){
        osEvent evt = queue.get();
        if(evt.status == osEventMessage){
            message_t *message = (message_t *)evt.value.p;
            step=message->steps;
            tempe= message->Temper;
            htratePm= message->HeartRt;
            ax= message->AccelX;
            ay= message->AccelY;
            az= message->AccelZ;
            mpool.free(message);
        } 
        //detect key
        if(button.read()==0){
            ThisThread::sleep_for(10ms);
             if(button.read()==0){
                  OLED_Clear();
                  page=page?0:1;
             }
        }
        switch(page){
            //max30102
            case 1:
                //Temperature 
                snprintf(temp,100,"  Temp:% 4.1f C ", tempe);
                OLED_ShowCH(0,2,temp); 
                //heartrate
                snprintf(temp,100,"HeartR:% 4.0f bmp ", htratePm);
                OLED_ShowCH(0,0,temp);
                break;
            //mpu6050
            case 0:                              
                snprintf(temp,100,"Acce x:% 6.3fg  ", ax);
                OLED_ShowCH(0,0,temp); 
                snprintf(temp,100,"Acce y:% 6.3fg  ", ay);
                OLED_ShowCH(0,2,temp); 
                snprintf(temp,100,"Acce z:% 6.3fg  ", az);
                OLED_ShowCH(0,4,temp);
                snprintf(temp,100," Steps:% 6d     ", step);
                OLED_ShowCH(0,6,temp);                                         
                break;
            default:
                break;
        }
    }  
}

int main()
{
    char temp1[100];
    float ax1,ay1,az1;
    float htratePm1;
    int  htrateAvg;
    float tempe;

    OLED_Init();
    OLED_ShowCH(0,0,(char*)"Calibrating");
    OLED_ShowCH(0,2,(char*)"Please Wait...");
    max30102_init();
    MPU_Init();
    mpu_dmp_init();
    MPU6050_calibrate();
    OLED_Clear();
  
    //init
    BTserial.set_blocking(true);
    BTserial.set_baud(115200);   
    HAL_Delay(10);

    //start thread
    Thread_led.start(callback(led_thread));
    Thread_oled.start(callback(oled_thread));

    while (true) {
        //read value
        tempe =getTemperature();
        if(getHeartrate(&htrateAvg,&htratePm1) !=0)  htrateAvg=0;        
        MPU_Get_Acceleration(&ax1, &ay1, &az1);

        message_t *message = mpool.alloc();
        message->steps=getStep(ax1);
        message->AccelX = ax1;
        message->AccelY = ay1;
        message->AccelZ = az1;
        message->HeartRt = htrateAvg;
        message->Temper = tempe;
        queue.put(message);        

        memset(temp1,0,sizeof(temp1));
        snprintf(temp1,100,"Temperature=%4.1f,Heartrate=%4d,", tempe,htrateAvg);
        BTserial.write(temp1,strlen(temp1));
        memset(temp1,0,sizeof(temp1));
        snprintf(temp1,100,"AccelerationX=%6.3f,AccelerationY=%6.3f,AccelerationZ=%6.3f", 
                            ax1,ay1,az1);
        BTserial.write(temp1,strlen(temp1)); 
        mpool.free(message);
        ThisThread::sleep_for(200ms);
    }
}

uint32_t getStep(float ax)
{
    static uint32_t flag=0,steps=0,times;

    if(ax > 0.5 || ax < -0.5){
        if(flag == 0){
            flag=1;
            times= GetMpumillis();
            ++steps;
        }
        else{
            if((GetMpumillis()-times) > 1000 ){
                times= GetMpumillis();
                steps+=2;
            }
        }
    }
    else {
       flag=0; 
    }
    return steps;
}

