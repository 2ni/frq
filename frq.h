extern "C" {
  #include <stdio.h>
  #include "ets_sys.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "os_type.h"
  #include "user_interface.h"
  void ets_delay_us(uint16_t ms);
}

#ifndef FRQ_H_
#define FRQ_H_

#define MEASURE_TIME 20 // in ms
#define MEASURE_AMOUNT 3 // how many times shall we measure to calculate average

#define Z 0
static const int mux[] = {
    PERIPHS_IO_MUX_GPIO0_U, /* 0 - D3 */
    PERIPHS_IO_MUX_U0TXD_U, /* 1 - uart */
    PERIPHS_IO_MUX_GPIO2_U, /* 2 - D4 */
    PERIPHS_IO_MUX_U0RXD_U, /* 3 - uart */
    PERIPHS_IO_MUX_GPIO4_U, /* 4 - D2 */
    PERIPHS_IO_MUX_GPIO5_U, /* 5 - D1 */
    Z, /* 6  mystery */
    Z, /* 7  mystery */
    Z, /* 8  mystery */
    PERIPHS_IO_MUX_SD_DATA2_U, /* 9 - D11 (SD2) */
    PERIPHS_IO_MUX_SD_DATA3_U, /* 10 - D12 (SD3) */
    Z, /* 11  mystery */
    PERIPHS_IO_MUX_MTDI_U, /* 12 - D6 */
    PERIPHS_IO_MUX_MTCK_U, /* 13 - D7 */
    PERIPHS_IO_MUX_MTMS_U, /* 14 - D5 */
    PERIPHS_IO_MUX_MTDO_U /* 15 - D8 */
};
static const int func[] = { 0, 3, 0, 3, 0, 0, Z, Z, Z, 3, 3, Z, 3, 3, 3, 3 };
// for some reasons some pins are not advised to be used
static const int evil[] = { 0, 1, 0, 1,   0, 0, 1, 1,   1, 1, 1, 1,   0, 0, 0, 0 };

int get_func(int gpio);
void clear_isr(void);
void subscribe_isr(void (*isr)(), int gpio);
void ICACHE_FLASH_ATTR isr_measure_count(void);
void get_frequ(int gpio, float *value, bool *ready, bool mode);
void get_frequ(int gpio, float *value, bool *ready);
//float get_frequ(int gpio);
void test(void);
#endif
