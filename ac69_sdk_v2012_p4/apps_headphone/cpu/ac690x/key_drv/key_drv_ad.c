#include "includes.h"
#include "key_drv/key_drv_ad.h"
#include "key_drv/key.h"
#include "clock_api.h"
#include "adc_api.h"
#include "timer.h"
#include "sys_detect.h"
#include "rtc/rtc_api.h"
#include "clock.h"


#define AD_KEY_IO_PAX   6 

#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

	#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD)
		u16 adc_value[4];
		const u32 ad_table[] =
		{
		#if KEY_AD_EN
			AD_KEY_PR2,
			AD_KEY_PR1,
		#endif
			AD_KEY_LDOIN,
			AD_KEY_LDOREF,
		};

	#elif(ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)
		u16 adc_value[3];
		const u32 ad_table[] =
		{
		#if KEY_AD_EN
			AD_KEY_PR2,
		#endif
			AD_KEY_LDOIN,
			AD_KEY_LDOREF,
		};
	#endif  //(ADC_PWR_SOURCE == ADC_PWR_RTCVDD)

#define FULL_ADC   0x3ffL 
#define R_UP       220     //22K

#define ADC10_33(x)   (x)
#define ADC10_30(x)   (x*2200L/(2200 + R_UP))     //220K
#define ADC10_27(x)   (x*1000L/(1000 + R_UP))     //100K
#define ADC10_23(x)   (x*510L /(510  + R_UP))     //51K
#define ADC10_20(x)   (x*330L /(330  + R_UP))     //33K
#define ADC10_17(x)   (x*240L /(240  + R_UP))     //24K
#define ADC10_13(x)   (x*150L /(150  + R_UP))     //15K
#define ADC10_10(x)   (x*91L  /(91   + R_UP))     //9.1K
#define ADC10_07(x)   (x*62L  /(62   + R_UP))     //6.2K
#define ADC10_04(x)   (x*30L  /(30   + R_UP))     //3K
#define ADC10_00(x)   (0)

#define AD_NOKEY(x)     ((ADC10_33(x) + ADC10_30(x))/2)
#define ADKEY1_0(x)		((ADC10_30(x) + ADC10_27(x))/2)
#define ADKEY1_1(x)		((ADC10_27(x) + ADC10_23(x))/2)
#define ADKEY1_2(x)		((ADC10_23(x) + ADC10_20(x))/2)
#define ADKEY1_3(x)		((ADC10_20(x) + ADC10_17(x))/2)
#define ADKEY1_4(x)		((ADC10_17(x) + ADC10_13(x))/2)
#define ADKEY1_5(x)		((ADC10_13(x) + ADC10_10(x))/2)
#define ADKEY1_6(x)		((ADC10_10(x) + ADC10_07(x))/2)
#define ADKEY1_7(x)		((ADC10_07(x) + ADC10_04(x))/2)
#define ADKEY1_8(x)		((ADC10_04(x) + ADC10_00(x))/2)

u16 ad_rtcvdd_key_table[] =
{
    ADKEY1_0(FULL_ADC),ADKEY1_1(FULL_ADC),ADKEY1_2(FULL_ADC),ADKEY1_3(FULL_ADC),ADKEY1_4(FULL_ADC),
    ADKEY1_5(FULL_ADC),ADKEY1_6(FULL_ADC),ADKEY1_7(FULL_ADC),ADKEY1_8(FULL_ADC)
};

static void set_rtcvdd_table(u16 adc_rtcvdd)
{
	u16 rtcvdd;
	rtcvdd = adc_rtcvdd;
    ad_rtcvdd_key_table[0] = ADKEY1_0(rtcvdd);
    ad_rtcvdd_key_table[1] = ADKEY1_1(rtcvdd);
    ad_rtcvdd_key_table[2] = ADKEY1_2(rtcvdd);
    ad_rtcvdd_key_table[3] = ADKEY1_3(rtcvdd);
    ad_rtcvdd_key_table[4] = ADKEY1_4(rtcvdd);
    ad_rtcvdd_key_table[5] = ADKEY1_5(rtcvdd);
    ad_rtcvdd_key_table[6] = ADKEY1_6(rtcvdd);
    ad_rtcvdd_key_table[7] = ADKEY1_7(rtcvdd);
    ad_rtcvdd_key_table[8] = ADKEY1_8(rtcvdd);
}	

