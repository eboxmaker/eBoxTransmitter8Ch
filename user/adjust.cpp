
#include "adjust.h"
#include "ddc.h"
#include "bsp.h"

double RtoT(double R, uint8_t type);

PtData_t pt100;

uint8_t adjust_flag_rx,adjust_flag_pt;

static DataFloat_t adc_value;
static DataFloat_t adc_value1;
static DataFloat_t adc_value2;
static DataFloat_t adc_value3;
static DataFloat_t adc_voltage;
static DataFloat_t adc_voltage1;
static DataFloat_t adc_voltage2;
static DataFloat_t adc_voltage3;

double data1[12][2] = {
//    X      Y
//    {187.1, 25.4},
//    {179.5, 22.8},
//    {157.0, 20.6},
//    {197.0, 21.8},
//    {239.4, 32.4},
//    {217.8, 24.4},
//    {227.1, 29.3},
//    {233.4, 27.9},
//    {242.0, 27.8},
//    {251.9, 34.2},
//    {230.0, 29.2},
//    {271.8, 30.0}
};
double answer[2];
double SquarePoor[4];


void display(double *dat, double *Answer, double *SquarePoor, int rows, int cols)
{
    double v, *p;
    int i, j;
    uart1.printf("回归方程式:    Y = %.5lf", Answer[0]);
    for (i = 1; i < cols; i ++)
        uart1.printf(" + %.5lf*X%d\r\n", Answer[i], i);
    uart1.printf("回归显著性检验: \r\n");
    uart1.printf("回归平方和：%12.4lf  回归方差：%12.4lf \r\n", SquarePoor[0], SquarePoor[2]);
    uart1.printf("剩余平方和：%12.4lf  剩余方差：%12.4lf \r\n", SquarePoor[1], SquarePoor[3]);
    uart1.printf("离差平方和：%12.4lf  标准误差：%12.4lf \r\n", SquarePoor[0] + SquarePoor[1], sqrt(SquarePoor[3]));
    uart1.printf("F   检  验：%12.4lf  相关系数：%12.4lf \r\n", SquarePoor[2] /SquarePoor[3],
           sqrt(SquarePoor[0] / (SquarePoor[0] + SquarePoor[1])));
    uart1.printf("剩余分析: \r\n");
    uart1.printf("      观察值      估计值      剩余值    剩余平方 \r\n");
    for (i = 0, p = dat; i < rows; i ++, p ++)
    {
        v = Answer[0];
        for (j = 1; j < cols; j ++, p ++)
            v += *p * Answer[j];
        uart1.printf("%12.2lf%12.2lf%12.2lf%12.2lf \r\n", *p, v, *p - v, (*p - v) * (*p - v));
    }
}

void calibrate_para(uint8_t *ptr, uint16_t len )
{
    uint8_t *p;
    uint8_t data[16];
    DataDouble_t adc;
    DataDouble_t value;
    
    uint8_t rows = len/16;

    if(rows == 1)
    {
        ebox_memcpy(adc.byte,ptr,8);
        ptr+=8;
        ebox_memcpy(value.byte,ptr,8);
        ptr+=8;
        if(value.value == 0)
        {
            pt100.offsetPt.value = 538.667;
            pt100.offsetPt.value = adc_value.value ;//差值
            pt100.ccPt.value = 100;
            ddc_nonblocking(pt100.ccPt.byte,8,DDC_NoAck,10);

            pt100.ptMode = 0;
        }
    }
    else
    {
        for(int i = 0; i < rows; i++)
        {
            ebox_memcpy(adc.byte,ptr,8);
            ptr+=8;
            ebox_memcpy(value.byte,ptr,8);
            ptr+=8;

            data1[i][0] = (double)adc.value;
            data1[i][1] = (double)value.value;
        }
        linear_regression((double*)data1,rows,&answer[0],&answer[1],&SquarePoor[0]);
        display((double*)data1,answer,&SquarePoor[0],rows,2);
    
       
        pt100.offsetPt.value = answer[0];
        pt100.ratioPt.value = answer[1];
        pt100.ccPt.value = sqrt(SquarePoor[0] / (SquarePoor[0] + SquarePoor[1]));
        
 
        ddc_nonblocking(&pt100.ccPt.byte[0],8,DDC_NoAck,10);

        pt100.ptMode = 1;

    }
    ebox_memcpy(&data[0],pt100.ratioPt.byte,8);
    ebox_memcpy(&data[8],pt100.offsetPt.byte,8);
    
    ddc_nonblocking(data,16,DDC_NoAck,11);

    adjust_flag_pt = 1;
    adjust_save(&pt100);
    uart1.printf("\r\n***********para Pt saved!****************\r\n");
    PB8.toggle();

}

