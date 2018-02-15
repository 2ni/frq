/*
 * ESP8266 reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
 *
 * c implementation of frequency counter with GPIO interrupt
 *
 * frequency measurement on GPIO12 up to ~280kHz
 *
 */

#include "frq.h"
volatile uint32 frq_count;

void clear_isr() {
    // clear interrupts status
    uint32 s = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, s);
}

void ICACHE_RAM_ATTR subscribe_isr(void (*isr)()) {
    ETS_GPIO_INTR_DISABLE();

    ETS_GPIO_INTR_ATTACH(isr, 12); // GPIO12/D6 interrupt handler
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12); // Set GPIO12 function
    gpio_output_set(0, 0, 0, GPIO_ID_PIN(12)); // Set GPIO12 as input
    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(12)); // Clear GPIO12 status
    gpio_pin_intr_state_set(GPIO_ID_PIN(12), GPIO_PIN_INTR_POSEDGE); // Interrupt on any GPIO12 edge

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
float get_frequ(void) {
    unsigned long s;

    subscribe_isr(isr_measure_count);

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
