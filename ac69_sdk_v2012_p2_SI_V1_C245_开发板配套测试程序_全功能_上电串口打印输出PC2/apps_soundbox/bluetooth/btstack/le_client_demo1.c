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

#include "bluetooth/ble_api.h"

enum {
    ACC_IDLE,
    ACC_W4_SERVICE_RESULT,
    ACC_W4_READ_RESULT,
    ACC_W4_DATA,
} ;

#define CLI_FUNCTION    printf("@-%s\n",__FUNCTION__);

static u8 set_state_idle = 0;
static u8 acc_state;
static u16 client_conn_handle;

static u32 test_send_all_count = 0;
static u32 test_send_count = 0;
static u32 test_record_timer = 0;

static const u16 acc_service_uuid = 0xAE00;

static const u16 acc_chr_write_uuid = 0xAE01;
static const u16 acc_chr_notify_uuid = 0xAE02;

static const u16 acc_chr_write_uuid2 = 0xAE20;
static const u16 acc_chr_notify_uuid2 = 0xAE21;

u16 acc_chr_write_handle = 0;
u16 acc_chr_notify_handle = 0;

u16 acc_chr_write_handle2 = 0;
u16 acc_chr_notify_handle2 = 0;

u32 client_debug_all;

static void *ble_cbk_priv = NULL;
static ble_chl_recieve_cbk_t ble_recieve_cbk = NULL;
static ble_md_msg_cbk_t ble_md_msg_cbk = NULL;


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


void test_client_send_all_info(void)
{
    if (test_send_all_count) {
        printf("send all: %d\n", test_send_all_count);
    }

}

static void check_set_send_state(void)
{
    if (set_state_idle == 0) {
        if (acc_state == ACC_W4_DATA) {
            /* os_taskq_post(BTMSG_TASK_NAME, 1, MSG_TEST_BLE_RATE); */
            set_state_idle = 1;
        }
    }
}

static void check_test_time_rate(void)
{
    if ((get_sys_2msCount() - test_record_timer) >= 500) {
        printf("\n client: %d (Bps)\n", test_send_count);
        test_record_timer = get_sys_2msCount();
        test_send_count = 0;
    }
}


//ble send data
u32 client_data_send(u8 *data, u16 len)
{
    int ret_val;

    if (acc_chr_write_handle == NULL) {
        // not connect
        return 4;//
    }

    ret_val = client_cmd_ctrol(4, CLI_CMD_WRITE_NO_REQUEST, acc_chr_write_handle, data, len);

    if (ret_val !=  0) {
        if (ret_val == CLI_RESULT_BTSTACK_BUSY) {
            puts("\n buf_w_full,try!\n");
            return ret_val;//busy
        } else {
            //others reason
            puts("\n app_w_fail!\n");
            return 4;
        }
    }
    return 0;
}

//ble recieve data
static void client_data_recieve(u8 *data, u16 len)
{
    if (ble_recieve_cbk) {
        ble_recieve_cbk(ble_cbk_priv, 0, data, len);
    }

    printf("\n client_rdata(%d):", len);
    put_buf(data, len);
}

u32 client_data_send1(u8 *data, u16 len)
{
    int ret_val;

    if (acc_chr_write_handle2 == NULL) {
        // not connect
        return 4;//
    }

    /* check_test_time_rate(); */
    /* set_state_idle = 0;  */


    ret_val = client_cmd_ctrol(4, CLI_CMD_WRITE_NO_REQUEST, acc_chr_write_handle2, data, len);

    if (ret_val !=  0) {
        if (ret_val == CLI_RESULT_BTSTACK_BUSY) {
            puts("\n buf_w2_full,try!\n");
            return ret_val;//busy
        } else {
            //others reason
            puts("\n app_w2_fail!\n");
            return 4;
        }
    }

    test_send_count += len;
    test_send_all_count += len;
    return 0;
}


//ble recieve data
static void client_data_recieve1(u8 *data, u16 len)
{
    if (ble_recieve_cbk) {
        ble_recieve_cbk(ble_cbk_priv, 1, data, len);
    }

    /* printf("\n client_tone_rdata(%d):", len); */
    /* put_buf(data, len); */
}




