extern "C" {
  #include <stdio.h>
  #include "ets_sys.h"
  #include "osapi.h"
  #include "gpio.h"
  #include "os_type.h"
  #include "user_interface.h"
}


#ifndef FRQ_H_
#define FRQ_H_

#define MEASURE_TIME 10000 // in us
#define MEASURE_AMOUNT 3 // how many times shall we measure to calculate average


void clear_isr(void);
void subscribe_isr(void (*isr)());
void ICACHE_FLASH_ATTR isr_measure_count(void);
float get_frequ(void);

#endif
