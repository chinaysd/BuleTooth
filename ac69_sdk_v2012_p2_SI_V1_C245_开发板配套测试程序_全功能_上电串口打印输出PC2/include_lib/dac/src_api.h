/*
*********************************************************************************************************
*                                             BC51
*
*                                             CODE
*
*                          (c) Copyright 2015-2016, ZHUHAI JIELI
*                                           All Rights Reserved
*
* File : *
* By   : jamin.li
* DATE : 11/11/2015 build this file
* junqian 20170329 add para inbuf_len,outbuf_len,kick_start_len,cbuf_len 
*********************************************************************************************************
*/
#ifndef _SRC_H_
#define _SRC_H_

#include "rtos/os_api.h"
#include "hw_cpu.h"
#include "cpu.h"
#include "cbuf/circular_buf.h"
#include "common/printf.h"
#include "mem/malloc.h"

typedef struct{
    u16 inbuf_len;      ///SRC输入中断发生时，SRC模块一次从cbuf中输入的数据量。default is 128
    u16 outbuf_len;     ///SRC输出中断发生时，SRC模块一次输出的数据量。 default is 128
    u16 cbuf_len;       //输入缓存，大小是inbuf_len的整数倍。default is 128*12
    u16 kick_start_len; //当输入缓存中达到此数据量时，自动起动SRC转换。大小是inbuf_len的整数倍. default is 128*4
}src_buf_param_t;    //可以在src_init_api函数之前，调用此函数改变SRC内部BUF的配置，若不调用此函数，则使用默认配置。


typedef struct
{
    u8 nchannel;        //一次转换的通道个数，取舍范围(1 ~ 8)，最大支持8个通道
    u8 reserver[3]; 
    u16 in_rate;        ///输入采样率
    u16 out_rate;       ///输出采样率
    u16 in_chinc;       ///输入方向,多通道转换时，每通道数据的地址增量
    u16 in_spinc;       ///输入方向,同一通道后一数据相对前一数据的地址增量
    u16 out_chinc;      ///输出方向,多通道转换时，每通道数据的地址增量
    u16 out_spinc;      ///输出方向,同一通道后一数据相对前一数据的地址增量 
    void (*output_cbk)(u8* ,u16 ); ///一次转换完成后，输出中断会调用此函数用于接收输出数据，数据量大小由outbuf_len决定
} src_param_t;

int src_init_api(src_param_t *parm);  //初始化SRC,申请SRC需要的RAM等。初始化成功则返回0,失败则返回其它值。
u32 src_write_data_api(u8 *buf , u16 len);//向输入缓存cbuf中写数据,返回值为实际写入的数据量
void src_clear_data_api(void);//清除输入缓存cbuf中的数据。
void src_exit_api(void);//关闭SRC,释放SRC使用的RAM等
u32 get_srccbuf_total_len(void);  //input cbuf total len
u32 get_srccbuf_idle_len(void);   //input cbuf idle ram len
u32 get_srccbuf_data_len(void);   //input cbuf unread data len
int src_init_buf_param(src_buf_param_t *parm); //可以在src_init_api函数之前，调用此函数改变SRC内部BUF的配置，若不调用此函数，则使用默认配置。
#endif