static void parse_search_characteristic(report_characteristic_t *charact)
{
    CLI_FUNCTION
    /* printf("value_handle,uuid16,properties: %x,%x,%x\n",charact->value_handle,charact->uuid16,charact->properties); */
    if (charact->uuid16 == 0) {
        puts("uuid128:");
        put_buf(charact->uuid128, 16);
    }

    switch (charact->uuid16) {
    case acc_chr_write_uuid:
        puts("charct 001\n");
        acc_chr_write_handle = charact->value_handle;
        break;

    case acc_chr_write_uuid2:
        puts("charct 002\n");
        acc_chr_write_handle2 = charact->value_handle;
        break;

    case acc_chr_notify_uuid:
        puts("charct 003\n");
        acc_chr_notify_handle = charact->value_handle;
        break;

    case acc_chr_notify_uuid2:
        puts("charct 004\n");
        acc_chr_notify_handle2 = charact->value_handle;
        break;

    default:
        break;

    }
}

static void parse_report_att_data(att_data_report_t *report)
{
    /* CLI_FUNCTION */

    /* printf("value_handle,value_offset: %x,%x",report->value_handle,report->value_offset); */

	client_debug_all +=  report->blob_length;
	/* printf("\n--report_data:%d\n",client_debug_all); */

    if (report->value_handle == acc_chr_notify_handle) {
        /* puts("\n data_rec:"); */
        client_data_recieve(report->blob, report->blob_length);
    } else if (report->value_handle == acc_chr_notify_handle2) {

        /* puts("\n tone_rec:"); */
        client_data_recieve1(report->blob, report->blob_length);

    }
    /* put_buf(report->blob,report->blob_length); */

}



static void app_client_packet_handler(u16 event_type, u16 channel, u8 *packet, u16 size)
{
    switch (event_type) {
    case PK_CLI_NOTIFY_DATA:
        parse_report_att_data((void *)packet);
        break;

    case PK_CLI_INDICATE_DATA:
        parse_report_att_data((void *)packet);
        break;

    case PK_CLI_READ_DATA:
        parse_report_att_data((void *)packet);
        break;

    case PK_CLI_SEARCH_SERVICES_RESPOND:
        parse_search_characteristic((void *)packet);
        break;

    case PK_CLI_SEARCH_SERVICES_COMPLETE:
        acc_state = ACC_W4_DATA;
        puts("PK_CLI_SEARCH_SERVICES_COMPLETE\n");
        test_record_timer = get_sys_2msCount();
        /* check_set_send_state();						 */
        set_state_idle =  1;
        /* os_taskq_post(BTMSG_TASK_NAME, 1, MSG_TEST_BLE_RATE); */
        break;

    default:
        break;

    }
}


static u8 create_conn_flag = 0;
static u8 create_conn_remoter[7];
/* static const u8 JL_manufacturer_data[] = "JIELI"; */
void client_report_adv_data(adv_report_t *report_pt, u16 len)
{
    u8 i, lenght, ad_type;
    u8 *adv_data_pt;

    /* printf("event_type,addr_type: %x,%x; ",report_pt->event_type,report_pt->address_type); */
    /* put_buf(report_pt->address,6); */
    /* puts("adv_data_display:"); */
    /* put_buf(adv_pt->data,adv_pt->length); */

    adv_data_pt = report_pt->data;
    for (i = 0; i < report_pt->length;) {
        if (*adv_data_pt == 0) {
            puts("analyze end\n");
            break;
        }

        lenght = *adv_data_pt++;
        ad_type = *adv_data_pt++;

        i += (lenght + 1);

        switch (ad_type) {
        case 1:
            puts("flags:");
            break;
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            puts("service uuid:");
            if ((adv_data_pt[0] == 0x20) && (adv_data_pt[1] == 0xAE)) {
                
				create_conn_flag = 1;
            	create_conn_remoter[0] = report_pt->address_type;
				memcpy(&create_conn_remoter[1],report_pt->address,6);
			}
            break;
        case 8:
        case 9:
            puts("local name:");
            break;

        case 0xff:
            puts("Manufacturer data:");
            /* if(memcmp(adv_data_pt,(void*)JL_manufacturer_data,5) == 0) */
            /* { */
            /* }	 */
            break;
        default:
            puts("unknow ad_type:");
            break;
        }
        put_buf(adv_data_pt, lenght - 1);
        adv_data_pt += (lenght - 1);
    }

    if (create_conn_flag) {
        puts("******creat_connection...\n");
		create_conn_flag = 0;
		printf_buf(create_conn_remoter,7);
        client_cmd_ctrol(3, CLI_CMD_CREAT_CONNECTION, &create_conn_remoter[1],create_conn_remoter[0]);
    }
}

