#ifndef __RECORD_H__
#define __RECORD_H__

#include "includes.h"
#include "encode/encode.h"

// #define ENC_DEBUG

#ifdef ENC_DEBUG
    #define enc_puts     puts
    #define enc_printf   printf
#else
    #define enc_puts(...)
    #define enc_printf(...)
#endif

#define REC_TASK_NAME        "RECTask"
#define ENC_RUN_TASK_NAME       "EncRunTask"
#define ENC_WFILE_TASK_NAME     "EncWFileTask"

extern const struct task_info rec_task_info;

extern void bt_rec_buf_write(s16* buf,u32 len);
extern void rec_get_dac_data(RECORD_OP_API * rec_api, s16 *buf, u32 len);//录音获取pcm数据函数

#endif/*__RECORD_H__*/
