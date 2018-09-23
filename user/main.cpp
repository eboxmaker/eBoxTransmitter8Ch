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


 



void setup()
{
	ebox_init();
    
    uart1.begin(115200);
//    uart2.begin(115200);
//    uart2.attach(ddc_input,RxIrq);
//    uart2.interrupt(RxIrq,ENABLE);
    
  

    

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
    select_channel(0);
    
    //继电器
    PB10.mode(OUTPUT_PP);
    PB10.reset();//默认设置，使用PT100
    
    
    modbus_init();
    
    wdg.begin(2000);

}


int main(void)
{
	setup();

    uint64_t last_update;
    uint64_t rc_delay;

    int read_over_flag = 0;
    int ch_changed_flag = 0;
    int ch = 0;
	while(1)
	{		 	
        if(is_enter_adjust_flag.value == 0x55aa)
        {
            is_enter_adjust_flag.value = 0;
            select_channel(0);//默认使用0通道校准数据
            calibrate();
        }
        select_channel(ch);
        rc_delay = millis();
        PA5.set();
        while(millis() - rc_delay < 100)
        {
            modbus_loop();
        }
        PA5.reset();
        adc_value.value = adc.read_average(ADC_AIN0);
        adc_value1.value = adc.read_average(ADC_AIN1);


        pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
        pt100.rt.value = adc_value.value * pt100.ratioPt.value + pt100.offsetPt.value - 2*pt100.rx.value;

        pt100.temp.value = RtoT(pt100.rt.value,1);
        
        hold_update_ch(ch);
        ch++;
        ch %=8;

        wdg.feed();
        
    }
}

void select_channel(uint8_t ch)
{  
    PB0.write(ch&1);
    PB1.write(ch&2);
    PB2.write(ch&4);
}
void ddc_input()
{
    ddc_get_char(uart2.read());
   
}

void enter_adjust(uint8_t *ptr, uint16_t len)
{
    is_enter_adjust_flag.byte[0] = *ptr++;
    is_enter_adjust_flag.byte[1] = *ptr++;
    is_enter_adjust_flag.byte[2] = *ptr++;
    is_enter_adjust_flag.byte[3] = *ptr++;
}

