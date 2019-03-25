/*
 *********************************************************************************************************
 *Version: 1.0  2017-06-13 junqian
 *              //first version, only support android APP.
 *Version: 1.1  2017-06-22 junqian
 *              //communication use big endian, IOS APP support, add some cmd.
 *********************************************************************************************************
 */
#include "typedef.h"
#include "sdk_cfg.h"
#include "common/msg.h"
#include "common/app_cfg.h"
#include "rtos/os_api.h"
#include "rtos/task_manage.h"

#if  BLE_FINGER_SPINNER_EN
extern u16 cal_crc16_deal(u8 *ptr, u16 len);
extern void clear_wdt(void);
extern bool blefs_com_run_flag;


#if 0  //debug
#define blefs_deg     printf
#define blefs_degbuf  printf_buf
#else
#define blefs_deg(...)
#define blefs_degbuf(...)
#endif

#define BLEFS_MAX_BUF_LEN   (512+32)
#define BLEFS_TASK_NAME     "BleFsTask"

#if SUPPORT_APP_RCSP_EN && BLE_FINGER_SPINNER_EN
#error "Can't Open SUPPORT_APP_RCSP_EN and BLE_FINGER_SPINNER_EN in sdk_cfg.h at same time"
#endif

enum{
	MSG_BLEFS_PIXEL_DATA_PARSE = 0x1,
	MSG_BLEFS_PIXEL_DATA_RECV,
	MSG_BLEFS_SEND_CSW,
	MSG_BLEFS_REPORT_SCREEN_RESOLUTION,
	MSG_DISP_BASE,
	MSG_DISP_STYLE,
	MSG_DISP_SPEED,
};

enum{
    CBW_REPORT_SCREEN_RESOLUTION = 0X01,
    CBW_DISP_BASE = 0x02,
    CBW_DISP_STYLE = 0x03,
    CBW_DISP_SPEED = 0x04,
};
enum{
    BASE_SPEED = 0x01,
    BASE_BATTERY = 0x02,
    BASE_VERSION = 0x03,
};

enum{
    STYLE_FIXED = 0x01,
    STYLE_LEFT2RIGHT = 0x02,
    STYLE_RIGHT2LEFT = 0x03,
    STYLE_OUTER2CENTER = 0x04,
    STYLE_CENTER2OUTER = 0x05,
    STYLE_SIDE2MIDDLE = 0x06,
    STYLE_MIDDLE2SIDE = 0x07,
    STYLE_ROTATE_CLOCKWISE = 0x08,
    STYLE_ROTATE_ANTICLOCKWISE = 0x09,
};

typedef u32 (*blefs_comsend_cbk_t)(u8 *data,u16 len);
//CSW
typedef struct _CSW
{
    u32 signature;      //[3:0] Fixed to "BTST",0x42545354
    u8  tag[4];         //[7:4] last CBW tag
    u32 reserved;       //reserved
    u8  sta;            //[12] operate sta
    u8  reserved1[3];
}CSW_T;

typedef struct{
	u16 screen_high;
	u16 screen_width;
	u16 recv_total;  //unit byte
	u16 recv_cnt;	 //unit byte
	CSW_T csw;
	u8  *recv_buf;
}BLE_FINGER_SPINNER;

BLE_FINGER_SPINNER *p_blefs;   //fs stand for FingerSpinner
bool blefs_com_run_flag = false;
u16* sp_disp_buf = NULL;       //disp_buf. user could use this buf to display directly.
blefs_comsend_cbk_t blefs_comsend_callback = NULL;

//u16 big_little endian swap
u16 endian_swap_u16(u16 n)
{
    return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}
//u32 big_little endian swap
u32 endian_swap_u32(u32 n)
{
    return ((n & 0xff) << 24) |
           ((n & 0xff00) << 8) |
           ((n & 0xff0000UL) >> 8) |
           ((n & 0xff000000UL) >> 24);
}