void adjust_rx(uint8_t *ptr, uint16_t len )
{
    uint8_t data[16];
    DataDouble_t adc;
    DataDouble_t value;
    uint8_t rows = len/16;

    
    DataFloat_t value_cc;
    if(rows == 1)
    {
        ebox_memcpy(adc.byte,ptr,8);
        ptr+=8;
        ebox_memcpy(value.byte,ptr,8);

        if(value.value == 0)
        {
            pt100.ratioRx.value = 538.667;
            pt100.offsetRx.value = adc_value1.value - adc_value2.value;//差值
            pt100.ccRx.value = 100;
            ddc_nonblocking(pt100.ccRx.byte,8,DDC_NoAck,10);

        }
        pt100.rxMode = 0;
    }
    else
    {
        for(int i = 0; i < rows; i++)
        {
            ebox_memcpy(adc.byte,ptr,8);
            ptr+=8;
            ebox_memcpy(value.byte,ptr,8);
            ptr+=8;

            data1[i][0] = (double)adc.value;
            data1[i][1] = (double)value.value;
        }
        linear_regression((double*)data1,rows,&answer[0],&answer[1],&SquarePoor[0]);
        display((double*)data1,answer,&SquarePoor[0],rows,2);
    

        pt100.offsetRx.value  = answer[0];
        pt100.ratioRx.value = answer[1];
        pt100.ccRx.value = sqrt(SquarePoor[0] / (SquarePoor[0] + SquarePoor[1]));
        
        ddc_nonblocking(pt100.ccRx.byte,8,DDC_NoAck,10);

        pt100.rxMode = 1;

    }
    ebox_memcpy(&data[0],pt100.ratioRx.byte,8);
    ebox_memcpy(&data[8],pt100.offsetRx.byte,8);    
    ddc_nonblocking(data,16,DDC_NoAck,11);


    adjust_flag_rx = 1;
    adjust_save(&pt100);
    uart1.printf("\r\n***********para Rx saved!****************\r\n");

    PB9.toggle();

}
void calibrate()
{
    uint8_t *p;
    uint8_t *px;
    uint8_t data[20];

    uint8_t len;
    uint32_t ajust_timer;
    

    PB8.mode(OUTPUT_PP);
    PB9.mode(OUTPUT_PP);
    PB8.set();
    ddc_attach_chx(1,calibrate_para);
    ddc_attach_chx(2,adjust_rx);
    adjust_flag_rx = 0;
    adjust_flag_pt = 0;
    if(adjust_check() == true)
    {
        uart1.printf("\r\n***********para saved!****************\r\n****************\r\n****************\r\n****************\r\n****************\r\n****************\r\n");
        adjust_read(&pt100);
    }
    while(1)
    {
        if(millis() - ajust_timer > 1000)
        {
            PA5.toggle();
            if(adjust_check() == true)
            {
                uart1.printf("\r\noffset rx:%f\r\n",pt100.offsetRx.value);
                uart1.printf("ratio rx:%f\r\n",pt100.ratioRx.value);
                uart1.printf("cc rx:%f\r\n",pt100.ccRx.value);
                
                uart1.printf("offset pt:%f\r\n",pt100.offsetPt.value);
                uart1.printf("ratio pt:%f\r\n",pt100.ratioPt.value);
                uart1.printf("cc pt:%f\r\n",pt100.ccPt.value);
            }
            ajust_timer = millis();
            adc_value.value = adc.read_average(ADC_AIN0);
            adc_voltage.value = adc.adc_to_voltage(adc_value.value);
            data[0] = adc_value.byte[0];
            data[1] = adc_value.byte[1];
            data[2] = adc_value.byte[2];
            data[3] = adc_value.byte[3];
            data[4] = adc_voltage.byte[0];
            data[5] = adc_voltage.byte[1];
            data[6] = adc_voltage.byte[2];
            data[7] = adc_voltage.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,1);

            adc_value1.value = adc.read_average(ADC_AIN1);
            adc_voltage1.value = adc.adc_to_voltage(adc_value1.value);
            data[0] = adc_value1.byte[0];
            data[1] = adc_value1.byte[1];
            data[2] = adc_value1.byte[2];
            data[3] = adc_value1.byte[3];
            data[4] = adc_voltage1.byte[0];
            data[5] = adc_voltage1.byte[1];
            data[6] = adc_voltage1.byte[2];
            data[7] = adc_voltage1.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,2);


            adc_value2.value = adc.read_average(ADC_AIN2);
            adc_voltage2.value = adc.adc_to_voltage(adc_value2.value);
            data[0] = adc_value2.byte[0];
            data[1] = adc_value2.byte[1];
            data[2] = adc_value2.byte[2];
            data[3] = adc_value2.byte[3];
            data[4] = adc_voltage2.byte[0];
            data[5] = adc_voltage2.byte[1];
            data[6] = adc_voltage2.byte[2];
            data[7] = adc_voltage2.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,3);
 


            adc_value3.value = adc.read_average(ADC_AIN3);
            adc_voltage3.value = adc.adc_to_voltage(adc_value3.value);
            data[0] = adc_value3.byte[0];
            data[1] = adc_value3.byte[1];
            data[2] = adc_value3.byte[2];
            data[3] = adc_value3.byte[3];
            data[4] = adc_voltage3.byte[0];
            data[5] = adc_voltage3.byte[1];
            data[6] = adc_voltage3.byte[2];
            data[7] = adc_voltage3.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,4);



            pt100.rxOrigin.value = (adc_voltage1.value - adc_voltage2.value )/101;
            if(pt100.rxMode == 0)
                pt100.rx.value = (adc_value1.value -  adc_value2.value - pt100.offsetRx.value)/538.667;
            else
                pt100.rx.value = adc_value1.value*pt100.ratioRx.value + pt100.offsetRx.value;
                data[0] = pt100.rx.byte[0];
                data[1] = pt100.rx.byte[1];
                data[2] = pt100.rx.byte[2];
                data[3] = pt100.rx.byte[3];
                data[4] = pt100.rxOrigin.byte[0];
                data[5] = pt100.rxOrigin.byte[1];
                data[6] = pt100.rxOrigin.byte[2];
                data[7] = pt100.rxOrigin.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,5);



            if(pt100.ptMode == 0)
                pt100.rt.value = (adc_value.value  - pt100.offsetPt.value)/85.333;
            else
                pt100.rt.value = adc_value.value * pt100.ratioPt.value + pt100.offsetPt.value - pt100.rx.value;
            
            pt100.temp.value = RtoT(pt100.rt.value,1);

            data[0] = pt100.rt.byte[0];
            data[1] = pt100.rt.byte[1];
            data[2] = pt100.rt.byte[2];
            data[3] = pt100.rt.byte[3];
            data[4] = pt100.temp.byte[0];
            data[5] = pt100.temp.byte[1];
            data[6] = pt100.temp.byte[2];
            data[7] = pt100.temp.byte[3];
            ddc_nonblocking(data,8,DDC_NoAck,6);


            if(adjust_flag_rx == 1 && adjust_flag_pt == 1)
            {
                adjust_save(&pt100);
                uart1.printf("\r\n====ajust ok!!!!!====\r\n",ebox_get_free());
                return ;
            
            }
        }
    }
}

bool adjust_check()
{
    DataU32_t flag;
    iflash.read(0x800ff00,flag.byte,4);
    if(flag.value == 0x55aa)
        return true;
    else 
        return false;
}
void adjust_save(PtData_t *data)
{
    DataU32_t flag;
    flag.value = 0x55aa;
    iflash.write(0x800ff00,flag.byte,4);
    iflash.write(0x800ff10,(uint8_t*)data,sizeof(PtData_t));
}
void adjust_read(PtData_t *data)
{
    iflash.read(0x800ff10,(uint8_t*)data,sizeof(PtData_t));
}
PtData_t adjust_test()
{
    PtData_t x;

    
    return x;
}

