/*********************************************************************************************
    *   Filename        : sdk_cfg.h

    *   Description     : Config for Head Phone Case

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2016-12-01 17:26

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/

#ifndef _CONFIG_
#define _CONFIG_

#include "includes.h"



///任务堆栈大小配置测试
#define VM_TASK_STACK_SIZE          (1024 * 1)
#define MAIN_TASK_STACK_SIZE        (1024 * 2) //(1024 * 1)可行
#define MUSIC_TASK_STACK_SIZE       (1024 * 4)
#define MUSIC_PHY_TASK_STACK_SIZE   (1024 * 4)
#define TONE_TASK_STACK_SIZE        (1024 * 4)
#define TONE_PHY_TASK_STACK_SIZE    (1024 * 2)
#define UI_TASK_STACK_SIZE          (1024 * 2)

//配置对箱
///<蓝牙对耳使能,对耳暂时不开放ble
#define  BT_TWS                       0 
///对耳从机开机是否进入被连接还是回链状态
#define    TWS_SLAVE_WAIT_CON         1  
///对耳主从是否一起关机
#define    BT_TWS_POWEROFF_TOGETHER   1 

///对耳没连接成功之前，通过按键开启可发现使能,即主从同时按下配对按键才进行配对
#define    BT_TWS_SCAN_ENBLE         0 

//charge enable,C版以上的芯片才可用
#define CHARGE_PROTECT_EN    0

/********************************************************************************/
/*
 *          --------调试类配置
 */
////<开启系统打印调试功能
#define __DEBUG         

#ifdef __DEBUG
	// #define USE_USB_DM_PRINTF
#endif

///<LED指示使能
#define LED_EN                  1       
///<串口升级
#define UART_UPDATA_EN          0      
///<电量监测
#define SYS_LVD_EN              1       

///<按键双击功能
#define KEY_DOUBLE_CLICK             1  
/********************************************************************************/

/********************************************************************************/
/*
 *           --------电源类配置
 */
///   0:  no change   
#define    PWR_NO_CHANGE        0     
///   1:  LDOIN 5v -> VDDIO 3.3v -> DVDD 1.2v
#define    PWR_LDO33            1     
///   2:  LDOIN 5v -> LDO   1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_LDO15            2     
///   3:  LDOIN 5v -> DCDC  1.5v -> DVDD 1.2v, support bluetooth
#define    PWR_DCDC15           3     

///可选配置：PWR_NO_CHANGE/PWR_LDO33/PWR_LDO15/PWR_DCDC15
#define PWR_MODE_SELECT         PWR_DCDC15

//DCDC时BTAVDD,用LDO模式时可以忽略 0:1.61v  1:1.51v  2:1.43v  3:1.35v
//AC6904封装要用1.51v,即应设置为1
#define VDC15_LEV     3      

//0:0.703V  1:0.675V  2:0.631V  3:0.592V
//4:0.559V  5:0.527V  6:0.493V  7:0.462V 
//LC6904封装建议设置为5
#define POWER_DOWN_DVDD_LEV      7

///蓝牙无连接自动关机计时，u16类型，0表示不自动关机
#define AUTO_SHUT_DOWN_TIME     (3*2*60)          

///<电池电量低，是否切换电源输出配置
#define SWITCH_PWR_CONFIG		0
/*
	SYS_LDO_Level:3.53v-3.34v-3.18v-3.04v-2.87v-2.73v-2.62v-2.52v
	FM_LDO_Level:3.3v-3.04v-2.76v-2.5v
*/
///<Normal LDO level
#define SYS_LDO_NORMAL_LEVEL	2	//range:0~7:FM_LDO和VDDIO绑一起，建议:level=2
#define FM_LDO_NORMAL_LEVEL		0	//range:0~3
///<Reduce LDO level
#define SYS_LDO_REDUCE_LEVEL	3	//range:0~7
#define FM_LDO_REDUCE_LEVEL		1	//range:0~3

/********************************************************************************/

/********************************************************************************/
/*
 *           --------DAC VCOMO 配置
 */
///是否选择VCMO直推耳机
#define VCOMO_EN 	            1	
///可选配置：DAC_L_R_CHANNEL/DAC_L_CHANNEL/DAC_R_CHANNEL  
#define DAC_CHANNEL_SLECT       DAC_L_R_CHANNEL

