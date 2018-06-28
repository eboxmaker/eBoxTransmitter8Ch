#ifndef __EBOX_CFUN_H
#define __EBOX_CFUN_H

#ifdef __cplusplus
 extern "C" {
#endif
     
#include "math.h"
     
#include "../core/stdFun/big_little.h"
#include "../core/stdFun/crc.h"
#include "../core/stdFun/ebox_mem.h"
#include "../core/stdFun/ebox_printf.h"
#include "../core/stdFun/fifo.h"
#include "../core/stdFun/itoa.h"
#include "../core/stdFun/math_misc.h"
#include "../core/stdFun/util.h"
     

extern void        (*ebox_reset)();
extern uint64_t    (*micros)();
extern uint64_t    (*millis)();
extern void        (*delay_ms)(uint64_t ms);
extern void        (*delay_us)(uint64_t ms);
//extern void        (*interrupts)(void);
//extern int         (*no_interrupts)(void);

extern Cpu_t cpu;

#ifdef __cplusplus
}
#endif


#endif
