/*--------------------------------------------------------------------------*/
/**@file    LED.c
   @brief   LED 模块驱动接口函数
   @details
   @author  bingquan Cai
   @date    2012-8-30
   @note    AC319N
*/
/*----------------------------------------------------------------------------*/
#include "ui/ui_api.h"

#if UI_ENABLE

#include "ui/ui_common.h"
#include "ui/led/led7_drv.h"
#include "fm_radio.h"
#include "dec/decoder_phy.h"
#include "music_ui.h"
#include "music.h"
#include "timer.h"
#include "key_drv/key.h"
#include "file_operate/file_op.h"
//#include "play_file.h"
#include "dac/dac.h"
#include "rtc_setting.h"

#if REC_EN
    #include "encode/encode.h"
#endif

LED7_VAR led7_var;

const  u8 LED_NUMBER[10] =
{
    /*0*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F),
    /*1*/
    (u8)(LED_B | LED_C),
    /*2*/
    (u8)(LED_A | LED_B | LED_D | LED_E | LED_G),
    /*3*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_G),
    /*4*/
    (u8)(LED_B | LED_C | LED_F | LED_G),
    /*5*/
    (u8)(LED_A | LED_C | LED_D | LED_F | LED_G),
    /*6*/
    (u8)(LED_A | LED_C | LED_D | LED_E | LED_F | LED_G),
    /*7*/
    (u8)(LED_A | LED_B | LED_C),
    /*8*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_E | LED_F | LED_G),
    /*9*/
    (u8)(LED_A | LED_B | LED_C | LED_D | LED_F | LED_G),
};

const  u8 LED_LARGE_LETTER[26] =
{
    0x77,0x40,0x39,0x3f,0x79,///<ABCDE
    0x71,0x40,0x76,0x06,0x40,///<FGHIJ
    0x40,0x38,0x40,0x37,0x3f,///<KLMNO
    0x73,0x40,0x50,0x6d,0x78,///<PQRST
    0x3e,0x3e,0x40,0x76,0x40,///<UVWXY
    0x40///<Z
};

const  u8 LED_SMALL_LETTER[26] =
{
    0x77,0x7c,0x58,0x5e,0x79,///<abcde
    0x71,0x40,0x40,0x40,0x40,///<fghij
    0x40,0x38,0x40,0x54,0x5c,///<klmno
    0x73,0x67,0x50,0x40,0x78,///<pqrst
    0x3e,0x3e,0x40,0x40,0x40,///<uvwxy
    0x40///<z
};

/* 通过测试 7断的真值表
   0    1    2     3     4     5     6
0  X    1A   1B    1E    SD   播放   X
1  1F   X    2A    2B    2E   2D     X
2  1G   2F   X     :     3B   ||     MP3
3  1C   2G   3F    X     3C   4E     X
4  1D   2C   3G    3A    X    4C     4G
5  3D   U    3E    4D    4F   X      4B
6  X    X    MHz    X    X    4A     X
*/
/*
// 7断数码管转换表
const u8 led_7[35][2] =
{
  {0,1},//1A
  {0,2},//1B
  {3,0},//1C
  {4,0},//1D
  {0,3},//1E
  {1,0},//1F
  {2,0},//1G

  {1,2},//2A
  {1,3},//2B
  {4,1},//2C
  {1,5},//2D
  {1,4},//2E
  {2,1},//2F
  {3,1},//2G

  {4,3},//3A
  {2,4},//3B
  {3,4},//3C
  {5,0},//3D
  {5,2},//3E
  {3,2},//3F
  {4,2},//3G

  {6,5},//4A
  {5,6},//4B
  {4,5},//4C
  {5,3},//4D
  {3,5},//4E
  {5,4},//4F
  {4,6},//4G

  {0,5},//LED_PLAY
  {2,5},//LED_PAUSE
  {5,1},//LED_USB
  {0,4},//LED_SD
  {2,3},//LED_2POINT
  {6,2},//LED_MHZ
  {2,6},//LED_MP3
};
*/
/***********新7段数码管真值表**************/
/* 0    1     2     3     4     5     6
0  X    2A    2B    2C    2D    2E    2F
1  1A   X     2G    :     .     MHz   MP3
2  1B   X     X     4A    4B    4C    4D
3  1C   PLAY  3A    X     4E    4F    4G
4  1D   PAUSE 3B    3E    X     X     X
5  1E   USB   3C    3F    X     X     X
6  1F   SD    3D    3G    X     X     X
*/
const u8 led_7[36][2] = ///< 新7断数码管转换表
{
  {1,0},//1A
  {2,0},//1B
  {3,0},//1C
  {4,0},//1D
  {5,0},//1E
  {6,0},//1F
  {2,1},//1G

  {0,1},//2A
  {0,2},//2B
  {0,3},//2C
  {0,4},//2D
  {0,5},//2E
  {0,6},//2F
  {1,2},//2G

  {3,2},//3A
  {4,2},//3B
  {5,2},//3C
  {6,2},//3D
  {4,3},//3E
  {5,3},//3F
  {6,3},//3G

  {2,3},//4A
  {2,4},//4B
  {2,5},//4C
  {2,6},//4D
  {3,4},//4E
  {3,5},//4F
  {3,6},//4G

  {3,1},//LED_PLAY
  {4,1},//LED_PAUSE
  {5,1},//LED_USB
  {6,1},//SD
  {1,3},//:
  {1,5},//MHz
  {1,4},//.
  {1,6},//MP3
};

