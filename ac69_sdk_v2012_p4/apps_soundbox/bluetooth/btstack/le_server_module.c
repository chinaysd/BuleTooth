/*
 *********************************************************************************************************
 *                                            br16
 *                                            btstack
 *                                             CODE
 *
 *                          (c) Copyright 2016-2016, ZHUHAI JIELI
 *                                           All Rights Reserved
 *
 * File : *
 * By   : jamin.li
 * DATE : 2016-04-12 10:17AM    build this file
 *********************************************************************************************************
 */
#include "typedef.h"
//#include "error.h"
#include "sdk_cfg.h"
#include "common/msg.h"
#include "common/app_cfg.h"
#include "bluetooth/le_profile_def.h"
#include "bluetooth/le_server_module.h"
#include "bluetooth/le_finger_spinner.h"
#include "rtos/os_api.h"

#include "rtos/task_manage.h"
#include "rcsp/rcsp_interface.h"
#include "rcsp/rcsp_head.h"


#if(BLE_BREDR_MODE&BT_BLE_EN)


static uint16_t server_send_handle;//ATT 发送handle

uint16_t ble_conn_handle;//设备连接handle
static u8 ble_mutex_flag;// ble与spp 互斥标记,0:表示ble可连接，1：表示ble不可连接

static u32 app_data_send(u8 *data,u16 len);
static void app_data_recieve(u8 *data, u16 len);

void app_server_le_msg_callback(uint16_t msg, uint8_t *buffer, uint16_t buffer_size)
{
    /* printf("\n-%s, msg= 0x%x\n",__FUNCTION__,msg); */
    switch(msg)
    {
    case BTSTACK_LE_MSG_CONNECT_COMPLETE:
        ble_conn_handle = buffer[0]+ (buffer[1]<<8);
        printf("conn_handle= 0x%04x\n",ble_conn_handle);

#if SUPPORT_APP_RCSP_EN
        rcsp_event_com_start(RCSP_APP_TYPE_BLE);
        rcsp_register_comsend(app_data_send);
        
#elif  BLE_FINGER_SPINNER_EN
        blefs_com_init();
        blefs_register_comsend(app_data_send);
#endif // SUPPORT_APP_RCSP_EN

		break;

    case BTSTACK_LE_MSG_DISCONNECT_COMPLETE:
        printf("disconn_handle= 0x%04x\n",buffer[0]+ (buffer[1]<<8));
        server_send_handle = 0;

#if BLE_FINGER_SPINNER_EN
        blefs_com_stop();
#endif
        if(ble_conn_handle != 0)
        {
            ble_conn_handle = 0;

#if SUPPORT_APP_RCSP_EN
            rcsp_event_com_stop();
#endif // SUPPORT_APP_RCSP_EN

			if(ble_mutex_flag == 0)
            {
                server_advertise_enable();
            }
        }

        break;

	case BTSTACK_LE_MSG_INIT_COMPLETE:
        puts("init server_adv\n");
		server_advertise_enable();
		break;

    default:
        break;
    }

    /* puts("exit_le_msg_callback\n"); */
}


#if BLE_APP_UPDATE_SUPPORT_EN
extern void bt_exit_low_power_mode(void);
static u8 ble_app_update_flag = 0;
u8 get_ble_app_update_flag(void)
{
	return ble_app_update_flag;		
}

void ble_send_update_cmd()
{  
    puts("-------ble_update_run\n");
	ble_app_update_flag = 1;
    os_taskq_post_msg(MAIN_TASK_NAME,2,MSG_UPDATA,BLE_UPDATE);
}  

extern u32 ble_update_save_att_server_info(const u8 *profile_data,u16 profile_len,u16 write_handle,u16 read_handle);
static void ble_update_save_att_server_info_handle(void)
{
	printf("profile_data size:%d\n",sizeof(profile_data));
	u8 ble_update_write_handle = ATT_CHARACTERISTIC_00001532_1212_EFDE_1523_785FEABCD123_01_VALUE_HANDLE; 
	u8 ble_update_notify_handle = ATT_CHARACTERISTIC_00001531_1212_EFDE_1523_785FEABCD123_01_VALUE_HANDLE;	
	ble_update_save_att_server_info(profile_data,sizeof(profile_data),ble_update_write_handle,ble_update_notify_handle);	
}
#endif
// ATT Client Write Callback for Dynamic Data
// @param attribute_handle to be read
// @param buffer
// @param buffer_size
// @return 0
uint16_t app_server_write_callback(uint16_t attribute_handle, uint8_t * buffer, uint16_t buffer_size)
{

    u16 handle = attribute_handle;
    u16 copy_len;

#if 0
    if(buffer_size > 0)
    {
        printf("\n write_cbk: handle= 0x%04x",handle);
        put_buf(buffer,buffer_size);
    }
#endif

    switch (handle)
    {
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
        if(buffer[0] != 0)
        {
            server_send_handle = handle -1;
        }
        else
        {
            server_send_handle = 0;
        }
        break;


    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:

        /* printf("\n--write, %d\n",buffer_size); */
        /* put_buf(buffer,buffer_size); */
		app_data_recieve(buffer,buffer_size);

        break;

#if BLE_APP_UPDATE_SUPPORT_EN
	case ATT_CHARACTERISTIC_00001531_1212_EFDE_1523_785FEABCD123_01_CLIENT_CONFIGURATION_HANDLE:
		if(buffer[0] != 0)
		{
			ble_send_update_cmd();
		}
		break;
		case ATT_CHARACTERISTIC_00001531_1212_EFDE_1523_785FEABCD123_01_VALUE_HANDLE: 
		break;

		case ATT_CHARACTERISTIC_00001532_1212_EFDE_1523_785FEABCD123_01_VALUE_HANDLE:
		break;
#endif
    default:
        break;
    }

	return 0;
}

