#include "charge.h"
#include "sdk_cfg.h"
#include "rtc/rtc_api.h"
#include "led.h"
#include "sys_detect.h"
#include "dac.h"
#include "key_drv/key.h"
#include "key_drv/key_drv_io.h"
#include "key_drv/key_drv_ad.h"
#include "clock.h"
#include "timer.h"
#include "adc_api.h"
#include "irq_api.h"
#include "clock_api.h"

#define CHARGEE_DBG
#ifdef  CHARGEE_DBG
#define charge_putchar        putchar
#define charge_printf         printf
#define charge_buf            printf_buf
#else
#define charge_putchar(...)
#define charge_printf(...)
#define charge_buf(...)
#endif    //CHARGEE_DBG


enum {
    STEP_GET_POINT,
    STEP_GET_LINE,
};

//可配置选项
//充电时间过长，可以调整BUILD_POINT_THRESHOLD_VALUE宏，调试步长为1，值越大，充电时间越短，充满电压越低
#define BUILD_POINT_THRESHOLD_VALUE   3            // 阀值，增大此值可以缩短充电时间,调试时可以用步进为1调试
#define BUILD_POINT_CNT               60 * 500     // n * 2ms
#define CHARGE_MAX_CNT                80 * 60 * 2  // n * 500ms

#define C_POWER_BAT_CHECK_CNT         50
#define C_POWER_KEY_CHECK_CNT         800

#define BUILD_POINT_INIT_CNT          10*500    // n * 2ms
#define ADC_OVERSAMPLING_ACCURACY     6             // (10 +  ADC_OVERSAMPLING_ACCURACY) bits

static void charge_deal(u16 vbat);
extern void set_sys_ldo_level(u8 level);

struct charge_var {
    u8 step;
    u16 point_cnt;
    u32 point_total;
    u16 point_average;
    u16 last_bat_value;
    u8 send_msg_flag;
    u32 charge_cnt;
};

#if CHARGE_PROTECT_EN
static struct charge_var charge;
#define __this (&charge)

__timer_handle  *charge_timer0_hl;

static u8 charge_t0_cnt = 0;
static u32 power_off_cnt = 0;

static u16 time_cnt = 0;
static u16 max_value = 0;
static u16 min_value = 0xffff;
#endif

static void charge_var_init(void)
{
#if CHARGE_PROTECT_EN
    __this->step = STEP_GET_POINT;
    __this->point_cnt = 0;
    __this->point_total = 0;
    __this->point_average = 0;
    __this->last_bat_value = 0;
    __this->send_msg_flag = 0;
    __this->charge_cnt = 0;
#endif
}

u32 get_ldo5v_online_flag(void)
{
#if CHARGE_PROTECT_EN
    return get_ldo5v_detect_flag();
#else
    return 0;
#endif
}

void charge_mode_detect_ctl(u8 sw)
{
#if CHARGE_PROTECT_EN
    rtc_module_ldo5v_detect(sw, 0);
#endif
}

static void charge_full_deal(void)
{
#if CHARGE_PROTECT_EN
    charge_printf("--full--\n");
    R_LED_OFF();
    B_LED_ON();
#endif
}

static u16 charge_adc_res(u32 channel)
{
#if CHARGE_PROTECT_EN
    u16 adc_value = 0;
    u16 adc_con_tmp = 0;

    while (!(BIT(7) & JL_ADC->CON));	  //wait pending
    adc_value = JL_ADC->RES;

    adc_con_tmp = JL_ADC->CON & (~0x0F00);
    JL_ADC->CON = adc_con_tmp | channel; //AD channel select
    JL_ADC->CON |= BIT(6);             //AD start

    return adc_value;
#else
    return 0;
#endif
}


static void charge_timer0_isr_callback()
{
#if CHARGE_PROTECT_EN

    u8 val;
    charge_t0_cnt++;

    charge_deal(charge_adc_res(AD_KEY_LDOIN));


    if ((charge_t0_cnt % 5) == 0) {
        if (!get_ldo5v_online_flag()) {
            power_off_cnt ++;
            if (power_off_cnt > 20) {
                soft_power_ctl(PWR_OFF);
            }
        } else {
            power_off_cnt = 0;
        }
    }

    if (charge_t0_cnt == 250) {
        charge_t0_cnt = 0;

        //限定CHARGE_MAX_CNT时间内能一定显示充满
        __this->charge_cnt ++;
        if (__this->charge_cnt >= CHARGE_MAX_CNT) {
            __this->charge_cnt = 0;
            charge_full_deal();
        }
        /* charge_putchar('H'); */
    }
#endif
}

