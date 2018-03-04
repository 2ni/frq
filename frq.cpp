/*
 * ESP8266 reference: https://www.espressif.com/sites/default/files/documentation/2c-esp8266_non_os_sdk_api_reference_en.pdf
 *
 * c implementation of frequency counter with GPIO interrupt
 *
 * frequency measurement on GPIO up to ~280kHz
 * TODO https://www.pjrc.com/teensy/td_libs_FreqCount.html
 * xy.begin(); xy.available(); xy.read();
 *
 */

#include "frq.h"
volatile uint32 frq_count;
static os_timer_t tmr;
uint32 total_count;
float *ret_val;
int tmr_count;

/*
 * setup gpio interrupt handling
 */
void ICACHE_RAM_ATTR subscribe_isr(void (*isr)(int), int gpio, int *ok) {
    ETS_GPIO_INTR_DISABLE();
    if (ok) {*ok=0;}

    // some GPIO's should not be used
    if (!evil[gpio]) {
      PIN_FUNC_SELECT(mux[gpio], func[gpio]);
      // not needed, does interfere with preset gpio settings gpio_output_set(0, 0, 0, GPIO_ID_PIN(gpio));
      GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(gpio));
      ETS_GPIO_INTR_ATTACH(isr, gpio);
      gpio_pin_intr_state_set(GPIO_ID_PIN(gpio), GPIO_PIN_INTR_POSEDGE);

      if (ok) {*ok=1;}
    }
}

/*
 * gpio interrupt subroutine
 */
void ICACHE_RAM_ATTR isr_measure_count(int gpio) {
    ETS_GPIO_INTR_DISABLE();
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
    if (gpio_status & BIT(gpio)) {
      frq_count += 1;
    }

    GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);

    ETS_GPIO_INTR_ENABLE();
}

/*
 * timer intterupt subroutine
 */
void ICACHE_RAM_ATTR tmrfc(void *arg) {
  ETS_GPIO_INTR_DISABLE();
  printf("tmr isr. tmr_count: %d, frq_count: %d\n\r", tmr_count, frq_count);

  total_count += frq_count;
  frq_count = 0;
  tmr_count -= 1;

  if (tmr_count <= 0) {
    os_timer_disarm(&tmr);
    *((int *)arg) = true; // ready
    *ret_val = (float)total_count/MEASURE_AMOUNT/MEASURE_TIME;
    //printf("ret_val: %d.%01d\n\r", (int)*ret_val, (int)(*ret_val*10)%10);
  } else {
    os_timer_arm(&tmr, MEASURE_TIME, 0);
  }

  ETS_GPIO_INTR_ENABLE();
}

/*
 * measures frequency in kHz by counting ticks during given time period
 * gpio: which input pin to measure
 * pvalue: pointer to value where value will be saved when ready, -1 if failure
 * pready: pointer to boolean to wait for readiness of value
 * mode: 0 waiting loop, 1 timer interrupts
 */
void get_frequ(int gpio, float *pvalue, bool *pready) {
  get_frequ(gpio, pvalue, pready, 0);
}

void get_frequ(int gpio, float *pvalue, bool *pready, bool mode) {
  total_count = 0;
  *pready = false;
  ret_val = pvalue;
  frq_count = 0;

  int ok;
  subscribe_isr(isr_measure_count, gpio, &ok);
  if (!ok) {
    *pready = true;
    *ret_val = -1;
    return;
  }

  tmr_count = MEASURE_AMOUNT;

  // waiting time mode
  if (mode == 0) {
    unsigned long s;
    while (tmr_count > 0) {
        frq_count = 0;
        s = system_get_time();
        ETS_GPIO_INTR_ENABLE();
        while (system_get_time()<(s+MEASURE_TIME*1000)) {
          ; // wait for interrupts
        }
        ETS_GPIO_INTR_DISABLE();

        printf("time diff: %u\n\r", system_get_time()-s);
        total_count += frq_count;
        printf("frq_count (%d): %d\n\r", tmr_count, frq_count);
        tmr_count -= 1;
    }
    printf("total count: %u\n\r", total_count);
    *ret_val = (float)total_count/MEASURE_AMOUNT/MEASURE_TIME;
    *pready = true;
  } else {
    // timer interrtupt mode
    os_timer_disarm(&tmr);
    os_timer_setfn(&tmr, (os_timer_func_t *)tmrfc, pready);
    ETS_GPIO_INTR_ENABLE();
    os_timer_arm(&tmr, MEASURE_TIME, 0); // repeat mode does not seem to have same timings
  }
}
