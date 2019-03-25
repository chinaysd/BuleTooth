#ifndef TIMER_H
#define TIMER_H

#include "typedef.h"
#include "cpu/timer_api.h"


s32 timer0_init();

s32 timer2_init();

void set_tick_timer(u32 hz );
void init_tick_timer(u32 hz);

extern u32 make_seed(void);
extern u8 get_sys_halfsec(void);
extern u32 get_sys_2msCount(void);


extern __timer_handle  *timer0_hl;

#endif

