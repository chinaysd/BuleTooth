#ifndef __LETV_CLIENT_H__
#define __LETV_CLIENT_H__

#include "typedef.h"

#define CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE      8
#define BLE_HOST_CLIENT_READY                       0xff

void btstack_le_main(void);
// APP func
bool get_adv_type_uuid16_if_hid(u8 *packet);
bool get_adv_if_direct_reconnect(u8 *packet, u8 *reconnect_addr);
bool get_adv_if_assign_name(u8 *packet, const char *name);
u16  get_keyboard_hid_report_input_handle(void);
void letv_client_init(void *notify_callback, void *event_callback);
void letv_client_scan_control(bool enable);
void letv_client_connect_control(u8 *adv_public_addr);
void letv_client_reconnect_init(u8 *notify_info);

#endif

