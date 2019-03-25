#ifndef	_LED7_DRV_H_
#define _LED7_DRV_H_

#include "includes.h"

typedef struct _led7_VAR
{
    u8  bCoordinateX;
    u8  bFlashChar;
    u8  bFlashIcon;
    u8  bShowBuff[5];
    u8  bBrightness;
    u8  bShowBuff1[9];
}LED7_VAR;


//void set_LED_fade_out(void);
//void set_LED_all_on(void);

void led7_init(void);
void led7_scan(void *param);
void led7_clear_icon(void);
void led7_show_char(u8 chardata);
void led7_show_number(u8 number);
void led7_show_Hi(void);
void led7_show_music_main(void);
void led7_show_RTC_main(void);
void led7_show_filenumber(void);
void led7_show_volume(s32 vol);
void led7_show_fm_main(void);
void led7_show_IR_number(s32);
void led7_show_pc_main(void);
void led7_show_pc_vol_up(void);
void led7_show_pc_vol_down(void);
void led7_show_aux_main(void);
void led7_show_eq(s32 arg);
void led7_show_playmode(s32 arg);
void led7_show_pause(void);
void led7_show_fm_station(void);
void led7_show_waiting(void);
void led7_show_alarm(void);
//void led7_show_nop(void);
void led7_show_rec_main(void);
void led7_show_linin_main(u8 menu);
void led7_clear(void);
void led7_show_string_menu(u8 menu);
void led7_setX(u8 X);
#define LED_STATUS  led7_var.bShowBuff[4]

#define LED_A   BIT(0)
#define LED_B	BIT(1)
#define LED_C	BIT(2)
#define LED_D	BIT(3)
#define LED_E	BIT(4)
#define LED_F	BIT(5)
#define LED_G	BIT(6)
#define LED_H	BIT(7)

//for LED0
#define LED_PLAY	LED_A
#define LED_PAUSE	LED_B
#define LED_USB		LED_C
#define LED_SD		LED_D
#define LED_2POINT	LED_E
#define LED_MHZ	    LED_F
#define LED_DOT		LED_G
#define LED_MP3     LED_H

//Õ¼ÓÃPB3~PB9
#define  LEDN_PORT_OUT     JL_PORTB->OUT
#define  LEDN_PORT_HD      JL_PORTB->HD
#define  LEDN_PORT_DIR     JL_PORTB->DIR
#define  LEDN_PORT_PD      JL_PORTB->PD
#define  LEDN_PORT_PU      JL_PORTB->PU

#define  LEDN_S0_BIT   3 
#define  LEDN_S1_BIT   4
#define  LEDN_S2_BIT   5
#define  LEDN_S3_BIT   6
#define  LEDN_S4_BIT   7
#define  LEDN_S5_BIT   8
#define  LEDN_S6_BIT   9

#endif	/*	_LED_H_	*/
