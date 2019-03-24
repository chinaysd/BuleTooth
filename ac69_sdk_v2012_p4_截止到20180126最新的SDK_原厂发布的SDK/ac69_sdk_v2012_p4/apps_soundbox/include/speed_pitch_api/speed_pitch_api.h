#ifndef __SPEED_PITCH_API_H__
#define __SPEED_PITCH_API_H__
#include "typedef.h"
#include "dec/decoder_phy.h"

typedef struct __PS_OBJ	PS_OBJ;

#define PS_PITCHT_DEFAULT_VAL	(32768L)	///变调参数说明：>32768音调高，<32768音调低，变调比例pitchV/32768
#define PS_SPEED_DEFAULT_VAL	(80L)		///变数参数说明，>80变快，<80变慢,建议范围：40_160, 但是20-200也有效

PS_OBJ	*speed_pitch_creat(void);
void speed_pitch_destroy(PS_OBJ **obj);
void speed_pitch_set_output(PS_OBJ *obj, void *cbk, void *priv);
void speed_pitch_set_set_info_cbk(PS_OBJ *obj, u32(*cbk)(void *priv, dec_inf_t *inf, tbool wait), void *priv);
void speed_pitch_set_samplerate(PS_OBJ *obj, u16 sr);
void speed_pitch_set_track_nums(PS_OBJ *obj, u16 track);
void speed_pitch_set_speed(PS_OBJ *obj, u16 val);
void speed_pitch_set_pitch(PS_OBJ *obj, u16 val);
u16 speed_pitch_get_cur_speed(PS_OBJ *obj);
u16 speed_pitch_get_cur_pitch(PS_OBJ *obj);
void speed_pitch_main(PS_OBJ *obj, void *buf, u32 len);

#endif// __SPEED_PITCH_API_H__
