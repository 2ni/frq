/*
 * ESP8266 reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
 *
 * c implementation of frequency counter with GPIO interrupt
 *
 * frequency measurement on GPIO up to ~280kHz
 *
 */

#include "frq.h"
volatile uint32 frq_count;

void clear_isr() {
    // clear interrupts status
    uint32 s = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, s);
}

int get_func(int gpio) {
  int func=0;

  switch(gpio) {
    case 0:
      func = FUNC_GPIO0;
    break;
    case 1:
      func = FUNC_GPIO1;
    break;
    case 2:
      func = FUNC_GPIO2;
    break;
    case 3:
      func = FUNC_GPIO3;
    break;
    case 4:
      func = FUNC_GPIO4;
    break;
    case 5:
      func = FUNC_GPIO5;
    break;
    case 9:
      func = FUNC_GPIO9;
    break;
    case 10:
      func = FUNC_GPIO10;
    break;
    case 12:
      func = FUNC_GPIO12;
    break;
    case 13:
      func = FUNC_GPIO13;
    break;
    case 14:
      func = FUNC_GPIO14;
    break;
    case 15:
      func = FUNC_GPIO15;
    break;
  }

  return func;
}

void ICACHE_RAM_ATTR subscribe_isr(void (*isr)(), int gpio) {
    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(isr, gpio);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, get_func(gpio));
    gpio_output_set(0, 0, 0, GPIO_ID_PIN(gpio));
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio));
    gpio_pin_intr_state_set(GPIO_ID_PIN(gpio), GPIO_PIN_INTR_POSEDGE);

    ETS_GPIO_INTR_ENABLE() ;
}

void ICACHE_RAM_ATTR isr_measure_count() {
    ETS_GPIO_INTR_DISABLE();
    frq_count++;

    clear_isr();
    ETS_GPIO_INTR_ENABLE();
}

/*
 * function get()
 * measures frequency in kHz by counting ticks during given time period
 */
float get_frequ(int gpio) {
    unsigned long s;

    subscribe_isr(isr_measure_count, gpio);

    int total_count = 0;

    // get average measurements
    for (int c=0; c<MEASURE_AMOUNT; c++) {
        frq_count = 0;
        s = system_get_time();
        while (system_get_time()<(s+MEASURE_TIME)) {
          //os_delay_us(1000);
          ; // wait for interrupts
        }
        total_count += frq_count;
        //printf("frq_count (%d): %d\n", c, frq_count);
    }
    return (float)total_count/MEASURE_AMOUNT*1000/MEASURE_TIME;
}
