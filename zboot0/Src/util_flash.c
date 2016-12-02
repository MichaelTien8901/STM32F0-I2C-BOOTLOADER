#include "main.h"
#include "stm32f0xx_hal.h"
#include "bootconfig.h"
#include "util_flash.h"

static FLASH_EraseInitTypeDef EraseInitStruct;
static char erase_flags[FLASH_PAGE_NO];
uint32_t PageError = 0;
int8_t check_flash_address( uint32_t addr )
{
   uint32_t user_start_address;
   uint32_t user_start_page;
   if ( addr < FLASH_USER_START_ADDR ) return 0;
   user_start_page = (addr - ADDR_FLASH_PAGE_0) / FLASH_PAGE_SIZE;
   if ( erase_flags[user_start_page] ) return 1;
   erase_flags[user_start_page] = 1;
   user_start_address = user_start_page * FLASH_PAGE_SIZE + ADDR_FLASH_PAGE_0;
  /* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
  EraseInitStruct.PageAddress = user_start_address;
  EraseInitStruct.NbPages = 1;

  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
  {
    /*
      Error occurred while page erase.
      User can add here some code to deal with this error.
      PageError will contain the faulty page and then to know the code error on this page,
      user can call function 'HAL_FLASH_GetError()'
    */
    /* Infinite loop */
     return 0;
  }
  return 1;
}
HAL_StatusTypeDef flash_programming( uint32_t addr, uint16_t *data, uint16_t len )
{
   int16_t index;
   HAL_StatusTypeDef flag;
   if ( !check_flash_address( addr )) return HAL_ERROR;
   for ( index = 0; index < len; index ++ ) {
      flag = HAL_FLASH_Program( FLASH_TYPEPROGRAM_HALFWORD, addr, *data++ );
      addr += 2;
      if ( flag != HAL_OK ) return flag;
   }
   return HAL_OK;
}
void init_flash(void)
{
   int i;
   for ( i = 0; i < FLASH_PAGE_NO; i ++ ) {
      erase_flags[i] = 0;
   }   
}
