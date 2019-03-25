#ifndef __LCD_DRV_API_H__
#define __LCD_DRV_API_H__

#include "sdk_cfg.h"

///************************************************************/
///****************配置屏幕的大小
///****************说明：点阵屏是一个点对应一个bit
///************************************************************/

/*****************************************************************
66、 AC69 系列 SPI 接口有多组。 分别如下。
SPI0 这组被内部的 FLASH 用到了， 所以仅能用 SPI1 SPI2 这对应的 IO 口。


SPI0
PD0 SPI0CLKA PD1 SPI0DOA PD2 SPI0DIA PD3 SPI0CSA    ---  对应的是A出口
PB8 SPI0CLKB PB7 SPI0DOB PB5 SPI0DIB PB6 SPI0CSB    ---  对应的是B出口


SPI2
PB0 SPI2CLKA PB1 SPI2DOA PB2 SPI2DIA       ---  对应的是A出口
PC0 SPI2CLKB PC1 SPI2DOB PC2 SPI2DIB       ---  对应的是B出口


SPI1
PB11 SPI0CLKB PB12 SPI0DOB PB10 SPI0DIB    ---  对应的是A出口
PC4  SPI0CLKB PC5  SPI0DOB  PC3 SPI0DIB    ---  对应的是B出口

LCD-CS   --- PB9
LCD-RES  --- PB8
LCD-AO   --- PB7
LCD-CLK  --- PB11
LCD-DAT  --- PB12

*****************************************************************/

#define  LCDPAGE            8
#define  LCDCOLUMN          128

#define SCR_WIDTH           LCDCOLUMN
#define SCR_HEIGHT          (LCDPAGE*8)



//SPI1接口选择
#if 1//
//SPI1A
#define LCD_SPI_SET_GROUP()       do{JL_IOMAP->CON1 &= ~BIT(4);}while(0)
#define LCD_SPI_SET_DATAIN()      do{JL_PORTB->DIR &= ~BIT(10);}while(0)
#define LCD_SPI_SET_CLK()	      do{JL_PORTB->DIR &= ~BIT(11);}while(0)//第4脚
#define LCD_SPI_SET_DATAOUT()     do{JL_PORTB->DIR &= ~BIT(12);}while(0)//第5脚，6脚3v3

#else
//SPI1B
#define LCD_SPI_SET_GROUP()       do{JL_IOMAP->CON1  |= BIT(4);}while(0)
#define LCD_SPI_SET_DATAIN()      do{PORTC_DIR &= ~BIT(2);}while(0)
#define LCD_SPI_SET_CLK()	      do{PORTC_DIR &= ~BIT(5);}while(0)
#define LCD_SPI_SET_DATAOUT()     do{PORTC_DIR &= ~BIT(8);}while(0)

#endif

#define LCD_DATA_OUT()
#define	LCD_CLK_OUT()
#define LCD_SPI_SET_MODE_OUT()       do{JL_SPI1->CON &= ~BIT(3);}while(0)
#define LCD_SPI_SET_MODE_INOUT()     do{JL_SPI1->CON |= BIT(3);}while(0)


#define	LCD_A0_OUT()	    do{JL_PORTB->DIR &= ~BIT(10);}while(0)//第三脚
#define	LCD_A0_L()	        do{JL_PORTB->OUT &= ~BIT(10);}while(0)
#define	LCD_A0_H()	        do{JL_PORTB->OUT |= BIT(10);}while(0)

#define LCD_RES_OUT()	    do{JL_PORTB->DIR &= ~BIT(8);}while(0)//第二脚
#define LCD_RES_L()	        do{JL_PORTB->OUT &= ~BIT(8);}while(0)
#define LCD_RES_H()	        do{JL_PORTB->OUT |= BIT(8);}while(0)

#define LCD_CS_OUT()	    do{JL_PORTB->DIR &= ~BIT(9);}while(0)//第一脚
#define LCD_CS_L()	        do{JL_PORTB->OUT &= ~BIT(9);}while(0)
#define LCD_CS_H()	        do{JL_PORTB->OUT |= BIT(9);}while(0)


#define LCD_BL_ON()         //do{PORTB_DIR &= ~BIT(2); PORTB_OUT |= BIT(2);}while(0)
#define LCD_PORT_OUT()      do{LCD_DATA_OUT();LCD_CLK_OUT();LCD_A0_OUT();LCD_RES_OUT();LCD_CS_OUT();}while(0)
#define LCD_PORT_OUT_H()    do{LCD_DATA_OUT();LCD_CLK_OUT();LCD_A0_H();  LCD_RES_H();	LCD_CS_H();}while(0)

//全局变量声明
extern u8 disp_buf[];

//函数声明
void draw_lcd_buf(void);
void lcd_clear(void);
void lcd_init(void);
void lcd_hardware_init(void);
void lcd_128x64_set(u8 page,u8 column);
extern bool font_init(u8 language);
#endif/*__LCD_DRV_API_H__*/
