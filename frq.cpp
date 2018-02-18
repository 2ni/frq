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

void ICACHE_RAM_ATTR subscribe_isr(void (*isr)(int), int gpio, int *ok) {
    ETS_GPIO_INTR_DISABLE();
    if (ok) {*ok=0;}

    // some GPIO's should not be used
    if (!evil[gpio]) {
      PIN_FUNC_SELECT(mux[gpio], func[gpio]);
      gpio_output_set(0, 0, 0, GPIO_ID_PIN(gpio));
      GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio));
      ETS_GPIO_INTR_ATTACH(isr, gpio);
      gpio_pin_intr_state_set(GPIO_ID_PIN(gpio), GPIO_PIN_INTR_POSEDGE);

      if (ok) {*ok=1;}
    }

    ETS_GPIO_INTR_ENABLE() ;
}

void ICACHE_RAM_ATTR isr_measure_count(int gpio) {
    ETS_GPIO_INTR_DISABLE();
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    if (gpio_status & BIT(gpio)) {
      frq_count++;
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    ETS_GPIO_INTR_ENABLE();
}

/*
 * function get(<GPIO pin>)
 * returns -1 if sth is wrong
 * measures frequency in kHz by counting ticks during given time period
 */
float get_frequ(int gpio) {
    unsigned long s;
    int ok;
    int total_count = 0;

    subscribe_isr(isr_measure_count, gpio, &ok);
    if (!ok) { return -1.0; }

    // get average measurements
    for (int c=0; c<MEASURE_AMOUNT; c++) {
        frq_count = 0;
        s = system_get_time();
        while (system_get_time()<(s+MEASURE_TIME)) {
          // TODO use timer interrupt
          //os_delay_us(100);
          ; // wait for interrupts
        }
        total_count += frq_count;
        //printf("frq_count (%d): %d\n", c, frq_count);
    }
    return (float)total_count/MEASURE_AMOUNT*1000/MEASURE_TIME;
}
