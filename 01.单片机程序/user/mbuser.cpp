#include "mbuser.h"
#include "bsp.h"
#include "adjust.h"
#include "freemodbus.h"
#define LED1_ON   PB8.set()
#define LED1_OFF  PB8.reset()

#define LED2_ON   PB9.set()
#define LED2_OFF  PB9.reset()

#define  BUTTON1_READ()  PA8.read()

Timer timer4(TIM4);


void FreeModbusIoConfig(void)
{ 
    modbus.uart=&uart2;
	modbus.timer=&timer4;
	modbus.Mode4851=&PA4;
	modbus.Mode4852=&PA4;
	
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
    usRegInputBuf[0]=pt100.temp.value;
    usRegInputBuf[0]=pt100.temp.value;
    usRegInputBuf[0]=pt100.temp.value;
	
}
void hold_update_ch(uint8_t ch,PtData_t *pt)
{
    usRegHoldingBuf[ch] = pt->temp.value*10;
    usRegHoldingBuf[ch + 20] = pt->rx.value*100;
    if(pt->ptType == PT10)
        usRegHoldingBuf[ch + 10] = pt->rt.value*1000;
    else if(pt->ptType == PT100)
        usRegHoldingBuf[ch + 10] = pt->rt.value*100;
    else if(pt->ptType == PT1000)
        usRegHoldingBuf[ch + 10] = pt->rt.value*10;
}


void modbus_init()
{
	FreeModbusIoConfig();
	FreemodbusConfig();
}
void modbus_loop()
{
    static ResType_t temp_type = PT100;
    
    FreemodbusPoll();
//    LED_Poll();
    Button_Poll();
//    Adc_Poll();
    if(usRegHoldingBuf[30] == 1)
    {
        is_enter_adjust_flag.value = 0x55aa;
        usRegHoldingBuf[30] = 0;
    }
    if((ResType_t) usRegHoldingBuf[31] != temp_type)
    {
        if( usRegHoldingBuf[31] > 2 || usRegHoldingBuf[31] < 0)//非法参数。重新设置为默认值，并保存
            usRegHoldingBuf[31] = 1;
        
        pt100.ptType = (ResType_t) usRegHoldingBuf[31];
        temp_type = (ResType_t) usRegHoldingBuf[31];
        set_ratio(pt100.ptType);

   
        adjust_save(&pt100);
    }
    


}