#ifndef _CHARGE_H_
#define _CHARGE_H_

#include "typedef.h"

enum
{
	POWER_ON = 1,
	POWER_OFF,
};

u32 get_ldo5v_online_flag(void);

void charge_mode_detect_ctl(u8 sw);

void ldo5v_detect_deal(u8 mode);

void charge_disconnect_bt_check(void);

#endif    //_CHARGE_H_
