#ifndef _LED_H_
#define _LED_H_

#include "includes.h"
#include "sys_detect.h"
#include "sdk_cfg.h"

// #define USE_USB_IO_PWM     //用usb dm dp推灯时用此宏，主要针对6908的封装

#ifndef USE_USB_IO_PWM

#define R_LED_IO_BIT      BIT(3)  ////PA3;
#define B_LED_IO_BIT      BIT(4)  ////PA4;

#define LED_INIT_EN()     do{JL_PWM4->CON = 0;JL_IOMAP->CON1 |= BIT(11)|BIT(12)|BIT(13);}while(0)
#define LED_INIT_DIS()    do{JL_PWM4->CON = 0;JL_IOMAP->CON1 &= ~(BIT(11)|BIT(12)|BIT(13));}while(0)

#define R_LED_ON()        do{JL_PWM4->CON = BIT(1)|BIT(0); JL_PORTA->DIE |= R_LED_IO_BIT;JL_PORTA->DIR &=~ R_LED_IO_BIT;\
								 JL_PORTA->PU |= R_LED_IO_BIT;JL_PORTA->PD |= R_LED_IO_BIT;}while(0)
#define R_LED_OFF()       do{JL_PORTA->DIR |= R_LED_IO_BIT;JL_PORTA->PU &=~ R_LED_IO_BIT;JL_PORTA->PD &=~ R_LED_IO_BIT;}while(0)

#define B_LED_ON()        do{JL_PWM4->CON = BIT(3);JL_PORTA->DIE |= B_LED_IO_BIT;JL_PORTA->DIR &=~ B_LED_IO_BIT;\
								 JL_PORTA->PU |= B_LED_IO_BIT;JL_PORTA->PD |= B_LED_IO_BIT;}while(0)
#define B_LED_OFF()       do{JL_PORTA->DIR |= B_LED_IO_BIT;JL_PORTA->PU &=~ B_LED_IO_BIT;JL_PORTA->PD &=~ B_LED_IO_BIT;}while(0)

#else

void led_pwm_init();
void led_pwm_close();

#define LED_INIT_EN()     do{led_pwm_init();}while(0)
#define LED_INIT_DIS()    do{led_pwm_close();}while(0)

#define R_LED_ON()        do{USB_DP_PU(1);USB_DP_PD(1);USB_DP_DIR(0);}while(0)
#define R_LED_OFF()       do{USB_DP_PU(0);USB_DP_PD(0);USB_DP_DIR(1);}while(0)

#define B_LED_ON()        do{USB_DM_PU(1);USB_DM_PD(1);USB_DM_DIR(0);}while(0)
#define B_LED_OFF()       do{USB_DM_PU(0);USB_DM_PD(0);USB_DM_DIR(1);}while(0)

#endif  //USE_USB_IO_PWM

extern u32 g_led_fre;

void led_init(void);
void led_close(void);
void led_open(void);
void led_fre_set(u32 fre,u8 led_flash);
void led_scan(void *param);

#endif/*_LED_H_*/

