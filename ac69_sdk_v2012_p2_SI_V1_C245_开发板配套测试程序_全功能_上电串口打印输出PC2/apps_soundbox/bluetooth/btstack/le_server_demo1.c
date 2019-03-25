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
#include "rtos/os_api.h"
#include "timer.h"
#include "rtos/task_manage.h"

#if(BLE_BREDR_MODE&BT_BLE_EN)

#include <stdint.h>
#include "bluetooth/le_server_module.h"


const uint8_t profile_data[] = {
    // 0x0001 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x00, 0x18,
    // 0x0002 CHARACTERISTIC-GAP_DEVICE_NAME-READ | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x02, 0x03, 0x00, 0x00, 0x2a,
    // 0x0003 VALUE-GAP_DEVICE_NAME-READ | DYNAMIC-''
    0x08, 0x00, 0x02, 0x01, 0x03, 0x00, 0x00, 0x2a,

    // 0x0004 PRIMARY_SERVICE-GATT_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x04, 0x00, 0x00, 0x28, 0x01, 0x18,
    // 0x0005 CHARACTERISTIC-GATT_SERVICE_CHANGED-READ
    0x0d, 0x00, 0x02, 0x00, 0x05, 0x00, 0x03, 0x28, 0x02, 0x06, 0x00, 0x05, 0x2a,
    // 0x0006 VALUE-GATT_SERVICE_CHANGED-READ-''
    0x08, 0x00, 0x02, 0x00, 0x06, 0x00, 0x05, 0x2a,

    // 0x0007 PRIMARY_SERVICE-ae00
    0x0a, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x28, 0x00, 0xae,
    // 0x0008 CHARACTERISTIC-ae01-WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x04, 0x09, 0x00, 0x01, 0xae,
    // 0x0009 VALUE-ae01-WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    0x08, 0x00, 0x04, 0x01, 0x09, 0x00, 0x01, 0xae,
    // 0x000a CHARACTERISTIC-ae02-NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x03, 0x28, 0x10, 0x0b, 0x00, 0x02, 0xae,
    // 0x000b VALUE-ae02-NOTIFY-''
    0x08, 0x00, 0x10, 0x00, 0x0b, 0x00, 0x02, 0xae,
    // 0x000c CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x0c, 0x00, 0x02, 0x29, 0x00, 0x00,
    // 0x000d CHARACTERISTIC-ae20-WRITE_WITHOUT_RESPONSE | DYNAMIC
    0x0d, 0x00, 0x02, 0x00, 0x0d, 0x00, 0x03, 0x28, 0x04, 0x0e, 0x00, 0x20, 0xae,
    // 0x000e VALUE-ae20-WRITE_WITHOUT_RESPONSE | DYNAMIC-''
    0x08, 0x00, 0x04, 0x01, 0x0e, 0x00, 0x20, 0xae,
    // 0x000f CHARACTERISTIC-ae21-NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x0f, 0x00, 0x03, 0x28, 0x10, 0x10, 0x00, 0x21, 0xae,
    // 0x0010 VALUE-ae21-NOTIFY-''
    0x08, 0x00, 0x10, 0x00, 0x10, 0x00, 0x21, 0xae,
    // 0x0011 CLIENT_CHARACTERISTIC_CONFIGURATION
    0x0a, 0x00, 0x0a, 0x01, 0x11, 0x00, 0x02, 0x29, 0x00, 0x00,
    // END
    0x00, 0x00,
}; // total size 110 bytes


//
// list mapping between characteristics and handles
//
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_VALUE_HANDLE 0x0006
#define ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE 0x0009
#define ATT_CHARACTERISTIC_ae02_01_VALUE_HANDLE 0x000b
#define ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE 0x000c
#define ATT_CHARACTERISTIC_ae20_01_VALUE_HANDLE 0x000e
#define ATT_CHARACTERISTIC_ae21_01_VALUE_HANDLE 0x0010
#define ATT_CHARACTERISTIC_ae21_01_CLIENT_CONFIGURATION_HANDLE 0x0011


// DYNAMIC, read handle list
const uint16_t gatt_read_callback_handle_list[] = {
    ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE,
    // END
    0xffff
}; //0xffff is end

