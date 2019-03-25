#include "iis.h"
#include "dac/src_api.h"
#include "dac/dac_api.h"
#include "uart.h"
#include "dac.h"
#include "sys_detect.h"
#include "iis/wm8978.h"
#include "sdk_cfg.h"
#include "dac/dac_api.h"
#include "dac/eq_api.h"
#include "cbuf/circular_buf.h"
#include "vm/vm_api.h"
#include "key_drv/key_voice.h"
#include "rtos/os_api.h"
#include "aec/aec_api.h"
#include "echo/echo_api.h"
#include "common/app_cfg.h"
#include "dac/ladc.h"
#include "common/common.h"
#include "cpu/dac_param.h"
#include "dac/dac.h"
#include "dac/eq.h"
#include "timer.h"
#include "echo/echo_deal.h"
#include "play_sel/play_sel.h"
#include "fm/fm_radio.h"
#include "dac/src_api.h"

#if DAC2IIS_EN
extern void audio_link_init(void);
extern int dac_cbuf_enough(void);
extern  u32 read_buf[DAC_SAMPLE_POINT];
extern int check_a2dp_en();
extern void inc_dac_cnt(u32 dac_points);
extern void src_set_rate(u16 in_rate,u16 out_rate);
extern u8 dac_read_en;
extern DAC_CTL dac_ctl;
#if  0 
#define   putc_ds      putchar
#define   puts_ds      puts
#define   printf_ds    printf
#else
#define   putc_ds(...)
#define   puts_ds(...)
#define   printf_ds(...)
#endif

#define SRC2IIS_OBUF_LEN     128*6*2  //44.1/8 = 6.  
typedef struct{
    u16 dac_sr;         //cur samplerate
    u16 out_in_ratio;   //out_samplerate/in_samplerate
    tbool outbuf_read_en;
    cbuffer_t cb_out;   //src out cbuf control 
}dac2iis_param_t;
dac2iis_param_t  dac2iis_p;

u8 src2iis_outbuf[SRC2IIS_OBUF_LEN] AT(.dac_buf_sec);

void dac2iis_sr(u16 rate)
{
    if(rate != dac2iis_p.dac_sr){
        dac2iis_p.dac_sr = rate;
        src_clear_data_api(); 
        if(rate != 44100){ 
            src_set_rate(rate,44100);
        }
        if(11025==rate || 22050==rate || 44100==rate){
            dac2iis_p.out_in_ratio = 44100/rate;
        }else if(rate < 44100){
            dac2iis_p.out_in_ratio = 44100/rate+1;
        }else{ //48000
            dac2iis_p.out_in_ratio = 1;
        } 
        printf_ds("dac2iis_sr change = %d_%d\n",rate,dac2iis_p.out_in_ratio);
    }
}

u32 dac2iis_outbuf_freebuf_len(void)
{
    return dac2iis_p.cb_out.total_len - dac2iis_p.cb_out.data_len;
}

u32 dac2iis_outbuf_unread_data_len(void)
{
    return dac2iis_p.cb_out.data_len;
}

u32 dac2iis_inbuf_unread_data_len(void)
{
    return get_srccbuf_data_len();
}

u32 dac2iis_outbuf_read(u8* buf, int len)
{
   return cbuf_read(&dac2iis_p.cb_out,buf,len);
}

void dac2iis_outbuf_write(s16 *buf, u32 len)
{
    if(dac2iis_outbuf_freebuf_len() >= len){
        //putc_ds('o');
        cbuf_write(&dac2iis_p.cb_out,buf,len);
    }else{
        putc_ds('n');
    }
}


void dac2iis_src_open_ctrl(int open_flag)
{
    src_param_t src_p;
    src_buf_param_t  src_buf_p;
    if(open_flag == 1){     
        puts_ds("dac2iis_src_start \n");
        src_buf_p.inbuf_len = 128;    //SRC输入中断发生时，SRC模块一次从cbuf中输入的数据量
        src_buf_p.outbuf_len = 128;   //SRC输出中断发生时，SRC模块一次输出的数据量
        src_buf_p.cbuf_len = src_buf_p.inbuf_len*3;     //输入缓存cbuf大小 
        src_buf_p.kick_start_len = src_buf_p.inbuf_len ;    //向输入缓存存入的数据量达到底kick_start_len时，自动启动SRC转换
        if(src_init_buf_param(&src_buf_p) !=0 ){    //更改内部BUF的配置。 
		    puts_ds("!!!dac2iis src cfg fail\n");
		}
        src_p.in_chinc = 1;
        src_p.in_spinc = 2;
        src_p.out_chinc = 1;
        src_p.out_spinc = 2;
        src_p.in_rate =8000;
        src_p.out_rate =44100; 
        src_p.nchannel = 2;
        src_p.output_cbk = (void *)dac2iis_outbuf_write;
        if(src_init_api(&src_p) == 0){
            puts_ds("srcx open ok\n");
        }else{
            puts_ds("srcx open fail\n");
        }
     }else{
        src_exit_api();
        puts_ds("test_src_stop\n");
     }
}

void dac2iis_init(void)
{

#if IISCHIP_WM8978_EN 
    u8 WM8978_Init(void);
    WM8978_Init();
#endif
    memset(&dac2iis_p,0x00,sizeof(dac2iis_param_t));
    cbuf_init(&dac2iis_p.cb_out,src2iis_outbuf,SRC2IIS_OBUF_LEN);
    dac2iis_src_open_ctrl(1);
    audio_link_init(); 
}

