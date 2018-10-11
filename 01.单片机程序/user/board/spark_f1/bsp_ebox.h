/**
  ******************************************************************************
  * @file    bsp_ebox.h
  * @author  cat_li	
  * @version V1.0
  * @date    2018/07/31
  * @brief   硬件相关信息声明
  ******************************************************************************
  * @attention
  *
  * No part of this software may be used for any commercial activities by any form 
  * or means, without the prior written consent of shentq. This specification is 
  * preliminary and is subject to change at any time without notice. shentq assumes
  * no responsibility for any errors contained herein.
  * <h2><center>&copy; Copyright 2015 shentq. All Rights Reserved.</center></h2>
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __BSP_EBOX_H
#define __BSP_EBOX_H
#include "ebox.h"
#include "ads1118.h"
#include "pt100.h"


#define	HARDWARE	"ebox_spark，STM32F103C8T6"


#define  DEFAULT_TYPE PT1000

extern DataU32_t is_enter_adjust_flag;



extern Ads1118 adc;
extern Flash iflash;
extern Iwdg wdg;
extern Timer timer2;
void set_channel(uint8_t ch);
void set_ratio(ResType_t type);
void ddc_input();
void enter_adjust(uint8_t *ptr, uint16_t len);



extern void print_log(const char *name,const char *date);



#endif

