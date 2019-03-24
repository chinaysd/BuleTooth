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

#ifndef _LE_SERVER_MODULE_H_
#define _LE_SERVER_MODULE_H_


#include "bluetooth/ble_api.h"

// ATT Client Read Callback for Dynamic Data
// - if buffer == NULL, don't copy data, just return size of value
// - if buffer != NULL, copy data and return number bytes copied
// @param attribute_handle to be read
// @param buffer
// @param buffer_size
// @return data's len
typedef uint16_t (*app_server_read_cbk_t)(uint16_t attribute_handle, uint8_t * buffer, uint16_t buffer_size);

// ATT Client Write Callback for Dynamic Data
// @param attribute_handle to be read
// @param buffer
// @param buffer_size
// @return 0
typedef uint16_t (*app_server_write_cbk_t)(uint16_t attribute_handle, uint8_t *buffer, uint16_t buffer_size);

//
// bt_stack_le_msg_e


// extern bool server_get_ccc_handle_info(u16 handle,server_ccc_info_t *ccc_info_p);

/*
 * @e returns 0 if ok, error otherwise
 */
extern int server_notify_indicate_send(u16 handle,u8* data,u16 len);
extern int ble_user_send_test_key_num(u16 conn_handle,u8 key_num);
extern void server_register_app_callbak(app_btstack_le_msg_cbk_t le_msg_callback,app_server_read_cbk_t read_callback,app_server_write_cbk_t write_callback);
extern void server_register_profile_data(const u8 *data);

extern void server_advertise_enable(void);
extern void server_advertise_disable(void);
extern void ble_hci_disconnect(u16 conn_handle);


#endif