// DYNAMIC, write handle list
const uint16_t gatt_write_callback_handle_list[] = {
    ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE,
    ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE,
    ATT_CHARACTERISTIC_ae20_01_VALUE_HANDLE,
    ATT_CHARACTERISTIC_ae21_01_CLIENT_CONFIGURATION_HANDLE,
    // END
    0xffff
};//0xffff is end

static const u8 profile_adv_ind_data[31] = {
    0x02, 0x01, 0x06,
    0x04, 0x0d, 0x00, 0x05, 0x10,
    0x03, 0x03, 0x20, 0xae,
    0x09, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,

};


static uint16_t server_send_handle;//ATT 发送handle
static uint16_t server_tone_handle;//ATT 发送handle

uint16_t ble_conn_handle;//设备连接handle
static u8 ble_mutex_flag;// ble与spp 互斥标记,0:表示ble可连接，1：表示ble不可连接

static u32 test_recieve_all_count = 0;
static u32 test_recieve_count = 0;
static u32 test_record_timer = 0;

static void *ble_cbk_priv = NULL;
static ble_chl_recieve_cbk_t ble_recieve_cbk = NULL;
static ble_md_msg_cbk_t ble_md_msg_cbk = NULL;

static u32 server_data_send(u8 *data, u16 len);
static void server_data_recieve(u8 *data, u16 len);
static void server_tone_data_recieve(u8 *data, u16 len);

u32  server_debug_all;

static OS_MUTEX ble_send_mutex;
static inline int ble_send_mutex_init(void)
{
    return os_mutex_create(&ble_send_mutex);
}
static inline int ble_send_mutex_enter(void)
{
    os_mutex_pend(&ble_send_mutex, 0);
    return 0;
}
static inline int ble_send_mutex_exit(void)
{
    return os_mutex_post(&ble_send_mutex);
}
static inline int ble_send_mutex_del(void)
{
    return os_mutex_del(&ble_send_mutex, OS_DEL_ALWAYS);
}


static OS_MUTEX ble_wait_mutex;
static inline int ble_wait_mutex_init(void)
{
    return os_mutex_create(&ble_wait_mutex);
}
static inline int ble_wait_mutex_enter(void)
{
    os_mutex_pend(&ble_wait_mutex, 0);
    return 0;
}
static inline int ble_wait_mutex_exit(void)
{
    return os_mutex_post(&ble_wait_mutex);
}
static inline int ble_wait_mutex_del(void)
{
    return os_mutex_del(&ble_wait_mutex, OS_DEL_ALWAYS);
}




void test_server_recieve_all_info(void)
{
    if (test_recieve_all_count) {
        printf("recieve all: %d\n", test_recieve_all_count);
    }

}


static void check_test_time_rate(void)
{
    if ((get_sys_2msCount() - test_record_timer) >= 500) {
        printf("\n recieve: %d (Bps)\n", test_recieve_count);
        test_record_timer = get_sys_2msCount();
        test_recieve_count = 0;
    }
}

//ble send data
u32 server_data_send(u8 *data, u16 len)
{
    int ret_val;

    if (server_send_handle == NULL) {
        // not connect
        return 4;//
    }

    ret_val = server_notify_indicate_send(server_send_handle, data, len);

    if (ret_val !=  0) {
        if (ret_val == 0x57) {
            puts("\n buf is full,try again!\n");
            return 0x57;//busy
        } else {
            //others reason
            puts("\n app_ntfy_fail!\n");
            return 4;
        }
    }
    return 0;
}

//ble recieve data
static void server_data_recieve(u8 *data, u16 len)
{
    if (ble_recieve_cbk) {
        ble_recieve_cbk(ble_cbk_priv, 0, data, len);
    }

    printf("\n server_rdata(%d):", len);
    put_buf(data, len);
}

//ble send data
u32 server_data_send1(u8 *data, u16 len)
{
    int ret_val;

    if (server_tone_handle == NULL) {
        // not connect
        return 4;//
    }

    ret_val = server_notify_indicate_send(server_tone_handle, data, len);

    if (ret_val !=  0) {
        if (ret_val == 0x57) {
            puts("\n tone is full,try again!\n");
            return 0x57;//busy
        } else {
            //others reason
            puts("\n tone_ntfy_fail!\n");
            return 4;
        }
    }
    return 0;
}

