#ifndef __MBUSER_H
#define __MBUSER_H
#include "freemodbus.h"
#include "bsp.h"
void modbus_init();
void modbus_loop();


void hold_update_ch(uint8_t ch,PtData_t *pt);


#endif
