#include "sdk_cfg.h"
#include "vm/vm_api.h"
#include "bluetooth/letv_client.h"

#define LETV_DEBUG_EN

#ifdef LETV_DEBUG_EN
#define letv_putchar(x)        putchar(x)
#define letv_puts(x)           puts(x)
#define letv_u32hex(x)         put_u32hex(x)
#define letv_u8hex(x)          put_u8hex(x)
#define letv_pbuf(x,y)         printf_buf(x,y)
#define letv_printf            printf
#else
#define letv_putchar(...)
#define letv_puts(...)
#define letv_u32hex(...)
#define letv_u8hex(...)
#define letv_pbuf(...)
#define letv_printf(...)
#endif

#define MUST_DEBUG_EN
#ifdef MUST_DEBUG_EN
#define must_putchar(x)        putchar(x)
#define must_puts(x)           puts(x)
#define must_u32hex(x)         put_u32hex(x)
#define must_u8hex(x)          put_u8hex(x)
#define must_pbuf(x,y)         printf_buf(x,y)
#define must_printf            printf
#else
#define must_putchar(...)
#define must_puts(...)
#define must_u32hex(...)
#define must_u8hex(...)
#define must_pbuf(...)
#define must_printf(...)
#endif

//letv hid report handle
#define HID_REPORT_1ST_ATT_HANDLE       47
#define HID_REPORT_2ND_ATT_HANDLE       54
#define JBL_MIC_DATA_HANDLE             76
#define JBL_BATTERY_LEVEL_HANDLE        92

// this c file use value(global) ram addr
#define __THIS_FILE_VALUE_RAM_ADDR        //SEC(.non_volatile_ram)

// global var
__THIS_FILE_VALUE_RAM_ADDR
static u8 ble_resume_connect_addr[6];
__THIS_FILE_VALUE_RAM_ADDR
static u8 g_reconnect_info[VM_BLE_RECONNECT_INFO_LEN];


#if 0
void mic_16bit_mono_example(u8 *packet, u16 size)
{
#define ONETIME_TRANS_LEN       20
    s16 mic_temp_buf[ONETIME_TRANS_LEN];
    u32 j = 0, k = 1;
    u32 i;

    putchar('&');
    memset((u8 *)mic_temp_buf, 0, sizeof(mic_temp_buf));
    for (i = 0; i < (ONETIME_TRANS_LEN / 2); i++, j += 2, k += 2) {
        mic_temp_buf[j] = ((s16 *)packet)[i];
        mic_temp_buf[k] = ((s16 *)packet)[i];
    }
    dac_write((u8 *)mic_temp_buf, sizeof(mic_temp_buf));
}
#endif

static void lose_packet_test(u8 *packet)
{
    u8 mic_start_string[] = {
            0x6E, 0x61, 0x6E, 0x6F, 0x73, 0x69, 0x63, 0x20, 0x76, 0x6F, 0x69, 0x63, 0x65, 0x20, 0x73, 0x74, 
            0x61, 0x72, 0x74, 0x20
    };
    static bool start_flag_aa55 = 0;
    static bool start_flag_55 = 0;
    static u8 per_num_aa55 = 0;
    static u8 per_num_55 = 0;
    u8 cur_num = 0;

    if(!memcmp(mic_start_string, packet, sizeof(mic_start_string))) {
        start_flag_aa55 = 1;
        start_flag_55 = 1;
    }

    u8 start_sync[] = {0xAA, 0x55};
    if(!memcmp(packet, start_sync, sizeof(start_sync))) {
        cur_num = packet[2];
        if(start_flag_aa55) {
            start_flag_aa55 = 0;
        } else {
            if((u8)(cur_num - per_num_aa55) != (u8)1 ) {
                puts("*************************lose packet\n");
                printf("cur_num =0x%x\n", cur_num);
                printf("per_num_aa55 =0x%x\n", per_num_aa55);
                printf("sub=0x%x\n", (u8)(cur_num - per_num_aa55));
                /* while(1); */
            }
        }
        per_num_aa55 = cur_num;
    }
    else if(packet[0] == 0x55) {
        cur_num = packet[1];
        if(start_flag_55) {
            start_flag_55 = 0;
        } else {
            if((u8)(cur_num - per_num_55) != (u8)1 ) {
                puts("*************************lose packet\n");
                printf("cur_num =0x%x\n", cur_num);
                printf("per_num_55 =0x%x\n", per_num_55);
                printf("sub=0x%x\n", (u8)(cur_num - per_num_55));
                /* while(1); */
            }
        }
        per_num_55 = cur_num;
    }
        
}

/* LISTING_START(tracking): Tracking throughput */
#define uint32_t    u32
#define REPORT_INTERVAL_MS 3000
static uint32_t test_data_sent;
static uint32_t test_data_start;
extern volatile u32 g_ble_data_rate_10ms_cnt;