void dac2iis_inbuf_write(u8 *buf, u32 len)  //向SRC缓存buf中写数据
{
    //putc_ds('i');
    src_write_data_api(buf,len);
}


int dac_read_and_mix(void) //read form audio_buf, and mix audio data 
{
    int read_ret;
    read_ret = dac_read(read_buf,DAC_BUF_LEN); 
	if(is_dac_mute()) {
		memset(read_buf,0x00,DAC_BUF_LEN);
	}
	else{
#if VOCAL_REMOVER
		if((vocal_flag)&&(get_decode_nch() == 2)){
			dac_digital_lr_sub(read_buf,DAC_BUF_LEN);
		}
#endif

#if DAC_SOUNDTRACK_COMPOUND
		dac_digital_lr2one(read_buf,DAC_BUF_LEN);
#endif
	}

	dac_digit_energy_value(read_buf, DAC_BUF_LEN);
    digital_vol_ctrl(read_buf, DAC_BUF_LEN);

#if ECHO_EN
    if(fm_mode_var && ((fm_mode_var->scan_mode >= FM_SCAN_BUSY)|| (fm_mode_var->fm_mute)))   ///FM正在搜台，只响应部分按键 //MUTE
	{
	}
	else
	{
		cbuf_mixture_echo(read_buf,DAC_BUF_LEN);
	}
#endif

#if REC_EN
	rec_get_dac_data(rec_bt_api, (s16 *)read_buf, DAC_BUF_LEN);	//获取蓝牙和混响声音
#if (REC_SOURCE == 1)//get data from dac
	rec_get_dac_data(rec_fm_api, (s16 *)read_buf, DAC_BUF_LEN);	//获取FM和混响声音
	rec_get_dac_data(rec_aux_api, (s16 *)read_buf, DAC_BUF_LEN);	//获取AUX和混响声音
#endif

#endif

	if((aec_interface.fill_dac_echo_buf)&&(read_ret == DAC_BUF_LEN)) 
	{
		aec_interface.fill_dac_echo_buf((s16*)read_buf,DAC_SAMPLE_POINT << 1);
	}
    return read_ret;
}

void dac2iis_outbuf_try_read(void* iis_buf, u16 kick_read_len)
{    
        if(dac2iis_outbuf_unread_data_len()>= kick_read_len ){  //512 byte, kick to get SRC result data.
            dac2iis_p.outbuf_read_en = true;
        }
     
       if(dac2iis_p.outbuf_read_en){
           if(dac2iis_outbuf_unread_data_len()>=128){  //read success
                //putc_ds('r');
                dac2iis_outbuf_read((void*)read_buf,128);  //get SRC result data
#if KEY_TONE_EN
            	add_key_voice((s16*)read_buf,DAC_SAMPLE_POINT*2);
#endif

#if EQ_EN
                eq_run((short*)read_buf,(short*)iis_buf,DAC_SAMPLE_POINT);
#else
	            memcpy(iis_buf,read_buf,DAC_BUF_LEN);
#endif
                return; 
           }else{
                dac2iis_p.outbuf_read_en = false;
                putc_ds('n');
           }
       }
       memset(iis_buf,0x00,DAC_BUF_LEN);
}

//--------------------------------------------------------------------------------
void iis_isr_callback(void * iis_buf)
{
#if DAC_SINE_DEBUG
	memcpy(iis_buf,sine_buf_32K,128);
	return;
#endif

#if BT_TWS 
      inc_dac_cnt(32);
#endif

	if(0 == dac_read_en)
	{
		if(dac_cbuf_enough() != 0)	{
           if((dac2iis_outbuf_unread_data_len()>=128) && (44100 != dac2iis_p.dac_sr)){ //read src outbuf remain data 
               dac2iis_outbuf_try_read(iis_buf,128);
           }else{
			   memset(iis_buf,0x00,DAC_BUF_LEN);
		    	#if KEY_TONE_EN
                add_key_voice((s16*)iis_buf,DAC_SAMPLE_POINT*2);
		    	#endif
			   dac_digit_energy_value(iis_buf, DAC_BUF_LEN);
           }
           return;
		}
		dac_read_en = 1;
	}

    if(44100 == dac2iis_p.dac_sr){     //44.1k direct to iis, not passed SRC.
         if(dac_read_and_mix() != DAC_BUF_LEN) {    
		     dac_read_en = 0;
             putc_ds('o'); 
         }else{
            dac2iis_outbuf_write((void*)read_buf,DAC_BUF_LEN);
         }
    }else{
        //write data to SRC incbuf.
        u32 blklen = 0;
        u32 outblk = dac2iis_outbuf_freebuf_len()/128/dac2iis_p.out_in_ratio;
        u32 inblk = dac2iis_inbuf_unread_data_len()/128;
        //printf_ds(">%d_%d ",outblk,inblk);
        if(inblk >= outblk){// src_incbuf is enough to change
             blklen = 0; 
        }else{ //actual fill blks. 
             blklen = outblk - inblk;
        }
	    while(blklen--){
            if(dac_read_and_mix() != DAC_BUF_LEN) {   
                dac_read_en = 0;
	        }
		    dac2iis_inbuf_write((void*)read_buf,DAC_BUF_LEN);   //write to src incbuf. //if dac_read fail,here also fill a blk of ZERO to flush src outbuf
	    }
    }  
    dac2iis_outbuf_try_read(iis_buf,128*4);
	dac_cbuf_write_detect();
}

















#endif  //DAC2IIS EN
