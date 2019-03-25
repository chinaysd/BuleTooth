#include "common/app_cfg.h"
#include "rtos/os_api.h"
#include "rtos/os_cfg.h"
#include "common/error.h"
#include "common/msg.h"
#include "rtos/task_manage.h"
#include "dac/dac_api.h"
#include "play_sel/play_sel.h"
#include "ui/led/led.h"
#include "dac/dac_api.h"
#include "dac/ladc.h"
#include "echo/echo_api.h"
#include "echo/echo_deal.h"
#include "echo_ui.h"
#include "echo.h"


#define ECHO_DEF_DEEP_VAL  1024 //默认深度 0-1024	//1024 -> echo_deep_max_set(vol)
#define ECHO_DEF_VOL_VAL   128  //默认强度 0-128

static const EF_REVERB_PARM2 ef_parm = 
{
	/* unsigned short deepval; 设置后，reverb_init初始化值会被覆盖*/
	ECHO_DEF_DEEP_VAL,

	/* unsigned short decayval; 设置后，reverb_init初始化值会被覆盖*/
	ECHO_DEF_VOL_VAL,

	/* unsigned short gainval;  音量增益 最大值：4096*/
    4000,

	/* unsigned short rs_mode;  1:高精度变采样    0:低精度变采样 */
	0x100,

	/* unsigned short filter_mode;  1:使用低通滤波器  0:不使用滤波器*/
	0,

	/* Low_pass_para  lp_para; (reserved)*/
	{
		0,0,0,0,0
	}
};



//agc：可设置范围(-27~76)
//-12db		-27~0
//normal	0~63
//x2		64~76
static const MIC_VAR_SET mic_parm = 
{
	/* u8 use_magc;//agc开关 */
    1,

	/* s8 max_gain;//agc最大增益(-27~76) && > min_gain*/
	30,

	/* s8 min_gain;//agc最小增益(-27~76) && < max_gain*/
	0,

	/* reserved */
	0,

	/* s32 tms;//爆掉之后的音量抑制多少ms */
	400,

	/* 破音阀值：s32 target  lg(coeff*32768)*20480 (建议值：80000-90000)*/
    85000,
};

static u8 reverb_switchv=0;	//DATA_HANDLE_FLAG_REVERB|DATA_HANDLE_FLAG_PITCH;
static AD2DA_CBUF *ad2da_ptr=NULL;

static void init_ad2da(void)
{
	ad2da_ptr=malloc(sizeof(AD2DA_CBUF));
	ASSERT(ad2da_ptr);
	reverb_ad2da_init(ad2da_ptr);
}

static int write_ad(short *buf)
{
	if(ad2da_ptr)
	{
		reverb_ad2da_wr(ad2da_ptr,buf);
		return 1;//设置ADC采样率,与DAC采样率一致
	}
	return 0;
}

void sync_adc_sr(short *buf)
{
    u16 sr_reg;

	write_ad(buf);

	if(ad2da_ptr){
		sr_reg=dac_get_samplerate();
		if(sr_reg != ladc_sr_get())
		{
			ladc_sr_set(sr_reg);
		}
	}
}

void rd_da(short *buf)
{
#if PITCH_EN		//变调时候，第一声必须获取数字通道
    if(ad2da_ptr)
    {
	   reverb_tad2da_rd(ad2da_ptr,buf);
   	}
#endif
}

void rd_da_rec(short *buf)//录音使用
{
#if (PITCH_EN == 0)
    if(ad2da_ptr)
    {
	   reverb_tad2da_rd_sp(ad2da_ptr,buf);
   	}
#endif
}

void rd_updata_rec(void)//录音使用
{
#if (PITCH_EN == 0)
	reverb_tad2da_rupdata(ad2da_ptr);
#endif
}

void echo_switch(void **mode_reverb, u8 on)
{
    if(on)
    {
        if(*mode_reverb)//not init
        {
            //already init
            puts("already init echo mic\n");
        }
        else
        {
			reg_mic_var((MIC_VAR_SET *)&mic_parm);

			reg_ef_reverb_parm2((EF_REVERB_PARM2 *)&ef_parm);

#if HOWLING_SUPPRESSION_EN
			//啸叫抑制值，范围128±2，建议值130
			howlingsuppress_suppression_val(130);
            howlingsuppress_sw(HOWLING_SUPPRESSION_EN);
#endif

#if PITCH_EN
            pitch_coff(80);/*64-128是把声音变沉，128-200是把声音变尖， 128 是正常 */
            pitch_sw(PITCH_EN);/*64-128是把声音变沉，128-200是把声音变尖， 128 是正常 */
#endif

			reverb_switchv = 0;
			reverb_switchv |=DATA_HANDLE_FLAG_REVERB;
			reverb_switchv |=DATA_HANDLE_FLAG_SR;

            puts("reverb_init\n");
            *mode_reverb = (void*)reverb_init(reverb_switchv);
            if(*mode_reverb)
            {
				/* set_reverb_parm(*mode_reverb,ECHO_DEF_DEEP_VAL,ECHO_DEF_VOL_VAL); */
				/* set_pitch_parm(*mode_reverb, 80); */

				/* echo_set_mic_vol(gain, 1);//*/
				ladc_pga_gain(2, 0);//2:mic_channel, 0:gain

#if (PITCH_EN == 0)
				mic_2_LR(1, 1);
#endif
				init_ad2da();

                puts("init echo mic succ\n");
            }
            else
            {
                puts("init echo mic err\n");
            }
        }
    }
    else
    {
        puts("reverb_stop\n");
        reverb_stop(*mode_reverb);

#if (PITCH_EN == 0)
		mic_2_LR(0, 0);
#endif

		if(ad2da_ptr)
		{
			free(ad2da_ptr);
			ad2da_ptr=NULL;
		}

        *mode_reverb = NULL;
        puts("reverb_stop_ok\n");
    }
}

void echo_msg_deal(void **mode_reverb, int *msg)
{
    int flag = 1;
	static u16 deep = 0;
	static u8 coff = 80;

    switch(msg[0])
    {
    case MSG_ECHO_START:
        if(*mode_reverb)
       	{
            puts("echo already running\n");
        }
        else
       	{
            echo_switch(mode_reverb,1);
        }
        break;

    case MSG_ECHO_STOP:
		if(*mode_reverb)
		{
			echo_switch(mode_reverb,0);
		}
		else
		{
			puts("echo not run\n");
		}
		break;

    case MSG_ECHO_SET_PARM:
		deep = (deep + 100 > 1024)? 0 : deep+100;
		printf("coff = %d\n", deep);
		set_reverb_parm(*mode_reverb, deep, -1);
		break;

    case MSG_PITCH_SET_PARM:
		coff = (coff + 10 > 200)? 80:coff+10;
		printf("coff = %d\n", coff);
		pitch_coff(coff);
		break;

    default:
        flag = 0;
        break;
    }

    if(flag != 0)
    {
        UI_menu(MENU_REFRESH);
    }
}

void echo_exit(void **mode_reverb)
{
	echo_switch(mode_reverb,0);
}

