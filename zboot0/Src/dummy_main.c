#include "main.h"
#include "stm32f0xx_hal.h"
//
// put the dummy_vector_table to application_address 08003000
//
extern void dummy_main_function(void);
uint32_t const dummy_vector_table[2] = {
  0x200004e8, 
  (uint32_t) dummy_main_function // 
};