static void app_client_le_msg_callback(u16 msg, u8 *buffer, u16 buffer_size)
{
    /* printf("\n-%s, msg= 0x%x\n",__FUNCTION__,msg); */
    switch (msg) {
    case BTSTACK_LE_MSG_DAEMON_SENT:
        /* check_set_send_state();						 */
        break;

    case BTSTACK_LE_MSG_CONNECT_COMPLETE:
        client_conn_handle = buffer[0] + (buffer[1] << 8);
        printf("conn_handle= 0x%04x\n", client_conn_handle);
        client_cmd_ctrol(3, CLI_CMD_SEARCH_SERVICES, PFL_SERVER_UUID16, acc_service_uuid);
        acc_state = ACC_W4_SERVICE_RESULT;
        test_send_count = 0;
        test_send_all_count = 0;
        create_conn_flag = 0;
        if (ble_md_msg_cbk) {
            ble_md_msg_cbk(ble_cbk_priv, BTSTACK_LE_MSG_CONNECT_COMPLETE, 0, 0);
        }
		client_debug_all = 0;
        break;

    case BTSTACK_LE_MSG_DISCONNECT_COMPLETE:
        printf("disconn_handle= 0x%04x\n", buffer[0] + (buffer[1] << 8));

        acc_state = ACC_IDLE;
        client_conn_handle = 0;
        acc_chr_write_handle = 0;
        acc_chr_notify_handle = 0;
        acc_chr_write_handle2 = 0;
        acc_chr_notify_handle2 = 0;

        /* client_cmd_ctrol(2,CLI_CMD_SCAN_ENABLE,1); */
        if (ble_md_msg_cbk) {
            ble_md_msg_cbk(ble_cbk_priv, BTSTACK_LE_MSG_DISCONNECT_COMPLETE, &buffer[2], 1);
        }
        break;

    case BTSTACK_LE_MSG_INIT_COMPLETE:
		/* ble_client_creat_connect(); */
        break;

    case BTSTACK_LE_MSG_ADV_REPORT:
        client_report_adv_data((void *)buffer, buffer_size);
        break;

	case BTSTACK_LE_MSG_CONNECT_FAIL:
        puts("conn_fail\n");
		ble_client_creat_connect();
		break;

    default:
        break;
    }

}

extern void set_ble_config_info(u32 config);
void app_client_init(void)
{
    printf("\n%s\n", __FUNCTION__);
    acc_state = ACC_IDLE;
    client_conn_handle = 0;
    ble_send_mutex_init();
    ble_wait_mutex_init();
    set_ble_config_info(GAP_INIT_CFG_MASTER);
    client_register_app_callback(app_client_le_msg_callback, app_client_packet_handler);
}


void client_disconnect_send(void)
{
    if (client_conn_handle) {
        client_cmd_ctrol(2, CLI_CMD_DISCONNECT, client_conn_handle);
    }
}



bool ble_client_check_connect_complete(void)
{
    if ((client_conn_handle != 0)
        && (acc_state == ACC_W4_DATA)) {
        return TRUE;
    }
    return FALSE;
}

bool ble_client_creat_connect(void)
{
    client_cmd_ctrol(2, CLI_CMD_SCAN_ENABLE, 1);
    return TRUE;
}

bool ble_client_dis_connect(void)
{
    if (create_conn_flag) {
        client_cmd_ctrol(1, CLI_CMD_CREAT_CANNEL);
    } else if (client_conn_handle) {
        client_cmd_ctrol(2, CLI_CMD_DISCONNECT, client_conn_handle);
        client_conn_handle = 0;
    } else {
        client_cmd_ctrol(2, CLI_CMD_SCAN_ENABLE, 0);
    }
    return TRUE;
}

void ble_client_regiest_callback(ble_chl_recieve_cbk_t recieve_cbk, ble_md_msg_cbk_t msg_cbk, void *priv)
{
    ble_cbk_priv = priv;
    ble_recieve_cbk = recieve_cbk;
    ble_md_msg_cbk = msg_cbk;
}



int ble_client_data_send(u16 chl, u8 *data, u16 len)
{
    int res = 0;//sucess
    ble_send_mutex_enter();
    ble_wait_mutex_enter();
    ble_send_mutex_exit();
    if (chl == 0) {
        res = client_data_send(data, len);
    } else if (chl == 1) {
        res = client_data_send1(data, len);
    } else {
        res = -1;
    }
    ble_wait_mutex_exit();
    return res;
}


static const u8 test_client_string0[] = "11111111111111111111111";
static const u8 test_client_string1[] = "2222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222";

static u8 test_proc_step = 0;
void ble_client_test_process(void)
{
    switch (test_proc_step) {
    case 0:
        ble_client_creat_connect();
        test_proc_step++;
        break;
    case 1:
        if (ble_client_check_connect_complete()) {
            ble_client_data_send(0, (void *)test_client_string0, 16);
            ble_client_data_send(1, (void *)test_client_string1, 64);
        }
        break;
    case 2:
        break;
    default:
        break;

    }
}





#endif
