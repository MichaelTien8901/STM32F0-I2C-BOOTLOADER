#include "main.h"
#include "stm32f0xx_hal.h"
//
// put the dummy_vector_table to application_address 08003000
//
extern void dummy_main_function(void);
typedef void (*pfunc)(void);
pfunc const dummy_vector_table[2] = {
  (pfunc) 0x200004e8, // stack
//  (uint32_t) dummy_main_function // extended constant initialiser used warning??
  dummy_main_function
};

