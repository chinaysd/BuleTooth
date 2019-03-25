/******************************************************************************************* 
 *
 *   File Name: .h 
 *
 *   Version: 1.00 
 *
 *   Discription: 
 *
 *   Author:Bingquan Cai 
 *
 *   Email :bingquan_cai@zh-jieli.com 
 *
 *   Date:
 *
 *   Copyright:(c)JIELI  2016  @ , All Rights Reserved.
 *
 *   *******************************************************************************************/

#ifndef __INTERRUPT_API_H__
#define __INTERRUPT_API_H__

#include "typedef.h"

#define IRQ_TIME0_IDX      2		//0
#define IRQ_TIME1_IDX      3		//0
#define IRQ_TIME2_IDX      4		//0
#define IRQ_TIME3_IDX      5		//0
#define IRQ_USB_SOF_IDX    6		//1
#define IRQ_USB_CTRL_IDX   7		//1
#define IRQ_RTC_IDX        8		//0
#define IRQ_ALINK_IDX      9		//1
#define IRQ_DAC_IDX        10		//1
#define IRQ_PORT_IDX       11		//0
#define IRQ_SPI0_IDX       12		//0
#define IRQ_SPI1_IDX       13		//0
#define IRQ_SD0_IDX        14		//0
#define IRQ_SD1_IDX        15		//0
#define IRQ_UART0_IDX      16		//0
#define IRQ_UART1_IDX      17		//0
#define IRQ_UART2_IDX      18		//0
#define IRQ_PAP_IDX        19		//0
#define IRQ_IIC_IDX        20		//0
#define IRQ_SARADC_IDX     21		//0
#define IRQ_FM_HWFE_IDX    22		//1
#define IRQ_FM_IDX         23		//1
#define IRQ_FM_LOFC_IDX    24		//1
#define IRQ_BREDR_IDX      25		//2
#define IRQ_BT_CLKN_IDX    26		//2
#define IRQ_BT_DBG_IDX     27		//1
#define IRQ_BT_PCM_IDX     28		//2
#define IRQ_SRC_IDX        29		//1
#define IRQ_EQ_IDX         31		//1
#define IRQ_BLE_IDX        36		//2
#define IRQ_NFC_IDX        37		//1
#define IRQ_CTM_IDX		   43		//1
#define IRQ_SOFT0_IDX      62		//0
#define IRQ_SOFT_IDX       63		//0
#define IRQ_EXCEPTION_IDX  64		//0

struct irq_driver {
    void (*init)(void);

    void (*handler_register)(u32, void *, u32);

    void (*handler_unregister)(u32);

    void (*commmon_handler)(u32);

    void (*enable)(u32);

    void (*disable)(u32);

    bool (*read)(u32);

    void (*normal_enable)();

    void (*normal_disable)();

    void (*normal_disable_all)();

    void (*sys_enable)();

    void (*sys_disable)();
};

// #define REGISTER_IRQ_DRIVER(drv) \
	// const struct irq_driver *__irq_drv \
			// __attribute__((section(".text"))) = &drv
#define REGISTER_IRQ_DRIVER(drv) \
	const struct irq_driver *__irq_drv \
			sec_used(.ram1_data) = &drv

extern const struct irq_driver *__irq_drv;

/********************************************************************************/
/*
 *      API
 */
void irq_init(void);

void irq_handler_register(u32 idx, void *hdl, u32 prio);

void irq_handler_unregister(u32 idx);

void irq_common_handler(u32 idx);

void irq_enable(u32 idx);

void irq_disable(u32 idx);

bool irq_read(u32 idx);

void irq_global_enable(void);

void irq_global_disable(void);

void irq_clear_all_ie(void);

void irq_sys_enable(void);

void irq_sys_disable(void);

void irq_index_tab_reg(void * tab, u32 max_cnt);//max_cnt:byte

u8 irq_index_to_prio(u8 idx);

#define IRQ_REGISTER(idx, hdl) \
    SET(interrupt("")) void irq_##hdl() \
    {\
        hdl();\
        irq_common_handler(idx);\
    }

#define IRQ_REQUEST(idx, hdl, prio) \
    irq_handler_register(idx, irq_##hdl, irq_index_to_prio(idx))

#endif