bool blefs_register_comsend(blefs_comsend_cbk_t cbk)
{
    if(!blefs_com_run_flag){
        blefs_deg("ble finger spinner not run\n");
        return false;
    }
    if(blefs_comsend_callback != NULL){
        blefs_deg("already register\n");
    }
    blefs_comsend_callback = cbk;
    return TRUE;
}

static void report_screen_resolution(void)
{
	p_blefs->csw.reserved = (endian_swap_u16(p_blefs->screen_width)<<16) | endian_swap_u16(p_blefs->screen_high);
	p_blefs->csw.sta = 0;
	os_taskq_post_msg(BLEFS_TASK_NAME, 1, MSG_BLEFS_SEND_CSW);
}

//user could change pixel data format here
//example here: 10 leds, use hight 10 bit of u16 to display.
void copy_to_disp_buf(u8* buf, u16 len)
{
	int i;
	u8 *disp_buf = (u8*)sp_disp_buf;
	u8 tmp0;
	printf("copy_to_disp_buf\n");
	memset(sp_disp_buf,0x00,BLEFS_MAX_BUF_LEN);
	memcpy(sp_disp_buf,buf,len);

	for(i=0; i<len;){   //big edain to little edain
		tmp0 = disp_buf[i];
		disp_buf[i] = disp_buf[i+1];
		disp_buf[i+1] = tmp0;
		i += 2;
	}

	for(i=0; i<len/2;i++){
		sp_disp_buf[i] = sp_disp_buf[i]>>5;   //use low 11 bit of u16 to display.
	}
	printf("use low 11 bit of u16 to display(changed to little endian):\n");
	printf_buf((void*)sp_disp_buf,len);
}

void record_cbw_tag(u8* data)
{
	memcpy(p_blefs->csw.tag,data,4);
}

extern void blefs_com_task(void *p);
bool blefs_com_init(void)
{
    u8 err  = OS_NO_ERR;
    int blefs_len = sizeof(BLE_FINGER_SPINNER) + BLEFS_MAX_BUF_LEN*2;
    p_blefs = malloc(blefs_len);
	if(NULL == p_blefs){
		blefs_deg("malloc p_blefs fail\n");
		return false;
	}
    memset(p_blefs,0x00,blefs_len);
    p_blefs->recv_buf = (u8*)p_blefs + sizeof(BLE_FINGER_SPINNER);
    sp_disp_buf = (u16*)((u8*)p_blefs + sizeof(BLE_FINGER_SPINNER)+ BLEFS_MAX_BUF_LEN);
    printf("sp_disp_buf addr=0x%X\n",sp_disp_buf);
	err = os_task_create(blefs_com_task, (void*) 0, TaskBleFsComPrio, 60
		#if OS_TIME_SLICE_EN > 0
                         ,1
		#endif
                         , BLEFS_TASK_NAME);

    if (OS_NO_ERR != err){
	    blefs_deg("open blefs_com_task fail\n");
		return false;
    }

	blefs_deg("open blefs_com_task succ\n");
	p_blefs->csw.signature = 0x54535442;
	p_blefs->screen_high = 11;
	p_blefs->screen_width = 240;
	blefs_com_run_flag = true;
	return true;
}


void blefs_com_stop(void)
{
	if(!blefs_com_run_flag){
		return;
	}
	blefs_deg("ble_com_stop\n");
    blefs_com_run_flag = false;
    //OSTimeDly(1);
	if (os_task_del_req(BLEFS_TASK_NAME) != OS_TASK_NOT_EXIST)
    {
        os_taskq_post_event(BLEFS_TASK_NAME, 1, SYS_EVENT_DEL_TASK);
        do
        {
            OSTimeDly(1);
        }
        while (os_task_del_req(BLEFS_TASK_NAME) != OS_TASK_NOT_EXIST);
        blefs_deg("blecom:close ble_com_task succ\n");
    }
	if(p_blefs != NULL){
		blefs_deg("free ble_com_buf\n");
		free(p_blefs);
		p_blefs = NULL;
	}
}