static s32 charge_timer0_init(void)
{
#if CHARGE_PROTECT_EN

    s32 ret;
    __timer_param   timer_parm;
    timer_module_on();
    charge_timer0_hl = timer_open(TIMER0, TIMER_MAX_ISR_FUN);
    if (NULL == charge_timer0_hl) {
        printf("timer_open err");
        ret = TIMER_DRV_OPEN_ERR;
        return ret;
    }
    timer_parm.work_mode  = TIMER_WORK_MODE_COUNTER;
    timer_parm.tick_time  = 2000;//(2ms)
    ret = timer_init_api(charge_timer0_hl, &timer_parm);

    if (ret != TIMER_NO_ERR) {
        printf("timer_init err = %x\n", ret);
        return ret;
    }

    ret = timer_start(charge_timer0_hl);
    if (ret != TIMER_NO_ERR) {
        printf("timer_start err = %x\n", ret);
        return ret;
    }
    ret = timer_reg_isr_callback_fun(charge_timer0_hl, 1, charge_timer0_isr_callback);
    if (ret != TIMER_NO_ERR) {
        printf("timer_reg_isr_callback_fun err = %x\n", ret);
        return ret;
    }
    return ret;
#else
    return 0;
#endif
}

static void delay_ms()
{
    //Timer2 for delay
    JL_TIMER2->CON = BIT(14);
    JL_TIMER2->PRD = 375;
    JL_TIMER2->CNT = 0;
    SFR(JL_TIMER2->CON, 2, 2, 2); //use osc
    SFR(JL_TIMER2->CON, 4, 4, 3); //div64
    SFR(JL_TIMER2->CON, 14, 1, 1); //clr pending
    SFR(JL_TIMER2->CON, 0, 2, 1); //¶¨Ê±/¼ÆÊýÄ£Ê½
    while (!(JL_TIMER2->CON & BIT(15)));
    JL_TIMER2->CON = BIT(14);
}

static void delay_nms(u32 n)
{
    while (n--) {
        delay_ms();
    }
}

static void check_power_on_voltage(void)
{
    u8 val = 0;
    u8 tmp;

    u8 low_power_cnt = 0;
    u32 normal_power_cnt = 0;
    u32 delay_2ms_cnt = 0;

    adc_init_api(ad_table[0], clock_get_lsb_freq(), SYS_LVD_EN);

    LED_INIT_EN();
    KEY_INIT();

#if CHARGE_PROTECT_EN
    while (!get_ldo5v_online_flag())
#else
    while (1)
#endif
    {
        clear_wdt();
        delay_nms(2); //2ms
        delay_2ms_cnt++;
        adc_scan(NULL);

        if ((delay_2ms_cnt % 5) == 0) { //10ms
            delay_2ms_cnt = 0;
            /*battery check*/
            val = get_battery_level();
            if (val < 31) {
                low_power_cnt++;
                if (low_power_cnt > C_POWER_BAT_CHECK_CNT) {
                    R_LED_ON();
                    B_LED_OFF();
                    delay_nms(1000); //1s
                    soft_power_ctl(PWR_OFF);
                }
            } else {
                normal_power_cnt++;
                if (normal_power_cnt > C_POWER_BAT_CHECK_CNT) { //normal power
                    delay_2ms_cnt = 0;

                    while (1) {
                        clear_wdt();
                        delay_nms(2); //2ms
#if KEY_IO_EN
                        if (IS_KEY0_DOWN())
#else
                        if (1)
#endif
                        {
                            putchar('+');
                            delay_2ms_cnt++;
                            if (delay_2ms_cnt > C_POWER_KEY_CHECK_CNT) {
                                R_LED_ON();
                                B_LED_OFF();
                                return;
                            }
                        } else {
                            putchar('-');
                            delay_2ms_cnt = 0;
                            soft_power_ctl(PWR_OFF);
                        }
                    }
                }
            }
        }
    }
}