#else          //ADC_PWR_VDDIO33

u16 adc_value[3];
const u32 ad_table[] =
{
#if KEY_AD_EN
	AD_KEY_PR2,
#endif
    AD_KEY_LDOIN,
    AD_KEY_LDOREF,
};

#define R_UP       220     //22K

#define ADC10_33   (0x3ffL)
#define ADC10_30   (0x3ffL*2200/(2200 + R_UP))     //220K
#define ADC10_27   (0x3ffL*1000/(1000 + R_UP))     //100K
#define ADC10_23   (0x3ffL*510 /(510  + R_UP))     //51K
#define ADC10_20   (0x3ffL*330 /(330  + R_UP))     //33K
#define ADC10_17   (0x3ffL*240 /(240  + R_UP))     //24K
#define ADC10_13   (0x3ffL*150 /(150  + R_UP))     //15K
#define ADC10_10   (0x3ffL*91  /(91   + R_UP))     //9.1K
#define ADC10_07   (0x3ffL*62  /(62   + R_UP))     //6.2K
#define ADC10_04   (0x3ffL*30  /(30   + R_UP))     //3K
#define ADC10_00   (0)

#define AD_NOKEY        ((ADC10_33 + ADC10_30)/2)
#define ADKEY1_0		((ADC10_30 + ADC10_27)/2)
#define ADKEY1_1		((ADC10_27 + ADC10_23)/2)
#define ADKEY1_2		((ADC10_23 + ADC10_20)/2)
#define ADKEY1_3		((ADC10_20 + ADC10_17)/2)
#define ADKEY1_4		((ADC10_17 + ADC10_13)/2)
#define ADKEY1_5		((ADC10_13 + ADC10_10)/2)
#define ADKEY1_6		((ADC10_10 + ADC10_07)/2)
#define ADKEY1_7		((ADC10_07 + ADC10_04)/2)
#define ADKEY1_8		((ADC10_04 + ADC10_00)/2)


const u16 ad_key_table[] =
{
    ADKEY1_0,ADKEY1_1,ADKEY1_2,ADKEY1_3,ADKEY1_4,
    ADKEY1_5,ADKEY1_6,ADKEY1_7,ADKEY1_8
};
#endif //((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)

static void SET_AD_IO_OUT_1(void)
{
	PORTR_PU(PORTR2,0);
	PORTR_PD(PORTR2,0);
	PORTR_OUT(PORTR2,1);
	PORTR_DIR(PORTR2,0);
	PORTR_DIE(PORTR2,1);
}

static void SET_AD_IO(void)
{
	PORTR_PU(PORTR2,0);
	PORTR_PD(PORTR2,0);
	PORTR_OUT(PORTR2,0);
	PORTR_DIR(PORTR2,1);
	PORTR_DIE(PORTR2,1);
}
#endif //(ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)

volatile u8 adkey_lock_cnt = 0;

static void SET_ADKEY_LOCK_CNT(u8 cnt)
{
	CPU_SR_ALLOC(); 
	OS_ENTER_CRITICAL();

	adkey_lock_cnt = cnt;

	OS_EXIT_CRITICAL();
}

static u8 GET_ADKEY_LOCK_CNT(void)
{
	u8 val; 
	CPU_SR_ALLOC(); 
	OS_ENTER_CRITICAL();

	val = adkey_lock_cnt; 

	OS_EXIT_CRITICAL();
	return val;
}

static void POST_ADKEY_LOCK_CNT(void)
{
	CPU_SR_ALLOC(); 
	OS_ENTER_CRITICAL();

	adkey_lock_cnt --;

	OS_EXIT_CRITICAL();
}

/*----------------------------------------------------------------------------*/
/**@brief   ad按键初始化
   @param   void
   @param   void
   @return  void
   @note    void ad_key0_init(void)
*/
/*----------------------------------------------------------------------------*/
void ad_key0_init(void)
{
    s32 ret;
    key_puts("ad key init\n");

#if KEY_AD_EN

#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))
	//PR2 AD key init
	PORTR_PU(PORTR2,0);
	PORTR_PD(PORTR2,0);
	PORTR_DIR(PORTR2,1);
	PORTR_DIE(PORTR2,1);
	PORTR2_ADCEN_CTL(1);

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD)
	//PR1 AD key init
	PORTR_PU(PORTR1,1);
	PORTR_PD(PORTR1,0);
	PORTR_DIR(PORTR1,1);
	PORTR_DIE(PORTR1,0);
	PORTR1_ADCEN_CTL(1);
