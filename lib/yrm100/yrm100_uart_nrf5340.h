#ifndef YRM100_UART_NRF5340_H
#define YRM100_UART_NRF5340_H

/*
 * nRF5340 UARTE1 backend for the YRM100 library.
 *
 * Uses UARTE1 with interrupt-driven receive ring buffer.
 * Default pins (nRF7002DK Arduino header):
 *   TX → P1.04  (connect to YRM100 RX)
 *   RX ← P1.05  (connect to YRM100 TX)
 *
 * Override at compile time with -DYRM100_UART_PORT=… etc.
 *
 * Usage:
 *   #include "yrm100.h"
 *   #include "yrm100_uart_nrf5340.h"
 *
 *   yrm100_t rfid;
 *   yrm100_init(&rfid, &yrm100_nrf5340_uart, 115200);
 */

#include "yrm100.h"

extern const yrm100_uart_t yrm100_nrf5340_uart;

#endif /* YRM100_UART_NRF5340_H */