//ble recieve data
static void server_data_recieve1(u8 *data, u16 len)
{
    /* test_recieve_count += len; */
    /* test_recieve_all_count += len; */
    /* check_test_time_rate();	 */

    if (ble_recieve_cbk) {
        ble_recieve_cbk(ble_cbk_priv, 1, data, len);
    }

    printf("\n server_tone_rdata(%d):", len);
    put_buf(data, len);
}


void app_server_le_msg_callback(uint16_t msg, uint8_t *buffer, uint16_t buffer_size)
{
    /* printf("\n-%s, msg= 0x%x\n",__FUNCTION__,msg); */
    switch (msg) {
    case BTSTACK_LE_MSG_CONNECT_COMPLETE:
        ble_conn_handle = buffer[0] + (buffer[1] << 8);
        printf("\n-server-conn_handle= 0x%04x\n", ble_conn_handle);
        test_recieve_all_count = 0;
        test_recieve_count = 0;
        if (ble_md_msg_cbk) {
            ble_md_msg_cbk(ble_cbk_priv, BTSTACK_LE_MSG_CONNECT_COMPLETE, 0, 0);
        }
        server_debug_all = 0;
		break;

    case BTSTACK_LE_MSG_DISCONNECT_COMPLETE:
        printf("\n-server-disconn_handle= 0x%04x\n", buffer[0] + (buffer[1] << 8));
        server_send_handle = 0;
        server_tone_handle = 0;

        if (ble_conn_handle != 0) {
            ble_conn_handle = 0;
            if (ble_mutex_flag == 0) {
                /* server_advertise_enable(); */
            }
        }

        if (ble_md_msg_cbk) {
            ble_md_msg_cbk(ble_cbk_priv, BTSTACK_LE_MSG_DISCONNECT_COMPLETE, &buffer[2], 1);
        }

        break;

    case BTSTACK_LE_MSG_INIT_COMPLETE:
        /* puts("init server_adv\n"); */
        /* server_advertise_enable(); */
        break;
	case BTSTACK_LE_MSG_CONNECT_FAIL:
        puts("server_conn_fail\n");
		ble_server_creat_connect();
		break;
    default:
        break;
    }

    /* puts("exit_le_msg_callback\n"); */
}


// ATT Client Write Callback for Dynamic Data
// @param attribute_handle to be read
// @param buffer
// @param buffer_size
// @return 0
uint16_t app_server_write_callback(uint16_t attribute_handle, uint8_t *buffer, uint16_t buffer_size)
{

    u16 handle = attribute_handle;
    u16 copy_len;

#if 0
    if (buffer_size > 0) {
        printf("\n write_cbk: handle= 0x%04x", handle);
        put_buf(buffer, buffer_size);
    }
#endif

    switch (handle) {
    case ATT_CHARACTERISTIC_ae02_01_CLIENT_CONFIGURATION_HANDLE:
        test_record_timer = get_sys_2msCount();
        if (buffer[0] != 0) {
            server_send_handle = handle - 1;
        } else {
            server_send_handle = 0;
        }
        break;

    case ATT_CHARACTERISTIC_ae21_01_CLIENT_CONFIGURATION_HANDLE:
        if (buffer[0] != 0) {
            server_tone_handle = handle - 1;
        } else {
            server_tone_handle = 0;
        }
        break;


    case ATT_CHARACTERISTIC_ae01_01_VALUE_HANDLE:

        server_data_recieve(buffer, buffer_size);

        break;

    case ATT_CHARACTERISTIC_ae20_01_VALUE_HANDLE:

        server_data_recieve1(buffer, buffer_size);

        break;

    default:
        break;
    }

    return 0;
}


extern void server_select_scan_rsp_data(u8 data_type);
extern void app_advertisements_set_params(uint16_t adv_int_min, uint16_t adv_int_max, uint8_t adv_type,
                                   uint8_t direct_address_typ, u8 *direct_address, uint8_t channel_map, uint8_t filter_policy);