#endif //(ADC_PWR_SOURCE == ADC_PWR_RTCVDD)

#else          //ADC_PWR_VDDIO33

	//PR2 AD key init
	PORTR_PU(PORTR2,0);
	PORTR_PD(PORTR2,0);
	PORTR_DIR(PORTR2,1);
	PORTR_DIE(PORTR2,1);
	PORTR2_ADCEN_CTL(1);
#endif //((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

#endif //KEY_AD_EN

    adc_init_api(ad_table[0],clock_get_lsb_freq(),SYS_LVD_EN);

#if SYS_LVD_EN 

	ret = timer_reg_isr_fun(timer0_hl,5,(void *)battery_check,NULL);
	if(ret != TIMER_NO_ERR)
	{
		printf("battery_check err = %x\n",ret);
	}
#endif//SYS_LVD_EN

	ret = timer_reg_isr_fun(timer0_hl,1,adc_scan,NULL);
	if(ret != TIMER_NO_ERR)
	{
		printf("adc_scan err = %x\n",ret);
	}
}

void ad_key0_init_io()
{
#if KEY_AD_EN
	 
	if(ad_table[0] == AD_KEY_PR1)
	{
		/* printf("AD_KEY_PR1\n"); */
		/* PORTR_PU(PORTR1,0); */
		/* PORTR_PD(PORTR1,0); */
		/* PORTR_DIR(PORTR1,1); */
		/* PORTR_DIE(PORTR1,1); */
		PORTR1_ADCEN_CTL(1);  //开PR1 ADC 功能
	}
	else if(ad_table[0] == AD_KEY_PR2)
	{
		/* printf("AD_KEY_PR2\n"); */
		/* PORTR_PU(PORTR2,0); */
		/* PORTR_PD(PORTR2,0); */
		/* PORTR_DIR(PORTR2,1); */
		/* PORTR_DIE(PORTR2,1); */
		PORTR2_ADCEN_CTL(1);  //开PR2 ADC 功能

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD)
		/* PORTR_PU(PORTR1,1); */
		/* PORTR_PD(PORTR1,0); */
		/* PORTR_DIR(PORTR1,1); */
		/* PORTR_DIE(PORTR1,0); */
		PORTR1_ADCEN_CTL(1);
#endif //#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD)

	}

	SET_ADKEY_LOCK_CNT(3);

#endif //KEY_AD_EN
}

static int ad_key0_clk_reset(void)
{
    adc_init_api(ad_table[0],clock_get_lsb_freq(),SYS_LVD_EN);
    return 0;
}

static struct clock_switch clock_switch_key;

CLOCK_SWITCH_USER_REGISTER(CLOCK_SWITCH_EXIT, &clock_switch_key, ad_key0_clk_reset, "AD_KEY");


#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

extern u8 get_rtc_ldo_level(void);
extern void rtc_ldo_level(u8 level);

static u8 rtcvdd_cnt = 10;
static u8 rtcvdd_full_cnt = 0xff;
u16 rtcvdd_full_value = FULL_ADC;
u16 max_value = 0;
u16 min_value = 0xffff;
u32 total_value = 0;


/*把cnt个值里的最大值和最小值去掉，求剩余cnt-2个数的平均值*/
static u16 rtcvdd_full_vaule_update(u16 value)
{
#if KEY_AD_EN

	u16 full_value = FULL_ADC;
	if(rtcvdd_full_cnt == 0xff)
	{	
		/* printf("first time\n"); */
		rtcvdd_full_cnt = 0;
		SET_ADKEY_LOCK_CNT(50);
		return value;   //first time
	}
	else
	{
		rtcvdd_full_cnt ++; 
		if(value > max_value)
			max_value = value;
		if(value < min_value)
			min_value = value;
		total_value += value;

		/* printf("%d %x %x %x %x\n",rtcvdd_full_cnt , value , max_value , min_value , total_value); */
#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD) 
		if(rtcvdd_full_cnt > 10-1) //算10个数
#else
		if(rtcvdd_full_cnt > 6-1)  //算6个数
#endif
		{
			full_value = (total_value - max_value - min_value)/(rtcvdd_full_cnt - 2);
			/* printf("-----%x %x %x\n",full_value,AD_NOKEY(full_value),ADKEY1_0(full_value)); */
			rtcvdd_full_cnt = 0;
			max_value = 0;
			min_value = 0xffff;
			total_value = 0;
		}
		else
		{
			return rtcvdd_full_value;	
		}
	}
	return full_value;
#else
	return FULL_ADC;
#endif
}	

