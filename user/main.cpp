/*
file   : *.cpp
author : shentq
version: V1.0
date   : 2015/7/5

Copyright 2015 shentq. All Rights Reserved.
*/

//STM32 RUN IN eBox


/*
一个简单的命令帧接收示例
*/
#include "ebox.h"
 #include "ebox_analog.h"
#include "ddc.h"
#include "adjust.h"
#include "bsp.h"
#include "filters.h"

#include "mbuser.h"


static DataFloat_t adc_value;
static DataFloat_t adc_value1;
static DataFloat_t adc_value2;
static DataFloat_t adc_value3;


uint64_t last_update;
 
Timer timer2(TIM2);


void ddc_input()
{
    ddc_get_char(uart1.read());
   
}


double v0,v1,v2,v3,v4;
double hex0,hex1,hex2,hex3;
double dif12;
double res;
double get_rx();
float temperature;
double rx;

DataU32_t is_enter_adjust_flag;
void enter_adjust(uint8_t *ptr, uint16_t len)
{
    is_enter_adjust_flag.byte[0] = *ptr++;
    is_enter_adjust_flag.byte[1] = *ptr++;
    is_enter_adjust_flag.byte[2] = *ptr++;
    is_enter_adjust_flag.byte[3] = *ptr++;
}
void setup()
{
	ebox_init();
    
    uart1.begin(115200);
    uart1.attach(ddc_input,RxIrq);
    uart1.interrupt(RxIrq,ENABLE);
    
  
    ddc_init();
    ddc_attach_chx(20,enter_adjust);
    timer2.begin(10);
    timer2.attach(ddc_loop);
    timer2.interrupt(ENABLE);
    timer2.start();
    

    if(adjust_check() == true)
    {

        adjust_read(&pt100);
        uart1.printf("\r\noffset rx:%f\r\n",pt100.offsetRx.value);
        uart1.printf("ratio rx:%f\r\n",pt100.ratioRx.value);
        uart1.printf("cc rx:%f\r\n",pt100.ccRx.value);
        
        uart1.printf("offset pt:%f\r\n",pt100.offsetPt.value);
        uart1.printf("ratio pt:%f\r\n",pt100.ratioPt.value);
        uart1.printf("cc pt:%f\r\n",pt100.ccPt.value);

        uart1.printf("\r\n***********calibration paraments is exsit!****************\r\n");
    }
	adc.begin(1);


    
    PA5.mode(OUTPUT_PP);
    
    PB0.mode(OUTPUT_PP);
    PB1.mode(OUTPUT_PP);
    PB2.mode(OUTPUT_PP);
    
    //默认选择0通道
    PB0.reset();
    PB1.reset();
    PB2.reset();
    
    //继电器
    PB10.mode(OUTPUT_PP);
    PB10.reset();
    
    
    PA4.mode(OUTPUT_PP);
    PA4.set();
    
    modbus_init();
    

}

int main(void)
{
    uint8_t data[16];
	setup();



	while(1)
	{		 	
        if(is_enter_adjust_flag.value == 0x55aa)
        {
            is_enter_adjust_flag.value = 0;
            calibrate();
        }
        if(millis() - last_update > 1000)
        {
            adc_value.value = adc.read_average(ADC_AIN0);
            adc_value1.value = adc.read_average(ADC_AIN1);


            pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
            pt100.rt.value = adc_value.value * pt100.ratioPt.value + pt100.offsetPt.value - 2*pt100.rx.value;

            pt100.temp.value = RtoT(pt100.rt.value,1);


        }

        modbus_loop();
        
    }
}
