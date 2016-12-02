#ifndef _UTIL_FLASH_H
#define _UTIL_FLASH_H
int8_t check_flash_address( uint32_t addr );
HAL_StatusTypeDef flash_programming( uint32_t addr, uint16_t *data, uint16_t len );
void init_flash(void);

#endif