//ble send data
static u32 app_data_send(u8 *data,u16 len)
{
    int ret_val;

	if(0 == server_send_handle)
	{
	    return 4;// is disconn
	}

    ret_val = server_notify_indicate_send(server_send_handle,data,len);

    if(ret_val !=  0)
    {
        if(ret_val == 0x57)
		{
			puts("stack busy!!!\n");
		}
		puts("\n app_ntfy_fail!!!\n");
        return ret_val;
    }
    return 0;
}

extern u8 get_ble_test_key_flag(void);
void ble_server_send_test_key_num(u8 key_num)
{
	if(get_ble_test_key_flag())
	{
		if(!ble_conn_handle)
			return;		

		ble_user_send_test_key_num(ble_conn_handle,key_num);
	}
}
//ble recieve data
static void app_data_recieve(u8 *data, u16 len)
{
#if SUPPORT_APP_RCSP_EN
   rcsp_comdata_recieve(data,len);
#elif BLE_FINGER_SPINNER_EN
   blefs_comdata_parse(data,len);
#endif // SUPPORT_APP_RCSP_EN

} 

extern void server_select_scan_rsp_data(u8 data_type);
extern void server_set_scan_rsp_data(u8 *data);
extern void server_set_advertise_data(const u8 *data);
extern void app_advertisements_set_params(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type,
                                   uint8_t direct_address_typ, u8 *direct_address, uint8_t channel_map, uint8_t filter_policy);
extern void server_connection_parameter_update(int enable);
extern void app_connect_set_params(uint16_t interval_min,uint16_t interval_max,uint16_t supervision_timeout);
extern void s_att_server_register_conn_update_complete(void (*handle)(uint16_t min,uint16_t max,uint16_t timeout));

/*
 * @brief Set Advertisement Paramters
 * @param adv_int_min
 * @param adv_int_max
 * @param adv_type
 * @param direct_address_type
 * @param direct_address
 * @param channel_map
 * @param filter_policy
 *
 * @note own_address_type is used from gap_random_address_set_mode
 */
void app_set_adv_parm(void)
{
// setup advertisements
	uint16_t adv_int_min = 0x0080;
	uint16_t adv_int_max = 0x00a0;
	uint8_t adv_type = 0;
	u8 null_addr[6];
	memset(null_addr, 0, 6);

	app_advertisements_set_params(adv_int_min, adv_int_max, adv_type, 0, null_addr, 0x07, 0x00);
}

//连接参数请求，只能修改3个参数
void app_set_connect_param(void)
{
// setup connect
	uint16_t conn_interval_min = 50;
	uint16_t conn_interval_max = 120;
	uint16_t conn_supervision_timeout = 550;

	app_connect_set_params(conn_interval_min,conn_interval_max,conn_supervision_timeout);
}


void app_server_conn_update_callback(uint16_t min,uint16_t max,uint16_t timeout)
{

	printf("app_min = %d\n",min);
	printf("app_max = %d\n",max);
	printf("timeout= %d\n",timeout);

}

extern void ble_update_save_att_server_info_handle_register(void (*handle)(void));
void app_server_init(void)
{
    printf("\n%s\n",__FUNCTION__);
    server_register_profile_data(profile_data);
    server_set_advertise_data(profile_adv_ind_data);
	server_register_app_callbak(app_server_le_msg_callback,NULL,app_server_write_callback);
	server_select_scan_rsp_data(0);     //scan_rsp类型选择，0：默认，1：会自动填写ble名字，其他自由填写，2：全部内容自由填写
	server_set_scan_rsp_data(profile_scan_rsp_data);//注册scan_rsp包内容
	app_set_adv_parm();//注册广播参数
	server_connection_parameter_update(0);//连接参数使能，0：不修改连接参数，1：修改连接参数
	app_set_connect_param();//注册连接参数
	s_att_server_register_conn_update_complete(app_server_conn_update_callback);//注册连接参数请求回调函数
	ble_mutex_flag = 0;
#if BLE_APP_UPDATE_SUPPORT_EN
	ble_update_save_att_server_info_handle_register(ble_update_save_att_server_info_handle);	
#endif
}

/*
spp 和 ble 互斥连接
1、当spp 连接后，ble变为不可连接
2、当ble连上后，若有spp连接上，则断开ble；ble变为不可连接
 */
void ble_enter_mutex(void)
{
	P_FUNCTION


    if(ble_mutex_flag == 1)
	{
		return;
	}

	puts("in enter mutex\n");
    ble_mutex_flag = 1;
    if(ble_conn_handle != 0)
    {
        ble_hci_disconnect(ble_conn_handle);
        rcsp_event_com_stop();
    }
    else
    {
        server_advertise_disable();
    }
    puts("exit_enter_mu\n");
}

void ble_exit_mutex(void)
{
    P_FUNCTION

	if(ble_mutex_flag == 0)
	{
		return;
	}
   
	puts("in_exit mutex\n");
	ble_mutex_flag = 0;
   	server_advertise_enable();
   	puts("exit_exit_mu\n");
}

void ble_server_disconnect(void)
{
    P_FUNCTION
    if(ble_conn_handle != 0)
    {
        printf("server discon handle= 0x%x\n ",ble_conn_handle); 
		ble_hci_disconnect(ble_conn_handle);
    }
    puts("exit_discnt\n");
}



#endif //#if (BLE_BREDR_MODE&BT_BLE_EN)