//DAC 声音输出到IIS
#define DAC2IIS_EN                      0     //1:使能DAC声音从IIS以固定采样率44.1K输出。
  #if(DAC2IIS_EN == 1) 
     #define DAC2IIS_OUTCLK_AUTO_CLOSE  O    //1:当没有声音时，自动关闭MCLK,SCLK,LRCLK时钟输出。 0:不关闭
     #define IISCHIP_WM8978_EN          1    //1: 使能WM8978 IIS芯片。
  #else
     #define DAC2IIS_OUTCLK_AUTO_CLOSE  0    //1:当没有声音时，自动关闭MCLK,SCLK,LRCLK时钟输出。 0:不关闭
     #define IISCHIP_WM8978_EN          0    //1: 使能WM8978 IIS芯片。
  #endif
/********************************************************************************/

/********************************************************************************/
/*
 *           --------音效类配置
 */
///<EQ模块开关
#define EQ_EN			        1       
///<EQ uart online debug
#define EQ_UART_BDG	    		0       
///<dac声道合并
#define DAC_SOUNDTRACK_COMPOUND 0       
///<自动mute
#define DAC_AUTO_MUTE_EN		1       
///<按键音
#define KEY_TONE_EN     	    1       
///<非0表示使用默认音量
#define SYS_DEFAULT_VOL         0      

///电话接听和挂断用语言识别  
#define  USE_HTK                0             

///1:播完来电提示音后启动语言识别  0：一直播着提示音启动语言识别,一直播提示音要把系统时钟提高到160M以上
#define TONE_AFTER_HTK_START    1

/********************************************************************************/


/********************************************************************************/
/*
 *           --------外设类配置
 */
#define SDMMC0_EN       0
#define SDMMC1_EN       0
#define USB_DISK_EN     0
#define USB_PC_EN       0

/********************************************************************************/




/********************************************************************************/
/*
 *           --------蓝牙类配置
 */
#include "bluetooth/bluetooth_api.h"

#define NFC_EN          0  ///<NFC ENABLE

///可选配置：NORMAL_MODE/TEST_BQB_MODE/TEST_FCC_MODE/TEST_FRE_OFF_MODE/TEST_BOX_MODE
#define BT_MODE             NORMAL_MODE

#if ((BT_MODE == TEST_FCC_MODE)||(BT_MODE == TEST_BQB_MODE))
  #undef PWR_MODE_SELECT
  #define PWR_MODE_SELECT         PWR_LDO15
#endif
#if (BT_MODE == TEST_FCC_MODE)
  #undef UART_UPDATA_EN
  #define UART_UPDATA_EN    0
#endif

///模拟配置
#define BT_ANALOG_CFG       0
#define BT_XOSC             0//

///<蓝牙后台
#define BT_BACKMODE         0             
///dependency
#if (BT_BACKMODE == 0)
    ///<HID拍照的独立模式只支持非后台
    #define BT_HID_INDEPENDENT_MODE  0    
#endif

#if USE_HTK
///<来电报号
  #define BT_PHONE_NUMBER     0          
#else
   #define BT_PHONE_NUMBER     1          
#endif

///<蓝牙K歌宝
#define BT_KTV_EN			0

/*
 *           --------蓝牙低功耗设置 
 */
///可选配置：SNIFF_EN/SNIFF_TOW_CONN_ENTER_POWERDOWN_EN
#define SNIFF_MODE_CONF    SNIFF_EN

///可选配置：BT_POWER_DOWN_EN/BT_POWER_OFF_EN
#define BT_LOW_POWER_MODE  BT_POWER_DOWN_EN //BT_POWER_OFF_EN

#define BT_OSC              0
#define RTC_OSCH            1
#define RTC_OSCL            2

///可选配置：BT_OSC/RTC_OSCH/RTC_OSCL
#define LOWPOWER_OSC_TYPE   BT_OSC

///可选配置：32768L//24000000L
#define LOWPOWER_OSC_HZ     24000000L

///可选配置：BT_BREDR_EN/BT_BLE_EN/(BT_BREDR_EN|BT_BLE_EN)
#define BLE_BREDR_MODE      (BT_BREDR_EN)

///TWS 固定左右耳选择 可选配置：TWS_CHANNEL_LEFT/TWS_CHANNEL_RIGHT)
#define    BT_TWS_CHANNEL_SELECT         0 ////固定左右耳时,左耳为主,右耳不进行搜索配对

