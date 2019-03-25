#ifndef _RF_TEST_UART_H_
#define _RF_TEST_UART_H_

//for call
void rf_uart_write(char a);
void rf_test_uart_init_callback(void);
void register_rf_test_uart_callback(u8 (*)(u8),u8 (*)(u8));
void rf_test_uart_para_init(char *uart_name,u32 baud,u8 io);
void ble_rf_test_info_register(u8 ble_bredr_mode,void (*set_ble_rf_test_default_para)(void));
void __set_ble_rf_para_init(u8 trx_mode,u8 channel,u8 packet_type,u8 packet_len,u8 tx_power);

#endif
