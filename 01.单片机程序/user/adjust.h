#ifndef __ADJUST_H
#define __ADJUST_H
#include "ebox.h"
#include "LinearRegression.h"
#include "pt100.h"


void calibrate();


bool adjust_check();
void adjust_save(PtData_t *data);
void adjust_read(PtData_t *data);

#endif
