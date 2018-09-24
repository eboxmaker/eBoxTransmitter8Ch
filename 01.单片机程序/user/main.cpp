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
    uart2.begin(115200);

    //ADC初始化
	adc.begin(1);


    //LED初始化
    PA5.mode(OUTPUT_PP);
    
    //通道选择初始化
    PB0.mode(OUTPUT_PP);
    PB1.mode(OUTPUT_PP);
    PB2.mode(OUTPUT_PP);
    //默认选择0通道
    set_channel(0);
    
    //继电器
    PB10.mode(OUTPUT_PP);
    PB10.reset();//默认设置，使用PT100
    
    //modbus初始化
    modbus_init();
    
    
    //加载校准参数
    if(adjust_check() == true)
    {

        adjust_read(&pt100);
        uart1.printf("\r\n***********calibration paraments is exsit!****************\r\n");
        uart1.printf("\r\noffset rx:%f\r\n",pt100.offsetRx.value);
        uart1.printf("ratio rx:%f\r\n",pt100.ratioRx.value);
        uart1.printf("cc rx:%f\r\n",pt100.ccRx.value);
        
        uart1.printf("offset pt:%f\r\n",pt100.offsetPt.value);
        uart1.printf("ratio pt:%f\r\n",pt100.ratioPt.value);
        uart1.printf("cc pt:%f\r\n",pt100.ccPt.value);
        
        if( pt100.ptType > 2 || pt100.ptType < 0)//非法参数。重新设置为默认值，并保存
        {
            pt100.ptType = DEFAULT_TYPE;
            set_ratio(DEFAULT_TYPE);
            adjust_save(&pt100);
        }
        
        usRegHoldingBuf[31] = pt100.ptType;//更新热电阻类型
    }
    else//加载失败则进入校准程序
    {
        pt100.ptType = DEFAULT_TYPE;
        set_channel(0);//默认使用0通道校准数据
        calibrate();
    }
    //看门狗初始化
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
        //是否收到校准命令
        if(is_enter_adjust_flag.value == 0x55aa)
        {
            is_enter_adjust_flag.value = 0;
            set_channel(0);//默认使用0通道校准数据
            calibrate();
        }
        
        
        //主逻辑循环
        //由于多通道设计，受到最终模拟通道带宽限制，
        //所以切换通道后必须要有个延时来确保模拟量正确的切换完成。
        set_channel(ch);
        rc_delay = millis();
        PA5.set();
        while(millis() - rc_delay < 150)
        {
            modbus_loop();
        }
        PA5.reset();
        
        //得去ADC
        adc_value.value = adc.read_average(ADC_AIN0);
        adc_value1.value = adc.read_average(ADC_AIN1);

        //计算电阻
        pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
        pt100.rt.value = adc_value.value * pt100.ratioPt.value + pt100.offsetPt.value - 2*pt100.rx.value;

        //计算温度
        pt100.temp.value = RtoT(pt100.rt.value,pt100.ptType);
        
        //更新
        hold_update_ch(ch,&pt100);
        
        //切换通道
        ch++;
        ch %=8;

        //喂狗
        wdg.feed();

    }
}

void set_channel(uint8_t ch)
{  
    PB0.write(ch&1);
    PB1.write(ch&2);
    PB2.write(ch&4);
}
void set_ratio(ResType_t type)
{
    switch((uint8_t)type)
    {
        case (uint8_t)PT10: PB10.write(0);break;
        case (uint8_t)PT100: PB10.write(0);break;
        case (uint8_t)PT1000: PB10.write(1);break;
    }   
}