bool is_blefs_cmd(u8* data)
{
    if(0x92!=data[15] || 0x10!=data[14] ){
        return false;
    }
	if( ((0x4A==data[0] && 0x4C==data[1] && 0x42==data[2] && 0x54==data[3]))){ //big Edian
		return true;
	}else{
		return false;
	}
}

bool pixel_data_crc_check(u8 *recv_buf, u16 recv_len)
{
    blefs_deg("pixel_data_crc_check\n");
	u8 *recv_data = recv_buf + 0x25;
	u16 recv_data_len = recv_len-0x25;
	u16 calc_crc = cal_crc16_deal(recv_data,recv_data_len);
	u16 recv_crc = ((u16)recv_buf[0x21]<<8) | recv_buf[0x22];
	blefs_deg("recv_crc_&_calc_crc:0x%X_0x%X\n",recv_crc,calc_crc);
	if(recv_crc == calc_crc){
         return true;
	}else{
	     return false;
	}
}


u32 blefs_comdata_parse(u8 *data, u16 len)
{
    bool report_csw_flag = false;
    static bool data_packet_flag = false;
	if(!blefs_com_run_flag){
		blefs_deg("ble_com not run\n");
		return 0;
	}
    //blefs_deg("rvlen=%d\n",len);
    //blefs_degbuf(data,len);

    //0x90 01  ask support screen resolution.
	if(is_blefs_cmd(data) && (0x00!=data[16]))   //01  //ask screen resolution;
	{
	    record_cbw_tag(&data[4]);
	    blefs_deg("cmd recv:\n");
	    blefs_degbuf(data,len);
	    switch(data[16])
	    {
            case CBW_REPORT_SCREEN_RESOLUTION:
                os_taskq_post(BLEFS_TASK_NAME, 1, MSG_BLEFS_REPORT_SCREEN_RESOLUTION);
                break;

            case CBW_DISP_BASE:
                os_taskq_post(BLEFS_TASK_NAME, 2, MSG_DISP_BASE,data[17]);
                report_csw_flag = true;
                break;
           case CBW_DISP_STYLE:
                os_taskq_post(BLEFS_TASK_NAME, 2, MSG_DISP_STYLE,data[17]);
                report_csw_flag = true;
                break;
           case CBW_DISP_SPEED:
                os_taskq_post(BLEFS_TASK_NAME, 2, MSG_DISP_SPEED,data[17]);
                report_csw_flag = true;
                break;
           default:
                break;
	    }
	    if(report_csw_flag){
            os_taskq_post(BLEFS_TASK_NAME, 2, MSG_BLEFS_SEND_CSW,0);
	    }
		return  0;
	}

    //0x90 00  data
	if(is_blefs_cmd(data) && 0x00==data[16])   //new pixel data packet;
	{
	    data_packet_flag = true;
		p_blefs->recv_cnt = 0;
		p_blefs->recv_total  = (((u16)data[10]<<8) | data[11]) + 0x20-1;
		memcpy(p_blefs->recv_buf, data, len);
		record_cbw_tag(&data[4]);
		p_blefs->recv_cnt += len;;
		blefs_deg("new pixel len = 0x%X_%d\n",p_blefs->recv_total,p_blefs->recv_total);
		blefs_deg("[%d]",p_blefs->recv_cnt);
		blefs_degbuf(data,len);

	}else if(data_packet_flag){
		if(p_blefs->recv_cnt >= BLEFS_MAX_BUF_LEN-len){
            data_packet_flag = false;
            os_taskq_post_msg(BLEFS_TASK_NAME, 3, MSG_BLEFS_PIXEL_DATA_RECV,p_blefs->recv_buf+0x25,p_blefs->recv_cnt-0x25);
            os_taskq_post(BLEFS_TASK_NAME, 2, MSG_BLEFS_SEND_CSW,1);
			printf("recv_buf full,disard %d data,recv_cnt=%d\n",p_blefs->recv_total-p_blefs->recv_cnt, p_blefs->recv_cnt);
			blefs_degbuf(p_blefs->recv_buf,p_blefs->recv_cnt);
			return 0;
		}
		memcpy(p_blefs->recv_buf + p_blefs->recv_cnt, data, len);
		p_blefs->recv_cnt += len;
		blefs_deg("[%d]", p_blefs->recv_cnt+len);
		blefs_degbuf(data,len);
		if(p_blefs->recv_total ==  p_blefs->recv_cnt){   //data recv complete
			blefs_deg("!!!packet ok\n");
			blefs_degbuf(p_blefs->recv_buf,p_blefs->recv_total);
			os_taskq_post_msg(BLEFS_TASK_NAME, 3, MSG_BLEFS_PIXEL_DATA_RECV,p_blefs->recv_buf+0x25,p_blefs->recv_total-

0x25);
			if(pixel_data_crc_check(p_blefs->recv_buf,p_blefs->recv_total)){
                 blefs_deg("recv CRC ok\n");
                 os_taskq_post(BLEFS_TASK_NAME, 2, MSG_BLEFS_SEND_CSW,0);   //crc ok
			}else{
			     blefs_deg("recv CRC fail\n");
                 os_taskq_post(BLEFS_TASK_NAME, 2, MSG_BLEFS_SEND_CSW,3);    //crc fail
			}
		}
	}else{
        blefs_deg("some other data<%d>", len);
		blefs_degbuf(data,len);
	}
	return 0;
}

