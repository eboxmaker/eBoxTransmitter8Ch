#include "ebox.h"
#include "bsp_ebox.h"
#include "ddc.h"
#include <Modbus.h>
#include <ModbusSerial.h>

#include "adjust.h"
/**
	*	1	此例程演示了UartStream操作
	*	2	UartStream是串口集成了stream的类。支持了stream的所有功能
	*		包括读取一个String，查找一个关键字，关键词等
	*   	但是这个类中都是阻塞性的读取，
    *       如果开始读取，就会等到读取结束后还会延时设定的超时时间。用户需注意使用
	*/
	

/* 定义例程名和例程发布日期 */
#define EXAMPLE_NAME	"UartStream example"
#define EXAMPLE_DATE	"2018-08-13"


ModbusSerial mb;

#define MB_HOLD_OFFSET 40

void update_temp();
void hold_update_ch(uint8_t ch,PtData_t *pt);
void is_enter_adjust();
void change_ptx_mode();


void setup()
{
	ebox_init();
    uart1.begin(115200);
    print_log(EXAMPLE_NAME,EXAMPLE_DATE);
    
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
    PA7.mode(OUTPUT_PP);
    PA7.reset();//默认设置，使用PT100


    mb.config(&uart2, 9600,&PA4);
    mb.setSlaveId(1);  

    for(int i = 0; i < 80 ; i++)
    {
        mb.addHreg(i);
    }
    
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
        
        mb.Hreg(MB_HOLD_OFFSET + 31,  pt100.ptType);//更新热电阻类型
    }
    else//加载失败则进入校准程序
    {
        pt100.ptType = DEFAULT_TYPE;
        set_channel(0);//默认使用0通道校准数据
        calibrate();
    }
    //看门狗初始化
    wdg.begin(4000);
}
int main(void)
{
    uint64_t last_update;

    int read_over_flag = 0;
    int ch_changed_flag = 0;

	setup();
	while (1)
	{
        
        //是否收到校准命令
        is_enter_adjust();
        change_ptx_mode();
        update_temp();
        
        
        mb.task();
        

        //喂狗
        wdg.feed();

	}
}

void hold_update_ch(uint8_t ch,PtData_t *pt)
{
    mb.Hreg(ch+40,(word)pt->temp.value*10);
    mb.Hreg(ch+40 + 20,(word)pt->rx.value*100);

    if(pt->ptType == PT10)
        mb.Hreg(ch + 40 + 10,(word)pt->rt.value*1000);
    else if(pt->ptType == PT100)
        mb.Hreg(ch + 40 + 10,(word)pt->rt.value*100);
    else if(pt->ptType == PT1000)
        mb.Hreg(ch + 40 + 10,(word)pt->rt.value*10);
}



static DataFloat_t adc_value;
static DataFloat_t adc_value1;
static DataFloat_t adc_value2;
static DataFloat_t adc_value3;


void update_temp()
{
   static int ch = 0;
    uint64_t rc_delay;

    
    //主逻辑循环
    //由于多通道设计，受到最终模拟通道带宽限制，
    //所以切换通道后必须要有个延时来确保模拟量正确的切换完成。
    set_channel(ch);
    rc_delay = millis();
    PA5.set();
    while(millis() - rc_delay < 50)
    {
        mb.task();
    }
    PA5.reset();
    
    //得去ADC
    adc_value.value = adc.read_average(ADC_AIN0);
    adc_value1.value = adc.read_average(ADC_AIN1);
    adc_value2.value = adc.read_average(ADC_AIN2);

    //计算电阻
    pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
    pt100.rt.value = adc_value2.value * pt100.ratioPt.value + pt100.offsetPt.value - 2*pt100.rx.value;

    //计算温度
    pt100.temp.value = RtoT(pt100.rt.value,pt100.ptType);
    
    //更新
    hold_update_ch(ch,&pt100);
    
    //切换通道
    ch++;
    ch %=8;
    

}
void is_enter_adjust()
{
    if(mb.Hreg(MB_HOLD_OFFSET + 30) == 1)
    {
        mb.Hreg(MB_HOLD_OFFSET + 30,0);
        set_channel(0);//默认使用0通道校准数据
        calibrate();
    }
}
void change_ptx_mode()
{
    static ResType_t temp_type = PT100;

    if((ResType_t) mb.Hreg(MB_HOLD_OFFSET + 31) != temp_type)
    {
        if( mb.Hreg(MB_HOLD_OFFSET + 31) > 2 || mb.Hreg(MB_HOLD_OFFSET + 31) < 0)//非法参数。重新设置为默认值，并保存
            mb.Hreg(MB_HOLD_OFFSET + 31,1);
        
        pt100.ptType = (ResType_t) mb.Hreg(MB_HOLD_OFFSET + 31);
        temp_type = (ResType_t) mb.Hreg(MB_HOLD_OFFSET + 31);
        set_ratio(pt100.ptType);
        adjust_save(&pt100);
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
        case (uint8_t)PT10: {PB10.write(0);PA7.write(0);}break;
        case (uint8_t)PT100: {PB10.write(0);PA7.write(0);}break;
        case (uint8_t)PT1000: {PB10.write(1);delay_ms(1000);PA7.write(1);}break;
    }   
}
