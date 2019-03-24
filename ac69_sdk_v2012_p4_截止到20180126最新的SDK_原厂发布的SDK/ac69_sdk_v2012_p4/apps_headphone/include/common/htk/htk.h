#ifndef __HTK_H__
#define __HTK_H__
#include "sdk_cfg.h"

typedef tbool(*act_cb_fun)(void *priv, char result[], char nums, u8 isTimeout);
typedef int (*msg_cb_fun)(void *priv);
extern void htk_isr_callback(void *ladc_buf, u32 buf_flag, u32 buf_len);


extern void htk_start(u8 type);//iphone连接来电话时,内存资源不够，如果来电话播完提示音再启动,type参数为1
extern void htk_task_init(void);
extern void htk_task_exit(void);
extern u8 get_htk_state();
extern void set_htk_enable(u8 en);
tbool htk_player_process(void *priv, act_cb_fun act_cb, msg_cb_fun msg_cb, u16 timeout);

#endif//__HTK_H__