static u8 power_on_check(void)
{
    return 1;
}

static u8 power_off_check(void)
{
    return 1;
}

static void charge_power_idle(u8 mode)
{
#if CHARGE_PROTECT_EN

    LED_INIT_EN();
    R_LED_ON();
    B_LED_OFF();

    charge_var_init();

    adc_init_api(AD_KEY_LDOIN, clock_get_lsb_freq(), SYS_LVD_EN);

    charge_timer0_init();

    while (1) {
        clear_wdt();
        __asm__ volatile("idle");
        __asm__ volatile("nop");
        __asm__ volatile("nop");
    }
#endif
}

static void set_io_to_low_power_mode()
{
#if CHARGE_PROTECT_EN

    SFR(WLA_CON17, 10, 4, 0x0);   //osc HCS
    SFR(WLA_CON17, 0, 5, 0x7);    //osc CLS
    SFR(WLA_CON17, 5, 5, 0x7);    //osc CRS
    SFR(WLA_CON14, 13, 1, 0x0);   //osc bt oe
    SFR(WLA_CON14, 14, 1, 0x1);   //osc fm oe
    SFR(WLA_CON17, 14, 2, 0x0);   //osc LDO level
    SFR(WLA_CON14, 11, 1, 0x0);   //osc ldo en
    SFR(WLA_CON14, 12, 1, 0x0);   //osc test
    SFR(WLA_CON18, 2, 2, 0x0);    //osc xhd current

    dac_off_control(); //close dac mudule

    JL_AUDIO->LADC_CON = 0;
    JL_AUDIO->DAC_CON = 0;
    JL_AUDIO->ADA_CON0 = 0;
    JL_AUDIO->ADA_CON1 = 0;
    JL_AUDIO->ADA_CON2 = 0;

    JL_AUDIO->DAA_CON0 = 0;
    JL_AUDIO->DAA_CON1 = 0;
    JL_AUDIO->DAA_CON2 = 0;
    JL_AUDIO->DAA_CON3 = 0;
    JL_AUDIO->DAA_CON4 = 0;
    JL_AUDIO->DAA_CON5 = 0;

    SFR(JL_FMA->CON1, 12, 1,  0x0);

    JL_PORTA->DIR = 0xffff;
    JL_PORTA->PU  = 0;
    JL_PORTA->PD  = 0;
    JL_PORTA->DIE = 0;

    JL_PORTB->DIR = 0xffff;
    JL_PORTB->PU  = 0;
    JL_PORTB->PD  = 0;
    JL_PORTB->DIE = 0;

    JL_PORTC->DIR = 0xffff;
    JL_PORTC->PU  = 0;
    JL_PORTC->PD  = 0;
    JL_PORTC->DIE = 0;

    JL_PORTD->DIR |= 0xfff0;
    JL_PORTD->PU  &= ~0xfff0;
    JL_PORTD->PD  &= ~0xfff0;
    JL_PORTD->DIE &= ~0xfff0;

    PORTR_PU(PORTR0, 0);
    PORTR_PD(PORTR0, 0);
    PORTR_DIR(PORTR0, 1);
    PORTR_DIE(PORTR0, 0);

    PORTR_PU(PORTR1, 0);
    PORTR_PD(PORTR1, 0);
    PORTR_DIR(PORTR1, 1);
    PORTR_DIE(PORTR1, 0);

    PORTR_PU(PORTR3, 0);
    PORTR_PD(PORTR3, 0);
    PORTR_DIR(PORTR3, 1);
    PORTR_DIE(PORTR3, 0);

#ifndef USE_USB_DM_PRINTF
    USB_DP_DIR(1); //DP设置为输入
    USB_DM_DIR(1); //DP设置为输入
    USB_DP_PD(0);
    USB_DM_PD(0);
    USB_DM_PU(0);
    USB_DP_PU(0);

#endif  //USE_USB_DM_PRINTF

    SFR(JL_SYSTEM->LDO_CON, 15, 3, 0b101);     // 3.3 -> 1.2 DVDD set to 1.0v
    /* SFR(JL_SYSTEM->LDO_CON, 15, 3, 0b110);     // 3.3 -> 1.2 DVDD set to 0.9v */
#endif
}