/*检测到RTCVDD 比 VDDIO 高的时候自动把RTCVDD降一档*/
static u8 rtcvdd_auto_match_vddio_lev(u32 rtcvdd_value)
{
	u8 rtcvdd_lev = 0;
	if(rtcvdd_value >= FULL_ADC)    //trim rtcvdd < vddio
	{
		if(rtcvdd_cnt > 10)
		{
			rtcvdd_cnt = 0;
			rtcvdd_lev = get_rtc_ldo_level();
			rtcvdd_lev++;
			if(rtcvdd_lev < 8)
			{
				printf("trim rtcvdd - %d\n",rtcvdd_lev);
				rtc_ldo_level(rtcvdd_lev);
				SET_ADKEY_LOCK_CNT(50);
				return 1;
			}
		}
		else
		{
			rtcvdd_cnt ++;
		}
	}
	else
	{
		rtcvdd_cnt = 0;

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD)
		rtcvdd_full_value = rtcvdd_full_vaule_update(rtcvdd_value);
#endif
	}
	return 0;
}

static u8 check_rtcvdd_cnt = 0;
/*检测到RTCVDD 比 VDDIO 低0.2V的时候自动把RTCVDD升一档*/
static void check_rtcvdd_reset(void)
{
#if KEY_AD_EN
    u32 vddio_value;
    u32 rtcvdd_value;

	vddio_value = 124*FULL_ADC/adc_value[AD_CH_LDOREF]; 

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)
	rtcvdd_value = 124*adc_value[AD_CH_KEY]/adc_value[AD_CH_LDOREF];
#else
	rtcvdd_value = 124*adc_value[AD_CH_RTCVDD]/adc_value[AD_CH_LDOREF];
#endif //(ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)

	if(vddio_value - rtcvdd_value > 20)
	{
		check_rtcvdd_cnt ++ ;
		if(check_rtcvdd_cnt > 100)
		{
			printf("rtcvdd +++++\n");
			check_rtcvdd_cnt = 0;
			rtc_ldo_level(get_rtc_ldo_level() - 1);
			SET_ADKEY_LOCK_CNT(50);
			printf("rtcvdd level : %d\n",get_rtc_ldo_level());
		}
	}
	else
	{
		check_rtcvdd_cnt = 0;
	}
#endif //KEY_AD_EN
}
#endif //#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

/*----------------------------------------------------------------------------*/
/**@brief   ad通道采样函数
   @param   void
   @param   void
   @return  void
   @note    void adc_scan(void)
*/
/*----------------------------------------------------------------------------*/

#define RTCVDD_SCAN_INTERVAL 5

u8 next_channel;
volatile u8 adc_busy_flag = 0;
void adc_scan(void *param)
{
	u16 rtcvdd_value;

	if(adc_busy_flag)
		return;

    static u8 channel = 0;
    static u8 key_channel = 0;

	next_channel = channel + 1;

	if(next_channel == MAX_AD_CHANNEL)
		next_channel = 0;

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE) && (KEY_AD_EN)

	//set AD_CH_KEY channel
	if(next_channel == AD_CH_KEY)
	{
		key_channel ++;
		if(key_channel == RTCVDD_SCAN_INTERVAL)
		{
			/* putchar('1'); */
			SET_AD_IO_OUT_1();	
		}
		else
		{
			/* putchar('2'); */
			SET_AD_IO();	
		}
	}

	//get AD_CH_KEY channel
	if(channel == AD_CH_KEY)
	{
		if(key_channel == RTCVDD_SCAN_INTERVAL)
		{
			key_channel = 0;
			channel++;
			rtcvdd_value =	adc_res_api(ad_table[next_channel]);
			rtcvdd_full_value = rtcvdd_full_vaule_update(rtcvdd_value);
		}
		else
		{
    		adc_value[channel++] = adc_res_api(ad_table[next_channel]);
		}
	}
	else
	{
    	adc_value[channel++] = adc_res_api(ad_table[next_channel]);
	}
