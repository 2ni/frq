/*
 * ESP8266 reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
 *
 * c implementation of frequency counter with GPIO interrupt and timers
 *
 * frequency measurement on GPIO up to ~280kHz
 *
 */

#include "frq.h"
volatile uint32 frq_count;
static os_timer_t tmr;
uint32 total_count;
float *ret_val;
int tmr_count;

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

void ICACHE_RAM_ATTR tmrfc(void *arg) {
  ETS_GPIO_INTR_DISABLE();
  //printf("tmr isr. tmr_count: %d, frq_count: %d\n\r", tmr_count, frq_count);

  total_count += frq_count;
  frq_count = 0;
  tmr_count -= 1;

  if (tmr_count<=0) {
    os_timer_disarm(&tmr);
    *((int *)arg) = 1; // ready
    *ret_val = (float)total_count/MEASURE_AMOUNT/MEASURE_TIME;
  } else {
    os_timer_arm(&tmr, MEASURE_TIME, 0);
  }

  ETS_GPIO_INTR_ENABLE();
}

/*
 * function get(<GPIO pin>, <pointer of value>, <pointer of ready>)
 * call as follows
 *   int ready;
 *   float value;
 *   get_frequ(12, &value, &ready);
 *   while(!ready()) { yield; }
 *   if(value != -1) { do sth; }
 *
 * returns -1 if sth is wrong
 * measures frequency in kHz by counting ticks during given time period
 */
void get_frequ(int gpio, float *pvalue, int *pready) {
  total_count = 0;
  *pready = 0;
  ret_val = pvalue;
  frq_count = 0;

  int ok;
  subscribe_isr(isr_measure_count, gpio, &ok);
  if (!ok) {
    *pready = 1;
    *ret_val = -1;
    return;
  }

  tmr_count = MEASURE_AMOUNT;
  os_timer_disarm(&tmr);
  os_timer_setfn(&tmr, (os_timer_func_t *)tmrfc, pready);
  os_timer_arm(&tmr, MEASURE_TIME, 0); // repeat mode does not seem to have same timings
  ETS_GPIO_INTR_ENABLE() ;
}

