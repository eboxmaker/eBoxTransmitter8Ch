#include "ebox.h"
#include "bsp_ebox.h"
#include "ddc.h"


#include "adjust.h"
/**
	*	1	��������ʾ��UartStream����
	*	2	UartStream�Ǵ��ڼ�����stream���ࡣ֧����stream�����й���
	*		������ȡһ��String������һ���ؼ��֣��ؼ��ʵ�
	*   	����������ж��������ԵĶ�ȡ��
    *       �����ʼ��ȡ���ͻ�ȵ���ȡ�����󻹻���ʱ�趨�ĳ�ʱʱ�䡣�û���ע��ʹ��
	*/
	

/* ���������������̷������� */
#define EXAMPLE_NAME	"UartStream example"
#define EXAMPLE_DATE	"2018-08-13"



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
    
    //ADC��ʼ��
	adc.begin(1);


    //LED��ʼ��
    PA5.mode(OUTPUT_PP);

    //ͨ��ѡ���ʼ��
    PB0.mode(OUTPUT_PP);
    PB1.mode(OUTPUT_PP);
    PB2.mode(OUTPUT_PP);
    //Ĭ��ѡ��0ͨ��
    set_channel(0);

    //�̵���
    PB10.mode(OUTPUT_PP);
    PB10.reset();//Ĭ�����ã�ʹ��PT100
    PA7.mode(OUTPUT_PP);
    PA7.reset();//Ĭ�����ã�ʹ��PT100


    mb.config(&uart2, 9600,&PA4);
    mb.setSlaveId(1);  

    for(int i = 0; i < 80 ; i++)
    {
        mb.addHreg(i);
    }
    
        //����У׼����
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
        
        if( pt100.ptType > 2 || pt100.ptType < 0)//�Ƿ���������������ΪĬ��ֵ��������
        {
            pt100.ptType = DEFAULT_TYPE;
            set_ratio(DEFAULT_TYPE);
            adjust_save(&pt100);
        }
        
        mb.Hreg(MB_HOLD_OFFSET + 31,  pt100.ptType);//�����ȵ�������
    }
    else//����ʧ�������У׼����
    {
        pt100.ptType = DEFAULT_TYPE;
        set_channel(0);//Ĭ��ʹ��0ͨ��У׼����
        calibrate();
    }
    //���Ź���ʼ��
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
        
        //�Ƿ��յ�У׼����
        is_enter_adjust();
        change_ptx_mode();
        update_temp();
        
        
        mb.task();
        

        //ι��
        wdg.feed();

	}
}

void hold_update_ch(uint8_t ch,PtData_t *pt)
{
    mb.Hreg(ch+40,(word)(pt->temp.value*10));
    mb.Hreg(ch+40 + 20,(word)(pt->rx.value*100));

    if(pt->ptType == PT10)
        mb.Hreg(ch + 40 + 10,(word)(pt->rt.value*1000));
    else if(pt->ptType == PT100)
        mb.Hreg(ch + 40 + 10,(word)(pt->rt.value*100));
    else if(pt->ptType == PT1000)
        mb.Hreg(ch + 40 + 10,(word)(pt->rt.value*10));
}



static DataFloat_t adc_value;
static DataFloat_t adc_value1;
static DataFloat_t adc_value2;
static DataFloat_t adc_value3;


void update_temp()
{
   static int ch = 0;
    uint64_t rc_delay;

    
    //���߼�ѭ��
    //���ڶ�ͨ����ƣ��ܵ�����ģ��ͨ���������ƣ�
    //�����л�ͨ�������Ҫ�и���ʱ��ȷ��ģ������ȷ���л���ɡ�
    set_channel(ch);
    rc_delay = millis();
    PA5.set();
    while(millis() - rc_delay < 50)
    {
        mb.task();
    }
    PA5.reset();
    
    //��ȥADC
//    adc_value.value = adc.read_average(ADC_AIN0);
        mb.task();
    adc_value1.value = adc.read_average(ADC_AIN1);
        mb.task();
    adc_value2.value = adc.read_average(ADC_AIN2);
        mb.task();

    //�������
    pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
    if(adc_value2.value > 32760)
    {
        pt100.rt.value = -999.9;
        pt100.temp.value = -999.9;
    }
    else
    {
        pt100.rt.value = adc_value2.value * pt100.ratioPt.value + pt100.offsetPt.value - 2*pt100.rx.value;

        //�����¶�
        pt100.temp.value = RtoT(pt100.rt.value,pt100.ptType);
        pt100.temp.value = round(pt100.temp.value*10)/10.0;
    }
    
    //����
    hold_update_ch(ch,&pt100);
    
    //�л�ͨ��
    ch++;
    ch %=8;
    

}
void is_enter_adjust()
{
    if(mb.Hreg(MB_HOLD_OFFSET + 30) == 1)
    {
        mb.Hreg(MB_HOLD_OFFSET + 30,0);
        set_channel(0);//Ĭ��ʹ��0ͨ��У׼����
        calibrate();
    }
}
void change_ptx_mode()
{
    static ResType_t temp_type = PT100;

    if((ResType_t) mb.Hreg(MB_HOLD_OFFSET + 31) != temp_type)
    {
        if( mb.Hreg(MB_HOLD_OFFSET + 31) > 2 || mb.Hreg(MB_HOLD_OFFSET + 31) < 0)//�Ƿ���������������ΪĬ��ֵ��������
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