extern void	app_connect_set_params(uint16_t interval_min,uint16_t interval_max,uint16_t supervision_timeout);
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
static const u8 JL_manufacturer_data[] = "JIELI";
extern void server_set_advertise_manufacturer_data(u8 *data, u16 len);
extern void server_set_advertise_data(const u8 *data);
extern void server_connection_parameter_update(int enable);
void app_server_init(void)
{
    printf("\n%s\n", __FUNCTION__);
    server_register_profile_data(profile_data);
    server_set_advertise_data(profile_adv_ind_data);
    server_set_advertise_manufacturer_data((void *)JL_manufacturer_data, 6);
    server_register_app_callbak(app_server_le_msg_callback, NULL, app_server_write_callback);
	server_select_scan_rsp_data(0);
	app_set_adv_parm();
	server_connection_parameter_update(0);
	app_set_connect_param();
	s_att_server_register_conn_update_complete(app_server_conn_update_callback);
    ble_mutex_flag = 0;
    ble_send_mutex_init();
    ble_wait_mutex_init();
}

/*
spp 和 ble 互斥连接
1、当spp 连接后，ble变为不可连接
2、当ble连上后，若有spp连接上，则断开ble；ble变为不可连接
 */
void ble_enter_mutex(void)
{

    if (ble_mutex_flag == 1) {
        return;
    }

    puts("in enter mutex\n");
    ble_mutex_flag = 1;
    if (ble_conn_handle != 0) {
        ble_hci_disconnect(ble_conn_handle);
    } else {
        server_advertise_disable();
    }
    puts("exit_enter_mu\n");
}

void ble_exit_mutex(void)
{

    if (ble_mutex_flag == 0) {
        return;
    }

    puts("in_exit mutex\n");
    ble_mutex_flag = 0;
    server_advertise_enable();
    puts("exit_exit_mu\n");
}

void ble_server_disconnect(void)
{
    if (ble_conn_handle != 0) {
        printf("server discon handle= 0x%x\n ", ble_conn_handle);
        ble_hci_disconnect(ble_conn_handle);
    }
    puts("exit_discnt\n");
}


bool ble_server_check_connect_complete(void)
{

    if ((ble_conn_handle != 0)
        && (server_send_handle != NULL)
        && (server_tone_handle != NULL)) {
        return TRUE;
    }
    return FALSE;
}

bool ble_server_creat_connect(void)
{
    printf("fun = %s, line = %d\n", __func__, __LINE__);
    server_advertise_enable();
    return TRUE;
}

bool ble_server_dis_connect(void)
{
    if (ble_conn_handle) {
        ble_server_disconnect();
    } else {
        server_advertise_disable();
    }
    return TRUE;
}

void ble_server_regiest_callback(ble_chl_recieve_cbk_t recieve_cbk, ble_md_msg_cbk_t msg_cbk, void *priv)
{
    ble_cbk_priv = priv;
    ble_recieve_cbk = recieve_cbk;
    ble_md_msg_cbk = msg_cbk;
}

int ble_server_data_send(u16 chl, u8 *data, u16 len)
{
    int res = 0;//sucess
	u16 cnt;

    ble_send_mutex_enter();
    ble_wait_mutex_enter();
    ble_send_mutex_exit();

	server_debug_all +=  len;
	/* printf("\n--send_data:%d, %d\n",len,server_debug_all); */

	/* for(cnt = 0;cnt <len;cnt++) */
	/* { */
		/* data[cnt] = (u8)cnt;	 */
	/* } */

	/* memset(data,0x55,len); */

    if (chl == 0) {
        res = server_data_send(data, len);
    } else if (chl == 1) {
        res = server_data_send1(data, len);
    } else {
        res = -1;
    }
    ble_wait_mutex_exit();
    return res;
}

const u8 test_string0[] = "44444444444444444444444444444444444";
const u8 test_string1[] = "555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555";

static u8 test_proc_step = 0;
void ble_server_test_process(void)
{
    switch (test_proc_step) {
    case 0:
        ble_server_creat_connect();
        test_proc_step++;
        break;
    case 1:
        if (ble_server_check_connect_complete()) {
            ble_server_data_send(0, (void *)test_string0, 16);
            ble_server_data_send(1, (void *)test_string1, 64);
        }
        break;
    case 2:
        break;
    default:
        break;

    }
}



#endif //#if (BLE_BREDR_MODE&BT_BLE_EN)