/********************************************************************************/

/********************************************************************************/
/*
 *           --------芯片封装配置 
 */
///RTCVDD口没有绑出来要置1,目前对应封装芯片AC6905
#define RTCVDD_TYPE              0          
#define BTAVDD_TYPE              0          

/********************************************************************************/

/********************************************************************************/
/*
 *           --------MUSIC MACRO
 */
//SMP加密文件支持
#define MUSIC_DECRYPT_EN 		1	
//AB断点支持
#define MUSIC_AB_RPT_EN 		1	

///<MP3
#define DEC_TYPE_MP3_ENABLE     1
///<SBC
#define DEC_TYPE_SBC_ENABLE     1
///<AAC
#define DEC_TYPE_AAC_ENABLE		0

///<3K_code_space
#define DEC_TYPE_WAV_ENABLE     1
///<5K_code_space
#define DEC_TYPE_FLAC_ENABLE    0
///<8K_code_space
#define DEC_TYPE_APE_ENABLE     0
///<30K_code_space
#define DEC_TYPE_WMA_ENABLE     0
///<30K_code_space
#define DEC_TYPE_F1A_ENABLE     0

/********************************************************************************/

/********************************************************************************/
/*
 *           --------FM MACRO
 */
///<标准SDK
#define FM_RADIO_EN         0       
///dependency
#if (FM_RADIO_EN == 1)
    ///<RDA5807FM
    #define RDA5807                 0       
    ///<BK1080FM
    #define BK1080                  0       
    ///<QN8035FM
    #define QN8035                  0       
    ///<芯片内部FM
    #define FM_INSIDE               1       
#endif

/********************************************************************************/

/********************************************************************************/
/*
 *           --------ECHO MACRO
 */
///dependency
#if (BT_BACKMODE == 1)
    ///<不支持与蓝牙后台开启混响
    #define ECHO_EN             0       
    ///<混响模式 标准SDK
    #define ECHO_TASK_EN        0       
#else
    ///<混响功能 标准SDK
    #define ECHO_EN             0       
    ///<混响模式 标准SDK
    #define ECHO_TASK_EN        0       
#endif

/********************************************************************************/

/********************************************************************************/
/*
 *           --------RTC MACRO
 */
///<标准SDK RTC时钟模式
#define RTC_CLK_EN          0       
///dependency
#if (RTC_CLK_EN == 1)
    ///<RTC闹钟模式
    #define RTC_ALM_EN          1       
#endif

/********************************************************************************/

/********************************************************************************/
/*
 *           --------REC MACRO
 */
///dependency
#if (BT_BACKMODE == 1)
    ///<不支持与蓝牙后台开启录音
    #define REC_EN             0    
#else
    ///<标准SDK录音功能
    #define REC_EN             0    
#endif

#if (REC_EN == 1)
    ///<蓝牙录音使能
	#define BT_REC_EN		1       
    ///<MIC录音使能
	#define MIC_REC_EN		1       
    ///<FM录音使能
	#define FM_REC_EN		1       
    ///<AUX录音使能
	#define AUX_REC_EN		1       
#endif

/********************************************************************************/

/********************************************************************************/
/*
 *           --------UI MACRO
 */
///<UI_显示
#define UI_ENABLE                0     
///dependency
#if (UI_ENABLE == 1)
    #define LCD_128X64_EN        1      ///<lcd 支持
    #define LED_7_EN             0      ///<led 支持
#else
    ///都不支持
    #define LCD_128X64_EN        0
    #define LED_7_EN             0
#endif 
#if ((LCD_128X64_EN == 1) && (LED_7_EN == 1))
#error  "UI driver support only one"
#endif

#if (LCD_128X64_EN == 1)
    ///菜单显示
    #define LCD_SUPPORT_MENU     1       
    ///LRC歌词显示
    #define LRC_LYRICS_EN        1       
#else
    #define LCD_SUPPORT_MENU     0
    #define LRC_LYRICS_EN        0
#endif

/********************************************************************************/

#if (BT_MODE !=NORMAL_MODE)
  #if ((SNIFF_MODE_CONF)||(BT_LOW_POWER_MODE))
  #error  "-------BT MODE not support this config ,please check sdk_cfg.h------------"
  #endif
#endif


#endif