#else
   	adc_value[channel++] = adc_res_api(ad_table[next_channel]);
#endif //(ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)

    if (channel == MAX_AD_CHANNEL)
        channel = 0;
}

u32 get_next_adc_ch(void)
{
	return ad_table[next_channel];
}

/*----------------------------------------------------------------------------*/
/**@brief   获取ad按键值
   @param   void
   @param   void
   @return  key_number
   @note    tu8 get_adkey_value(void)
*/
/*----------------------------------------------------------------------------*/
#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

tu8 get_adkey_value(void)
{
#if KEY_AD_EN
    tu8 key_number;
    u32 key_value;
	u16 rtcvdd_value;

#if (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE)
	rtcvdd_value = adc_value[AD_CH_KEY];
#else
	rtcvdd_value = adc_value[AD_CH_RTCVDD];
#endif

	/* printf("rtcvdd_value:%x\n",rtcvdd_value); */

	if(rtcvdd_auto_match_vddio_lev(rtcvdd_value))
		return NO_KEY;

    key_value = adc_value[AD_CH_KEY];

	if(GET_ADKEY_LOCK_CNT())
	{
		POST_ADKEY_LOCK_CNT();
		return NO_KEY;
	}

    if (key_value > AD_NOKEY(rtcvdd_full_value))
	{
		check_rtcvdd_reset();
		return NO_KEY;
	}

	set_rtcvdd_table(rtcvdd_full_value);

    for (key_number = 0; key_number < sizeof (ad_rtcvdd_key_table) / sizeof (ad_rtcvdd_key_table[0]); key_number++)
	{	
		if (key_value > ad_rtcvdd_key_table[key_number])
			break;
	}

	/* printf("adkey_value:%x   key_num:0x%x\n",key_value,key_number); */

	return key_number;
#else
    return 0xff;
#endif
}	

#else          //ADC_PWR_VDDIO33

tu8 get_adkey_value(void)
{
#if KEY_AD_EN
    tu8 key_number;
    u32 key_value;

    key_value = adc_value[AD_CH_KEY];
    if (key_value > AD_NOKEY)
		return NO_KEY;

    for (key_number = 0; key_number < sizeof (ad_key_table) / sizeof (ad_key_table[0]); key_number++)
	{	
		if (key_value > ad_key_table[key_number])
		break;
	}
	/* printf("adkey_value:%d   key_num:0x%x\n",key_value*33/0x3ff,key_number); */
	return key_number;
#else
    return 0xff;
#endif
}	

#endif //#if ((ADC_PWR_SOURCE == ADC_PWR_RTCVDD) || (ADC_PWR_SOURCE == ADC_PWR_RTCVDD_SIMPLE))

/*----------------------------------------------------------------------------*/
/**@brief   获取ad值
   @param   void
   @return  ad value 1.8v return 180 
   @note    u32 get_ad_value(void)
*/
/*----------------------------------------------------------------------------*/
u32 get_ad_value(void)
{
#if KEY_AD_EN
	u32 value;
	u32 LDO_ref;
    u32 ad_value;

    value = adc_value[AD_CH_KEY];
	LDO_ref  = adc_value[AD_CH_LDOREF];
	ad_value = 124*value/LDO_ref;

    return ad_value;
#else
    return 0;
#endif
}

/*----------------------------------------------------------------------------*/
/**@brief   获取电池电量
   @param   void
   @param   void
   @return  电池电量值
   @note    tu8 get_battery_level(void)
*/
/*----------------------------------------------------------------------------*/
u16 get_battery_level(void)
{
    u16 battery_value,LDOIN_12,LDO_ref;
    LDOIN_12 = adc_value[AD_CH_LDOIN];
    LDO_ref  = adc_value[AD_CH_LDOREF];//0x181,1.2v

	battery_value = (((u16)LDOIN_12*372)/10)/((u16)LDO_ref); //针对AC69 
	/* battery_value = (((u16)LDOIN_12*368)/10)/((u16)LDO_ref); */
	/* printf("battery_value:0x%x    0x%x   %d\n",LDOIN_12,LDO_ref,battery_value); */
    return battery_value;

}
