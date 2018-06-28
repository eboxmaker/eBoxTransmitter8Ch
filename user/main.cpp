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
#include "freemodbus.h"
 #include "ebox_analog.h"
#include "ddc.h"
#include "adjust.h"
#include "bsp.h"
#include "filters.h"

#define LED1_ON   PB8.set()
#define LED1_OFF  PB8.reset()

#define LED2_ON   PB9.set()
#define LED2_OFF  PB9.reset()

#define  BUTTON1_READ()  PA8.read()
 
u8 count;
Timer timer4(TIM4);
Timer timer2(TIM2);
FilterAverage fmw0(50);

void FreeModbusIoConfig(void)
{ 
    modbus.uart=&uart1;
	modbus.timer=&timer4;
	modbus.Mode4851=&PB0;
	modbus.Mode4852=&PB1;
	
	modbus.Mode=MB_RTU;
	modbus.SlaveAdress=0x01;
	modbus.PortId=0x01;
	modbus.BaudRate=115200;
	modbus.Pariby=MB_PAR_NONE;	
  }


void LED_Poll(void)
{
  uint8_t LED_Status;
  LED_Status = ucRegCoilsBuf[0];
  if(LED_Status & 0x01) {LED1_ON;} else {LED1_OFF;}
  if(LED_Status & 0x02) {LED2_ON;} else {LED2_OFF;}
}

void Button_Poll(void)
{
  
  uint8_t Button_Status = 0x00;  
  BUTTON1_READ()?(Button_Status &=~ 0x01):(Button_Status |= 0x01);
 
  ucRegDiscreteBuf[0] = Button_Status;
} 

void Adc_Poll(void)
{ 
    usRegInputBuf[0]=analog_read(&PA0);
	usRegInputBuf[1]=analog_read(&PA1);

	usRegInputBuf[4]=analog_read(&PA4);
	usRegInputBuf[5]=analog_read(&PA5);
	
}
void ddc_input()
{
    ddc_get_char(uart1.read());
    //uart1.write(uart1.read());
}

double v0,v1,v2,v3,v4;
double hex0,hex1,hex2,hex3;
double dif12;
double res;
double RtoT(double R, uint8_t type);
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
   // uart2.begin(115200);
    uart3.begin(115200);
    uart1.attach(ddc_input,RxIrq);
    uart1.interrupt(RxIrq,ENABLE);
    
    ddc_init();
    ddc_attach_chx(20,enter_adjust);
     timer2.begin(10);
    //timer2.attach(t2it);
    timer2.attach(ddc_loop);
    timer2.interrupt(ENABLE);
    timer2.start();
    

//	FreeModbusIoConfig();
//	FreemodbusConfig();
//	
//    PB8.mode(OUTPUT_PP);
//    PB9.mode(OUTPUT_PP); 
//    
//    PA8.mode(INPUT);                                                                       
//		
//	PA0.mode(AIN);
//	PA1.mode(AIN);

//	PA4.mode(AIN);
//	PA5.mode(AIN);
    PA5.mode(OUTPUT_PP);
	adc.begin(1);
    uart1.printf("test:%d\r\n",adc.self_test());

    //DataU32_t flag;

//    xx.ccPt.value = 12345;
//    uart1.printf("ccpt:%f\r\n",xx.ccPt.value);
//    uart1.printf("flag:%d\r\n",adjust_check());
//    adjust_save(&xx);
//    uart1.printf("flag:%d\r\n",adjust_check());
//    xx.ccPt.value = 0;
//    uart1.printf("ccpt:%f\r\n",xx.ccPt.value);
//    adjust_read(&xx);
//    uart1.printf("ccpt:%f\r\n",xx.ccPt.value);
//    while(1);
    
    PA5.mode(OUTPUT_PP);
    
    PB0.mode(OUTPUT_PP);
    PB1.mode(OUTPUT_PP);
    PB2.mode(OUTPUT_PP);
    
    //默认选择0通道
    PB0.reset();
    PB1.reset();
    PB2.reset();

}
static DataFloat_t adc_value;
static DataFloat_t adc_value1;
static DataFloat_t adc_value2;
static DataFloat_t adc_value3;
static DataFloat_t adc_voltage;
static DataFloat_t adc_voltage1;
static DataFloat_t adc_voltage2;
static DataFloat_t adc_voltage3;

