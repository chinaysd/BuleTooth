#ifndef __BLE_API_H__ 
#define __BLE_API_H__ 
#include "typedef.h"

typedef void (*app_btstack_le_msg_cbk_t)(u16 msg, u8 *buffer, u16 buffer_size);
typedef void (*ble_client_packet_handler_t) (u16 event_type,u16 channel,u8 *packet, u16 size);

//bit map for gap_le_config 
#define GAP_INIT_CFG_MASTER    	(1<<0)

typedef enum
{
	PK_TYPE_NULL = 0,
	PK_CLI_SEARCH_SERVICES_RESPOND,
	PK_CLI_SEARCH_SERVICES_COMPLETE,
	PK_CLI_WRITE_RESPOND,
	PK_CLI_READ_RESPOND,
	PK_CLI_NOTIFY_DATA,
	PK_CLI_INDICATE_DATA,
	PK_CLI_READ_DATA,

}pk_event_type_e;


typedef enum
{
    BTSTACK_LE_MSG_NULL = 0,
    BTSTACK_LE_MSG_CONNECT_COMPLETE,
    BTSTACK_LE_MSG_DISCONNECT_COMPLETE,
    BTSTACK_LE_MSG_INIT_COMPLETE,
    BTSTACK_LE_MSG_SET_INTERVAL,
    BTSTACK_LE_MSG_CONNECT_FAIL,
    BTSTACK_LE_MSG_ADV_REPORT = 0x20,
    BTSTACK_LE_MSG_DAEMON_SENT = 0x40,
} bt_stack_le_msg_e;

typedef enum
{
	PFL_SERVER_UUID16,
	PFL_SERVER_UUID128,
}search_profile_type_e;

typedef struct
{
	u16  value_handle;
    u16  properties;
	u16  uuid16;
	u8   uuid128[16];
}report_characteristic_t;


typedef struct
{
    u8   event_type;
    u8   address_type;
    u8   address[6];
    u8   reserve;
    u8   length;
    u8   data[1];
} adv_report_t;

typedef struct {
    u16  value_handle;
    u16  value_offset;
    u16  blob_length;
    u8  *blob;
} att_data_report_t;



typedef enum
{
    CLI_CMD_NULL = 0,
    CLI_CMD_SCAN_PARAM,
    CLI_CMD_SCAN_ENABLE,
    CLI_CMD_CREAT_CONNECTION,
    CLI_CMD_CREAT_CANNEL,
    CLI_CMD_DISCONNECT,
    CLI_CMD_SEARCH_SERVICES,
    CLI_CMD_WRITE_REQUEST,
    CLI_CMD_WRITE_NO_REQUEST,
    CLI_CMD_READ_REQUEST,

}client_cmd_ctrl_e;


typedef enum
{
    CLI_RESULT_SUCCESS = 0,
    CLI_RESULT_MALLOC_FAIL,
    CLI_RESULT_DISCONNECT = 0x04,
    CLI_RESULT_BTSTACK_BUSY = 0x57,

}cli_cmd_result_e;

extern cli_cmd_result_e client_cmd_ctrol(int arg_cnt, ...);//arg_cnt

extern void client_register_app_callback(app_btstack_le_msg_cbk_t le_msg_callback,ble_client_packet_handler_t packet_cbk);

/*
#define BLE_EVENT_ANCS_META                                0x10


extern int server_request_pair_process(u16 conn_handle);
extern int server_advertise_set_time(u16 adv_min,u16 adv_max);
extern int server_advertise_set_interval(u16 conn_handle,u16 interval_min,u16 interval_max,u16 timeout);
*/

typedef void (*ble_chl_recieve_cbk_t)(void *priv, u16 channel, u8 *buffer, u16 buffer_size);
typedef void (*ble_md_msg_cbk_t)(void *priv, u16 msg, u8 *buffer, u16 buffer_size);



bool ble_server_check_connect_complete(void);
bool ble_server_creat_connect(void);
bool ble_server_dis_connect(void);
void ble_server_regiest_callback(ble_chl_recieve_cbk_t recieve_cbk, ble_md_msg_cbk_t msg_cbk, void *priv);
int ble_server_data_send(u16 chl, u8 *data, u16 len);


bool ble_client_check_connect_complete(void);
bool ble_client_creat_connect(void);
bool ble_client_dis_connect(void);
void ble_client_regiest_callback(ble_chl_recieve_cbk_t recieve_cbk, ble_md_msg_cbk_t msg_cbk, void *priv);
int ble_client_data_send(u16 chl, u8 *data, u16 len);


#endif