static void charge_deal(u16 vbat)
{
#if CHARGE_PROTECT_EN

    u16 value = 0;

    /* charge_printf("%x ",vbat); */

    switch (__this->step) {
    case STEP_GET_POINT:
        __this->point_cnt++;
        __this->point_total += vbat;

        if ((__this->last_bat_value == 0) && (__this->point_cnt >= BUILD_POINT_INIT_CNT)) {
            __this->last_bat_value = __this->point_total / (BUILD_POINT_INIT_CNT / (1 << ADC_OVERSAMPLING_ACCURACY));
            /* charge_printf("init  last_bat_value: %x\n",__this->last_bat_value); */
        }

        if (__this->point_cnt >= BUILD_POINT_CNT) {
            __this->step = STEP_GET_LINE;
        }
        break;

    case STEP_GET_LINE:
        __this->point_average = __this->point_total / (BUILD_POINT_CNT / (1 << ADC_OVERSAMPLING_ACCURACY));

#ifdef CHARGEE_DBG
        value = __this->point_average - __this->last_bat_value;

        if (__this->point_average - __this->last_bat_value > max_value) {
            max_value = value;
        }
        if (__this->point_average - __this->last_bat_value < min_value) {
            min_value = value;
        }

        time_cnt++;
        charge_printf("time:%d last:%x cur:%x D-value:%d max:%d min:%d\n", time_cnt, __this->last_bat_value, __this->point_average, value, max_value, min_value);

#endif
        if (__this->point_average - __this->last_bat_value <= BUILD_POINT_THRESHOLD_VALUE) {
            charge_full_deal();
        }

        __this->last_bat_value = __this->point_average;
        __this->point_cnt = 0;
        __this->point_total = 0;
        __this->point_average = 0;
        __this->step = STEP_GET_POINT;
        break;

    default:
        __this->step = STEP_GET_POINT;
        break;
    }
#endif
}

/* mode 1:power on     2:power off*/
void ldo5v_detect_deal(u8 mode)
{
    u8 ret = 0;
    u8 i = 0;
    u8 online_cnt = 0;

    u8 change_clk_flag = 0;

    printf("----01\n");

#if CHARGE_PROTECT_EN

    //check wheather ldo5v online
    for (i = 0; i < 10; i++) {
        delay_nms(2);
        if (get_ldo5v_online_flag()) {
            online_cnt++;
        }
    }

    if (online_cnt > 3) {
        if (mode == POWER_ON) {
            printf("----02\n");
            ret = power_on_check();
        } else if (mode == POWER_OFF) {
            printf("----03\n");
            ret = power_off_check();
        }

        if (ret) {
            printf("----04\n");
            extern void set_pwrmd(u8 mode);
            set_pwrmd(1);
            set_io_to_low_power_mode();

            clock_init(SYS_CLOCK_INPUT_BT_OSC, OSC_Hz, 24000000L);
            clock_set_sys_div(12);  //sys run 2M
            uart_set_baud(9600);
            set_sys_ldo_level(3);
            charge_power_idle(mode);
            change_clk_flag = 1;
        }
    } else
#endif
    {
        if (mode == POWER_ON) {
            printf("----05\n");
            check_power_on_voltage();
        }
    }

#if CHARGE_PROTECT_EN
    if (change_clk_flag) {
        set_sys_ldo_level(1);
        clock_init(SYS_CLOCK_IN, OSC_Hz, SYS_Hz);
        uart_set_baud(UART_BAUD_RAE);
    }
#endif
    return;
}

void charge_disconnect_bt_check(void)
{
#if CHARGE_PROTECT_EN
    static u8 send_msg_cnt = 0;

    if (get_ldo5v_online_flag()) {                   //充电线插入
        /* charge_printf("in : %d\n",send_msg_cnt); */
        send_msg_cnt ++;
        if (!__this->send_msg_flag && send_msg_cnt > 10) {
            __this->send_msg_flag = 1;
            send_msg_cnt = 0;

            JL_POWER->CON |= BIT(4);
            /* os_taskq_post("MainTask", 2, MSG_POWER_OFF_KEY_MSG, 0x44); */
        }
    } else {
        send_msg_cnt = 0;
        __this->send_msg_flag = 0;
    }
#endif
}