static void test_reset(void)
{
    test_data_start = os_time_get() * 10;
    /* test_data_start = g_ble_data_rate_10ms_cnt * 10; */
    test_data_sent = 0;
}

extern void ll_channel_map_print(void);
static void test_track_sent(int bytes_sent)
{
    test_data_sent += bytes_sent;
    // evaluate
    uint32_t now = os_time_get() * 10;
    /* uint32_t now = g_ble_data_rate_10ms_cnt * 10; */

    uint32_t time_passed = now - test_data_start;
    if (time_passed < REPORT_INTERVAL_MS) {
        return;
    }
    // print speed
    int bytes_per_second = test_data_sent * 1000 / time_passed;
    /* printf("time_passed : %d\n", time_passed); */
    printf("%u bytes sent-> %u.%03u kB/s\n", test_data_sent, bytes_per_second / 1000, bytes_per_second % 1000);
    /* ll_channel_map_print(); */

    // restart
    test_data_start = now;
    test_data_sent  = 0;
}

static void mic_data_rate_test(u8 *packet, u8 size)
{
    u8 mic_start_string[] = {
            0x6E, 0x61, 0x6E, 0x6F, 0x73, 0x69, 0x63, 0x20, 0x76, 0x6F, 0x69, 0x63, 0x65, 0x20, 0x73, 0x74, 
            0x61, 0x72, 0x74, 0x20
    };
    u8 mic_end_string[] = {
            0x6E, 0x61, 0x6E, 0x6F, 0x73, 0x69, 0x63, 0x20, 0x76, 0x6F, 0x69, 0x63, 0x65, 0x20, 0x73, 0x74, 
            0x6F, 0x70, 0x20, 0x20 
    };
    static bool start_flag = 0;

    if(!memcmp(mic_start_string, packet, sizeof(mic_start_string))) {
        puts("--start\n");
        start_flag  = 1;
        test_reset();
        return;
    }
    if(!memcmp(mic_end_string, packet, sizeof(mic_end_string))) {
        puts("--end\n");
        start_flag  = 0;
        return;
    }

    if (start_flag) {
        test_track_sent(size);
    }
}

#define LITTLE_ENDIAN_READ_16(buffer, pos) \
            (((u16) buffer[pos]) | (((u16)buffer[(pos) + 1]) << 8))

static void letv_hid_process(u8 packet_type, u16 channel, u8 *packet, u16 size)
{
    u16 value_handle;
    u8 key_value;
    static u8 notify_cnt = 0;

    /* letv_printf("\n--func=%s\n", __FUNCTION__); */
    value_handle = LITTLE_ENDIAN_READ_16(packet, 4);
    if(JBL_BATTERY_LEVEL_HANDLE == value_handle) {
        return;
    }
    /* if (value_handle != get_keyboard_hid_report_input_handle()) { */
    /*     return; */
    /* } */
    packet += CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE;
    size -= CHARACTERISTIC_VALUE_EVENT_HEADER_SIZE;

    /* letv_printf("value_handle=%d\n", value_handle); */
    /* letv_pbuf(packet, size); */
    if (value_handle == JBL_MIC_DATA_HANDLE) {
        /* mic_16bit_mono_example(packet, size); */
		mic_data_rate_test(packet, size);
        /* lose_packet_test(packet); */
    }

    /* switch (value_handle) { */
    /* case HID_REPORT_1ST_ATT_HANDLE: */
    /*     if (notify_cnt == 0) { */
    /*         notify_cnt++; */
    /*         key_value = *(packet + 3); */
    /*         g_ble_key_num = key_value; */
    /*         #<{(| letv_printf("key_value=%d\n", key_value); |)}># */
    /*     } else { */
    /*         notify_cnt = 0; */
    /*         g_ble_key_num = 0xff; */
    /*     } */
    /*  */
    /* case HID_REPORT_2ND_ATT_HANDLE: */
    /*     break; */
    /* } */
}

static void letv_reconnect_init(void)
{
    /* letv_printf("\n--func=%s\n", __FUNCTION__); */
    letv_client_reconnect_init(g_reconnect_info + 6);
}