uint64_t last_update;
int main(void)
{
    uint8_t data[16];
	setup();



	while(1)
	{		 	
        if(is_enter_adjust_flag.value == 0x55aa)
        {
            calibrate();
            is_enter_adjust_flag.value = 0;
        }
            adc_value.value = adc.read_average(ADC_AIN0);
            adc_voltage.value = adc.adc_to_voltage(adc_value.value);
            adc_value1.value = adc.read_average(ADC_AIN1);
            adc_voltage1.value = adc.adc_to_voltage(adc_value1.value);
            adc_value2.value = adc.read_average(ADC_AIN2);
            adc_voltage2.value = adc.adc_to_voltage(adc_value2.value);

            pt100.rxOrigin.value = (adc_voltage1.value - adc_voltage2.value )/101;
            if(pt100.rxMode == 0)
                pt100.rx.value = (adc_value1.value -  adc_value2.value - pt100.offsetRx.value)/538.667;
            else
                pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;

            
            if(pt100.ptMode == 0)
                pt100.rt.value = (adc_value.value  - pt100.offsetPt.value)/85.333;
            else
                pt100.rt.value = adc_value.value * pt100.ratioPt.value + pt100.offsetPt.value - pt100.rx.value;


            if(millis() - last_update > 1000)
            {
                PA5.toggle();
                last_update = millis();
                data[0] = pt100.rt.byte[0];
                data[1] = pt100.rt.byte[1];
                data[2] = pt100.rt.byte[2];
                data[3] = pt100.rt.byte[3];
                data[4] = pt100.rt.byte[0];
                data[5] = pt100.rt.byte[1];
                data[6] = pt100.rt.byte[2];
                data[7] = pt100.rt.byte[3];
                ddc_nonblocking(data,8,DDC_NoAck,6);  
                data[0] = pt100.rxOrigin.byte[0];
                data[1] = pt100.rxOrigin.byte[1];
                data[2] = pt100.rxOrigin.byte[2];
                data[3] = pt100.rxOrigin.byte[3];
                data[4] = pt100.rx.byte[0];
                data[5] = pt100.rx.byte[1];
                data[6] = pt100.rx.byte[2];
                data[7] = pt100.rx.byte[3];
                ddc_nonblocking(data,8,DDC_NoAck,5);

            }
//        FreemodbusPoll();
//        LED_Poll();
//        Button_Poll();
//        Adc_Poll();
//        PA5.toggle();
        
    }
}

double get_rx()
{
    double rx = 0;
    double vdif = 0;
    double temphex1 = 0,temphex2 = 0;
    double tempv1 = 0,tempv2 = 0;
    
   while(1)
   {
        uint16_t temp = adc.read(ADC_AIN1);
        if(fmw0.sample(temp) == true)
        {
            temphex1 = fmw0.out();
            tempv1 = adc.adc_to_voltage(hex1);
            break;
        }

   }
   while(1)
   {
        uint16_t temp = adc.read(ADC_AIN2);
        if(fmw0.sample(temp) == true)
        {
            temphex2 = fmw0.out();
            tempv2 = adc.adc_to_voltage(hex2);
            break;
        }
   }

    vdif = (tempv1 -  tempv2);
    
    return vdif;
}
double RtoT(double R, uint8_t type)
{
    uint8_t R0;// ???0????
    double T, tmp, min, mid, max;
    double x, y, z;
    double d = 3.9083/1e3, b = -5.775/1e7, c = -4.183/1e12;
    
    if(type <= 1)
    {
        if(type == 0)
        {
            R0 = 10; min = 1.852; mid = 10; max = 39.049;// Pt10
        }else
        {
            R0 = 100; min = 18.52; mid = 100; max = 390.49; // Pt100
        }
        if(R>=mid && R<=max)
        {
            y = sqrt(d*d - 4*b*(1-R/R0));
            T = (y-d) / (2*b); // ????
        }else if(R<mid && R>=min)
        {
            x = 0; y = -100; z = -201;
            tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
            while(fabs(tmp-R)>=0.007)
            {
                if(R > tmp)
                {
                    x = x; z = y;
                    y = (x+y)/2;
                }
                else
                {
                    x = y; z = z;
                    y = (y+z)/2;
                }
                tmp = R0*(1 + d*y + b*y*y + c*(y-100)*y*y*y);
            }
            T = y; // ????
        }
    }
    else
    {
        if(type == 2)// Cu50
            R0 = 50;
        else // Cu100
            R0 = 100;
            
        x = 150; y = 50; z = -50;
        tmp = R0*(1 + 4.289/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
        while(fabs(tmp-R)>=0.007)
        {
            if(R > tmp)
            {
                x = x; z = y; 
                y = (x+y)/2;
            }
            else
            {
                x = y; z = z;
                y = (y+z)/2;
            }
            tmp = R0*(1 + 4.28899/1e3*y - 2.133/1e7*y*y + 1.233/1e9*y*y*y);
        }
        T = y;
    }
    
    return T;
}

