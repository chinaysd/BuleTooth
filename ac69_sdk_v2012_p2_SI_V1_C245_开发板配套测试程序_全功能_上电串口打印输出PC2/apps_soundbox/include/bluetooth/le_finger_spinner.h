#ifndef _LE_LE_FINGER_SPINNER_H_
#define _LE_LE_FINGER_SPINNER_H_

typedef u32 (*blefs_comsend_cbk_t)(u8 *data,u16 len);

extern bool blefs_com_init(void);
extern void blefs_com_stop(void);
extern u32 blefs_comdata_parse(u8 *data, u16 len);
extern bool blefs_register_comsend(blefs_comsend_cbk_t cbk);

extern u16* sp_disp_buf;    //user can user this buf to display directly

#endif