//按位于 查表
const u8 bit_table[8] =
{
  0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,
};

const u8 playmodestr[][5] =
{
    " ALL",
    "Fold",
    " ONE",
    " rAn",
};

const u8 menu_string[][5] =
{
    " HI ",
    "Lod ",
    "bLUE",
    " PC ",
    " UP ",
    " dN ",
    " AUX",
    "-AL-"
};
const u8 other_string[][5]=
{
    "Eq ",
    "V ",
    "P ",
    " NOP",
    " rec",
};

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 设备显示
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_dev(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_dev(u8 dev)
{
//    /*Music Device type*/
    /* MUSIC_DIS_VAR *music_var; */
    /* music_var = (MUSIC_DIS_VAR*)UI_var.ui_buf_adr; */

    /* if(music_var) */
    {
        if((dev == SD0_DEVICE)||(dev == SD1_DEVICE))
        {
            LED_STATUS |= LED_SD;
        }
        else if(dev == USB_DEVICE)
        {
            LED_STATUS |= LED_USB;
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 状态位缓存清除函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_clear_icon(void)
*/
/*----------------------------------------------------------------------------*/
void led7_clear_icon(void)
{
    led7_var.bFlashChar = 0;
    led7_var.bFlashIcon = 0;
    led7_var.bShowBuff[4] = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 显示坐标设置
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led7_setX(u8 X)
*/
/*----------------------------------------------------------------------------*/
void led7_setX(u8 X)
{
    led7_var.bCoordinateX = X;
}

/*----------------------------------------------------------------------------*/
/**@brief   LED 清屏函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_null(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_null(void)
{
    led7_clear_icon();
    led7_var.bShowBuff[0] = 0;
    led7_var.bShowBuff[1] = 0;
    led7_var.bShowBuff[2] = 0;
    led7_var.bShowBuff[3] = 0;
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_init(void)
*/
/*----------------------------------------------------------------------------*/
void led7_init(void)
{
    led7_clear();
    s32 ret;
	ret = timer_reg_isr_fun(timer0_hl,1,led7_scan,NULL);
	if(ret != TIMER_NO_ERR)
	{
		printf("led7_scan err = %x\n",ret);
	}
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 单个字符显示函数
   @param   chardata：显示字符
   @return  void
   @author  Change.tsai
   @note    void led7_show_char(u8 chardata)
*/
/*----------------------------------------------------------------------------*/
void led7_show_char(u8 chardata)
{
    //必须保证传入的参数符合范围，程序不作判断
    //if ((chardata < ' ') || (chardata > '~') || (led7_var.bCoordinateX > 4))
    //{
    //    return;
    //}
    if ((chardata >= '0') && (chardata <= '9'))
    {
        led7_var.bShowBuff[led7_var.bCoordinateX++] = LED_NUMBER[chardata - '0'];
    }
    else if ((chardata >= 'a') && (chardata <= 'z'))
    {
        led7_var.bShowBuff[led7_var.bCoordinateX++] = LED_SMALL_LETTER[chardata - 'a'];
    }
    else if ((chardata >= 'A') && (chardata <= 'Z'))
    {
        led7_var.bShowBuff[led7_var.bCoordinateX++] = LED_LARGE_LETTER[chardata - 'A'];
    }
    else if (chardata == ':')
    {
        LED_STATUS |= LED_2POINT;
    }
    else if (chardata == ' ')
    {
        led7_var.bShowBuff[led7_var.bCoordinateX++] = 0;
    }
    else //if (chardata == '-')     //不可显示
    {
        led7_var.bShowBuff[led7_var.bCoordinateX++] = BIT(6);
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   led7_drv 字符串显示函数
   @param   *str：字符串的指针   offset：显示偏移量
   @return  void
   @author  Change.tsai
   @note    void led7_show_string(u8 *str)
*/
/*----------------------------------------------------------------------------*/
void led7_show_string(u8 *str)
{
    while (0 != *str)
    {
        led7_show_char(*str++);
    }
}

/*----------------------------------------------------------------------------*/
/** @brief:
    @param:
    @return:
    @author:
    @note:
*/
/*----------------------------------------------------------------------------*/
void led7_show_string_menu(u8 menu)
{

    if(menu >= (sizeof(menu_string)/5))
    {
        printf("*strid(%d) is over!\n",menu);
    }
    else
    {
        led7_show_string((u8 *)menu_string[menu]);
    }
}

void led7_show_linin_main(u8 menu)
{
    led7_show_string_menu(menu);

#if REC_EN

    RECORD_OP_API * rec_var_p;
    REC_CTL * rec_ctl_var;

    rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
    if((UI_var.ui_buf_adr)&&(rec_var_p))
    {
        rec_ctl_var = rec_var_p->rec_ctl;

        LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
        LED_STATUS &= ~(LED_SD|LED_USB);

        if((menu == MENU_AUX_MAIN)||(menu == MENU_BT_MAIN))
        {
            if((rec_ctl_var)&&(ENC_STOP !=rec_ctl_var->enable))
            {
                if (ENC_STAR == rec_ctl_var->enable)
                    LED_STATUS |= LED_PLAY;
                else if(ENC_PAUS == rec_ctl_var->enable)
                    LED_STATUS |= LED_PAUSE;

				led7_show_dev(rec_ctl_var->curr_device);
            }
        }
    }
#endif
}
/*----------------------------------------------------------------------------*/
/**@brief   Music 播放文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_filenumber(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_filenumber(void)
{
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR*)UI_var.ui_buf_adr;

    if(music_var)
    {
        /*Music File Number info*/
        //printf("ui_music.ui_play_file_num = %u----\n",ui_music.ui_play_file_num);
        itoa4(music_var->ui_curr_file);
        led7_show_string((u8 *)bcd_number);
    }
}


/*----------------------------------------------------------------------------*/
/**@brief   红外输入文件号显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_IR_number(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_IR_number(s32 arg)
{
    u16 ir_num;
    ir_num = (u16)(arg&0xffff);
    /*IR File Number info*/
    itoa4(ir_num);
    led7_show_string((u8 *)bcd_number);
}

/*----------------------------------------------------------------------------*/
/**@brief   Music模式 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_music_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_music_main(void)
{
    //led7_show_string((u8*)"MUSI");
    u32 play_time;
    MUSIC_DIS_VAR *music_var;

    music_var = (MUSIC_DIS_VAR*)UI_var.ui_buf_adr;

    if(music_var)
    {
        /*Music Play time info*/
        play_time = music_var->play_time;

        itoa2(play_time/60);
        led7_show_string((u8 *)bcd_number);

        itoa2(play_time%60);
        led7_show_string((u8 *)bcd_number);

        /* led7_show_dev(); */
		led7_show_dev(music_var->ui_curr_device);

        LED_STATUS |= LED_2POINT | LED_MP3;

        if (*music_var->ui_curr_statu == DECODER_PLAY)
        {
            LED_STATUS |= LED_PLAY;
            LED_STATUS &= ~LED_PAUSE;
            led7_var.bFlashIcon |= LED_2POINT;
        }
        else if (*music_var->ui_curr_statu == DECODER_PAUSE)
        {
            LED_STATUS &= ~LED_PLAY;
            LED_STATUS |= LED_PAUSE;
        }
    //
    //    if(mute)
    //    {
    //        led7_var.bFlashIcon |= LED_MP3;
    //        led7_var.bFlashChar = 0x0f;
    //    }
    //    else
    //    {
    //        led7_var.bFlashIcon &= ~LED_MP3;
    //        led7_var.bFlashChar = 0x00;
    //    }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   EQ显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_eq(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led7_show_eq(s32 arg)
{
    u8 eq_cnt;
    eq_cnt = (u8)arg;
    led7_show_string((u8 *)other_string[0]);
    led7_show_char(eq_cnt%10 + '0');
}

/*----------------------------------------------------------------------------*/
/**@brief   循环模式显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_playmode(s32 arg)
*/
/*----------------------------------------------------------------------------*/
void led7_show_playmode(s32 arg)
{
    u8 pm_cnt;
    pm_cnt = (u8)arg;
    led7_show_string((u8 *)&playmodestr[pm_cnt-FOP_MAX-1][0]);
}

/*----------------------------------------------------------------------------*/
/**@brief   音量显示函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_volume(s32 vol)
*/
/*----------------------------------------------------------------------------*/
void led7_show_volume(s32 vol)
{
    u8 tmp_vol;

    tmp_vol = (u8)vol;
    led7_show_string((u8 *)other_string[1]);
    itoa2(tmp_vol);
    led7_show_string((u8 *)bcd_number);
}

/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_fm_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_fm_main(void)
{
    /*FM - Frequency*/
 #if 1
    FM_MODE_VAR * fm_var;

    if(!UI_var.ui_buf_adr)
        return;

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if(fm_var)
    {
        itoa4(fm_var->wFreq);

        if (fm_var->wFreq <= 999)
            bcd_number[0] = ' ';

        led7_show_string((u8 *)bcd_number);
        LED_STATUS |= LED_MHZ;

#if REC_EN
        RECORD_OP_API * rec_var_p;
        REC_CTL * rec_ctl_var;

        if((fm_var->fm_rec_op)&&(*(RECORD_OP_API **)(fm_var->fm_rec_op)))
        {
            rec_var_p = *(RECORD_OP_API **)fm_var->fm_rec_op;
            rec_ctl_var = rec_var_p->rec_ctl;

            if((rec_ctl_var)&&(ENC_STOP != rec_ctl_var->enable))
            {
				LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
				LED_STATUS &= ~(LED_SD|LED_USB);

                /* led7_show_dev(); */
				led7_show_dev(rec_ctl_var->curr_device);
                if (ENC_STAR == rec_ctl_var->enable)
                    LED_STATUS |= LED_PLAY;
                else if(ENC_PAUS == rec_ctl_var->enable)
                    LED_STATUS |= LED_PAUSE;
            }

        }
#endif
    }

#endif // 0
}

/*----------------------------------------------------------------------------*/
/**@brief   FM 模式主界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_fm_station(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_fm_station(void)
{
//    /*FM - Station*/
#if 1
    FM_MODE_VAR * fm_var;

    if(!UI_var.ui_buf_adr)
        return;

    fm_var = *(FM_MODE_VAR **)UI_var.ui_buf_adr;

    if(fm_var)
    {
        led7_show_string((u8 *)other_string[2]);
        itoa2(fm_var->wFreChannel);
        led7_show_string((u8 *)bcd_number);
    }
#endif
}

#if 1 //RTC_CLK_EN
/*----------------------------------------------------------------------------*/
/**@brief   RTC 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_RTC_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_RTC_main(void)
{
    RTC_SETTING * rtc_var;
    RTC_TIME *ui_time;

    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if(rtc_var)
    {
        ui_time = rtc_var->calendar_set.curr_rtc_time;

        itoa2(ui_time->bHour);
        led7_show_string((u8 *)bcd_number);
        itoa2(ui_time->bMin);
        led7_show_string((u8 *)bcd_number);

        led7_var.bFlashIcon |= LED_2POINT;
        LED_STATUS |= LED_2POINT;

        if(rtc_var->rtc_set_mode == RTC_SET_MODE)
        {
            if(rtc_var->calendar_set.coordinate == COORDINATE_MIN)
                led7_var.bFlashChar = BIT(0)|BIT(1);
            else
                led7_var.bFlashChar = BIT(2)|BIT(3);
        }
        else
            led7_var.bFlashChar = 0;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   Alarm 显示界面
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_show_alarm(void)
*/
/*----------------------------------------------------------------------------*/
#if 1 //RTC_ALM_EN
void led7_show_alarm(void)
{
    RTC_SETTING * rtc_var;
    RTC_TIME *ui_time;
    rtc_var = (RTC_SETTING *)UI_var.ui_buf_adr;

    if(rtc_var)
    {
        puts("led7_show_alarm\n");

        ui_time = rtc_var->alarm_set.curr_alm_time;
        itoa2(ui_time->bHour);
        led7_show_string((u8 *)bcd_number);
        itoa2(ui_time->bMin);
        led7_show_string((u8 *)bcd_number);

        LED_STATUS |= LED_2POINT;

        if(rtc_var->rtc_set_mode == ALM_SET_MODE)
        {
            if(rtc_var->alarm_set.coordinate == COORDINATE_MIN)
                led7_var.bFlashChar = BIT(0)|BIT(1);
            else
                led7_var.bFlashChar = BIT(2)|BIT(3);
        }

        /*Alarm info - Switch On/Off*/
        if (rtc_var->alarm_set.alarm_sw)
        {
            LED_STATUS |= LED_PLAY;
        }
        else
        {
            LED_STATUS |= LED_PAUSE;
        }
    }
}
#endif
#endif

#if REC_EN
/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led7_show_rec_start(void)
*/
/*----------------------------------------------------------------------------*/
static void led7_show_rec_start(REC_CTL * rec_ctl_var)
{
    u32 rec_time;

//    LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
//    if(rec_ctl_var)
    {
        rec_time = rec_ctl_var->file_info.enc_time_cnt;

        itoa2(rec_time/60);
        led7_show_string((u8 *)bcd_number);

        itoa2(rec_time%60);
        led7_show_string((u8 *)bcd_number);

        /* led7_show_dev(); */
		led7_show_dev(rec_ctl_var->curr_device);

        LED_STATUS |= LED_2POINT; //| LED_PLAY;

        if (ENC_STAR == rec_ctl_var->enable)
            LED_STATUS |= LED_PLAY;
        else if (ENC_PAUS == rec_ctl_var->enable)
            LED_STATUS |= LED_PAUSE;
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   REC 显示界面
   @param   void
   @return  void
   @note    void led7_show_rec_main(void)
*/
/*----------------------------------------------------------------------------*/
void led7_show_rec_main(void)
{
    RECORD_OP_API * rec_var_p;
    REC_CTL * rec_ctl_var;

    LED_STATUS &= ~(LED_PLAY | LED_PAUSE);
    LED_STATUS &= ~LED_2POINT; //| LED_PLAY;
    LED_STATUS &= ~(LED_SD|LED_USB);

    if(UI_var.ui_buf_adr)
    {
        rec_var_p = *(RECORD_OP_API **)UI_var.ui_buf_adr;
        if(rec_var_p)
        {
            rec_ctl_var = rec_var_p->rec_ctl;
            if((rec_ctl_var)&&(ENC_STOP != rec_ctl_var->enable))
            {
                led7_show_rec_start(rec_ctl_var);
                return;
            }
        }
    }

    led7_show_string((u8 *)other_string[4]);
}
#endif

void LED_drive7(void)
{
    u8 k,i,j,temp;
    k=0;

    //led7_var.bShowBuff[0]=0xff;
    //led7_var.bShowBuff[1]=0xff;
    //led7_var.bShowBuff[2]=0xff;
    //led7_var.bShowBuff[3]=0xff;
    //led7_var.bShowBuff[4]=0xff;

    led7_var.bShowBuff1[0]=0;
    led7_var.bShowBuff1[1]=0;
    led7_var.bShowBuff1[2]=0;
    led7_var.bShowBuff1[3]=0;
    led7_var.bShowBuff1[4]=0;
    led7_var.bShowBuff1[5]=0;
    led7_var.bShowBuff1[6]=0;

    for(i=0;i<5;i++)
    {
        temp = led7_var.bShowBuff[i];
        if (get_sys_halfsec())
        {
            if ((led7_var.bFlashIcon) && (i == 4))
            {
                temp = LED_STATUS & (~led7_var.bFlashIcon);
            }
            else if (led7_var.bFlashChar & BIT(i))
            {
                temp = 0x0;
            }
        }

        for(j=0;j<7;j++)
        {
            if(temp & bit_table[j])
            {
                led7_var.bShowBuff1[led_7[k][0]] |= bit_table[led_7[k][1]];
            }
            k++;
        }
        if(j == 7)
        {
            if(temp & bit_table[j])
                led7_var.bShowBuff1[led_7[35][0]] |= bit_table[led_7[35][1]];
        }
    }
}

/*----------------------------------------------------------------------------*/
/**@brief   LED清屏函数
   @param   x：显示横坐标
   @return  void
   @author  Change.tsai
   @note    void led7_clear(void)
*/
/*----------------------------------------------------------------------------*/
void led7_clear(void)
{
    LEDN_PORT_OUT &= ~(BIT(LEDN_S0_BIT)|BIT(LEDN_S1_BIT)|BIT(LEDN_S2_BIT)|BIT(LEDN_S3_BIT)|BIT(LEDN_S4_BIT)|BIT(LEDN_S5_BIT)|BIT(LEDN_S6_BIT));
    LEDN_PORT_DIR |=  (BIT(LEDN_S0_BIT)|BIT(LEDN_S1_BIT)|BIT(LEDN_S2_BIT)|BIT(LEDN_S3_BIT)|BIT(LEDN_S4_BIT)|BIT(LEDN_S5_BIT)|BIT(LEDN_S6_BIT));
    LEDN_PORT_PU  &= ~(BIT(LEDN_S0_BIT)|BIT(LEDN_S1_BIT)|BIT(LEDN_S2_BIT)|BIT(LEDN_S3_BIT)|BIT(LEDN_S4_BIT)|BIT(LEDN_S5_BIT)|BIT(LEDN_S6_BIT));
    LEDN_PORT_PD  &= ~(BIT(LEDN_S0_BIT)|BIT(LEDN_S1_BIT)|BIT(LEDN_S2_BIT)|BIT(LEDN_S3_BIT)|BIT(LEDN_S4_BIT)|BIT(LEDN_S5_BIT)|BIT(LEDN_S6_BIT));
    LEDN_PORT_HD  &= ~(BIT(LEDN_S0_BIT)|BIT(LEDN_S1_BIT)|BIT(LEDN_S2_BIT)|BIT(LEDN_S3_BIT)|BIT(LEDN_S4_BIT)|BIT(LEDN_S5_BIT)|BIT(LEDN_S6_BIT));

}

/*----------------------------------------------------------------------------*/
/**@brief   LED扫描函数
   @param   void
   @return  void
   @author  Change.tsai
   @note    void led7_scan(void *param)
*/
/*----------------------------------------------------------------------------*/
void led7_scan(void *param)
{
    static u8 cnt;
    u8 seg;

    LED_drive7();

    seg = led7_var.bShowBuff1[cnt];

    led7_clear();

    switch(cnt)
    {
    case 0:
        LEDN_PORT_OUT |=  BIT(LEDN_S0_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S0_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S0_BIT);
        break;

    case 1:
        LEDN_PORT_OUT |=  BIT(LEDN_S1_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S1_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S1_BIT);
        break;

    case 2:
        LEDN_PORT_OUT |=  BIT(LEDN_S2_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S2_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S2_BIT);
        break;

    case 3:
        LEDN_PORT_OUT |=  BIT(LEDN_S3_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S3_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S3_BIT);
        break;

    case 4:
        LEDN_PORT_OUT |=  BIT(LEDN_S4_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S4_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S4_BIT);
        break;

    case 5:
        LEDN_PORT_OUT |=  BIT(LEDN_S5_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S5_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S5_BIT);
        break;

    case 6:
        LEDN_PORT_OUT |=  BIT(LEDN_S6_BIT);
        LEDN_PORT_HD  |=  BIT(LEDN_S6_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S6_BIT);
        break;

    default :
        break;
    }

    if(seg & BIT(0))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S0_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S0_BIT);
    }

    if(seg & BIT(1))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S1_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S1_BIT);
    }

    if(seg & BIT(2))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S2_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S2_BIT);
    }

    if(seg & BIT(3))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S3_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S3_BIT);
    }

    if(seg & BIT(4))
    {
       LEDN_PORT_OUT &= ~BIT(LEDN_S4_BIT);
       LEDN_PORT_DIR &= ~BIT(LEDN_S4_BIT);
    }

    if(seg & BIT(5))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S5_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S5_BIT);
    }

    if(seg & BIT(6))
    {
        LEDN_PORT_OUT &= ~BIT(LEDN_S6_BIT);
        LEDN_PORT_DIR &= ~BIT(LEDN_S6_BIT);
    }
    cnt = (cnt >= 7) ? 0 : cnt+1;
}
#endif
