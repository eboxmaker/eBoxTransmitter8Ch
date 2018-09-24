#ifndef __PT100_H
#define __PT100_H
#include "ebox.h"

typedef enum
{
    PT10 = 0,
    PT100,
    PT1000,
    
    CU50,
    CU100
}ResType_t;


typedef struct 
{
    DataDouble_t ratioPt;
    DataDouble_t offsetPt;
    DataDouble_t ccPt;

    DataDouble_t ratioRx;
    DataDouble_t offsetRx;  
    DataDouble_t ccRx;
    
    DataFloat_t rx;
    DataFloat_t rxOrigin;
    
    ResType_t   ptType;
    DataFloat_t rt;
    DataFloat_t temp;

}PtData_t;
extern PtData_t pt100;


double RtoT(double R, uint8_t type);


#endif