static void letv_handle_event_callback(u8 packet_type, u16 channel, u8 *packet, u16 size)
{
    static u8 report_status = 0;
    u8 *adv_public_addr = NULL;

#define HCI_EVENT_PACKET                            0x04
#define HCI_EVENT_LE_META                           0x3e
#define HCI_SUBEVENT_LE_ADVERTISING_REPORT          0x02
#define HCI_SUBEVENT_LE_CONNECTION_COMPLETE         0x01
#define HCI_EVENT_DISCONNECTION_COMPLETE            0x05
#define ADV_REPORT_PEER_ADDR_OFFSET                 6

    if ((packet_type != HCI_EVENT_PACKET) && (packet_type != BLE_HOST_CLIENT_READY)) {
        return;
    }

    if(packet_type == BLE_HOST_CLIENT_READY) {
        must_puts("\n****************************\n");
        must_puts("--------remote control ready\n");
        must_puts("****************************\n");

        memcpy(g_reconnect_info, ble_resume_connect_addr, 6);
        memcpy(g_reconnect_info + 6, packet, size);
        if(VM_BLE_RECONNECT_INFO_LEN != vm_write_api(VM_BLE_RECONNECT_INFO, g_reconnect_info)) {
            must_puts("----VM_BLE_RECONNECT_ADDR write err=0x%x\n");
        }
        return;
    }

    switch (packet[0]) {
    case HCI_EVENT_LE_META: {
        switch (packet[2]) {
        case HCI_SUBEVENT_LE_ADVERTISING_REPORT:
            adv_public_addr = packet + ADV_REPORT_PEER_ADDR_OFFSET;
            /* letv_pbuf(adv_public_addr, 6); */
            //解析 adv type
            if (get_adv_if_direct_reconnect(packet, ble_resume_connect_addr)) {
                letv_reconnect_init();
                goto __handle_connect;
            } else if (get_adv_type_uuid16_if_hid(packet) && \
                       get_adv_if_assign_name(packet, "Helios")) {
                goto __handle_connect;
            } else {
                return;
            }
__handle_connect:
            /* letv_puts("GATT Client: HCI_SUBEVENT_LE_ADVERTISING_REPORT\n"); */
            /* letv_pbuf(packet, size); */
            if (report_status == 0) {
                report_status++;
                letv_client_scan_control(0);
                letv_client_connect_control(adv_public_addr);
                memcpy(ble_resume_connect_addr, adv_public_addr, sizeof(ble_resume_connect_addr));
            }
            break;

        case HCI_SUBEVENT_LE_CONNECTION_COMPLETE:
            putchar('S');
            break;
        }
        break;
    }
    break;

    case HCI_EVENT_DISCONNECTION_COMPLETE:
        report_status = 0;
        putchar('E');
        letv_client_scan_control(1);
        break;

    default:
        break;
    }
}

void ble_gatt_client_init(void)
{
    letv_printf("\n--func=%s\n", __FUNCTION__);

	if(VM_BLE_RECONNECT_INFO_LEN == vm_read_api(VM_BLE_RECONNECT_INFO, g_reconnect_info)) {
        memcpy(ble_resume_connect_addr, g_reconnect_info, 6);
    } else {
        must_puts("ble_gatt_client_init read vm err\n");  
    }
    letv_client_init(&letv_hid_process, &letv_handle_event_callback);
}


/*******************************************************************/
/*
 *-------------------BLE KEY
 */
#if 0
//letv key map
#define HID_KEY_POWER_OFF               0x00
#define HID_KEY_VOL_NONE                0x7c
#define HID_KEY_VOL_UP                  0x80
#define HID_KEY_VOL_DOWN                0x81
#define HID_KEY_SYS_SET                 0x3c
#define HID_KEY_MIC_IN                  0x00
#define HID_KEY_CH_UP                   0x3a
#define HID_KEY_CH_DOWN                 0x3b
#define HID_KEY_UP                      0x52
#define HID_KEY_DOWN                    0x51
#define HID_KEY_LEFT                    0x50
#define HID_KEY_RIGHT                   0x49
#define HID_KEY_ENTER                   0x28
#define HID_KEY_DIR                     0x3e
#define HID_KEY_HOME                    0x3f
#define HID_KEY_REBACK                  0xf1

const u8 ble_key_map[] = {
    HID_KEY_POWER_OFF,
    HID_KEY_VOL_NONE,
    HID_KEY_VOL_UP,
    HID_KEY_VOL_DOWN,
    HID_KEY_SYS_SET,
    HID_KEY_CH_UP,
    HID_KEY_CH_DOWN,
    HID_KEY_UP,
    HID_KEY_DOWN,
    HID_KEY_LEFT,
    HID_KEY_RIGHT,
    HID_KEY_ENTER,
    HID_KEY_DIR,
    HID_KEY_HOME,
    HID_KEY_REBACK,
};

static u8 ble_key_get_number(void)
{
    u8 key_num;

    for (key_num = 0; key_num < sizeof(ble_key_map); key_num++) {
        if (ble_key_map[key_num] == g_ble_key_num) {
            break;
        }
    }
    if (key_num == sizeof(ble_key_map)) {
        key_num = INPUT_KEY_INVALID;
    }

    return key_num;
}
#endif