u32 blefs_csw_send(u8 sta)
{
    u32 ret;
    ret = blefs_comsend_callback((u8*)&p_blefs->csw,13);
    p_blefs->csw.reserved = 0;
    return ret;
}
void blefs_com_task(void *p)
{
    int msg[6];
    u32 err;
	u8 *recv_buf = NULL;
	u16 recv_len = 0;
    printf("\n***********************blefs_com_task********************\n");
	int i=0;
    while (1)
    {
		memset(msg,0x00,sizeof(msg));
        os_taskq_pend(0, ARRAY_SIZE(msg), msg);
        printf("get_msg, 0x%x\n", msg[0]);
        clear_wdt();

        switch (msg[0])
        {
		case MSG_BLEFS_PIXEL_DATA_RECV:
			recv_buf = (u8*)msg[1];
			recv_len = (u16)msg[2];
			printf("MSG_BLE_PIXEL_DATA_RECV len = %d\n",recv_len);
			printf_buf(recv_buf,recv_len);
			printf("sp_disp_buf addr = 0x%X\n",sp_disp_buf);
			copy_to_disp_buf(recv_buf,recv_len);
			break;

		case MSG_BLEFS_REPORT_SCREEN_RESOLUTION:
		    blefs_deg("MSG_BLE_REPORT_SCREEN_RESOLUTION\n");
			report_screen_resolution();
			break;

        case MSG_DISP_BASE:
            printf("CBW_DISP_BASE = %d\n", msg[1]);
            break;

        case MSG_DISP_STYLE:
            printf("MSG_DISP_STYLE = %d\n", msg[1]);
            break;

        case MSG_DISP_SPEED:
            printf("MSG_DISP_STYLE = %d\n", msg[1]);
			break;

		case MSG_BLEFS_SEND_CSW:
            blefs_csw_send((u16)msg[1]);
            break;

		case SYS_EVENT_DEL_TASK:
		    blefs_deg("SYS_EVENT_DEL_TASK\n");
			if (os_task_del_req_name(OS_TASK_SELF) == OS_TASK_DEL_REQ)
			{
				blefs_deg("blefs:DEL_TASK\n");
				os_task_del_res_name(OS_TASK_SELF);
			}

			break;

		default:
			break;
        }
    }
}


#endif




