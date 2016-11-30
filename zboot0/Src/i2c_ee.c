#include "main.h"
#include "stm32f0xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define I2C_EE_ADDR 0xA0
#define I2C_EE2_ADDR 0xA2
#define EE_I2C hi2c1
uint8_t hex_value( uint8_t ch ) 
{
   if ((ch >= '0' ) && ( ch <= '9'))
      return ch - '0';
   if ((ch >= 'A') && (ch <= 'F')) 
      return ch - 'A' + 10;
   if ((ch >= 'a') && (ch <= 'f'))
      return ch - 'a' + 10;
   return 0;
}
void hex2bin( uint8_t *pin, uint8_t *pout, uint8_t len ) 
{
   uint8_t i;
   for ( i = 0; i < len; i ++ ) {
      pout[i] = (hex_value( pin[i*2]) << 4)+ hex_value( pin[i*2+1]);
   }
}
HAL_StatusTypeDef EE_ReadBuffer( uint8_t device_id, uint16_t ReadAddr, uint8_t *pBuffer, uint16_t numByteToRead ) 
{
   // i2c two step
   // 1. write the read address only
   // 2. read the buffer using device_id+1
   uint8_t addr_buffer[2];
   HAL_StatusTypeDef flag;
   addr_buffer[0] = ReadAddr >> 8;
   addr_buffer[1] = ReadAddr;
   flag = HAL_I2C_Master_Transmit( &EE_I2C, device_id, addr_buffer, 2, 300 );
   if ( flag != HAL_OK ) return flag;
   flag = HAL_I2C_Master_Receive( &EE_I2C, device_id+1, pBuffer, numByteToRead, 300 );
   return flag;
}
void Bootloader_Error_Callback(void)
{
   extern void jump_to_application(void);
   jump_to_application();
}
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_6   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_PAGE_63 + FLASH_PAGE_SIZE   /* End @ of user Flash area */
static FLASH_EraseInitTypeDef EraseInitStruct;
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0, 2 Kbytes */
#define ADDR_FLASH_PAGE_6     ((uint32_t)0x08003000) /* Base @ of Page 6, 2 Kbytes */
#define ADDR_FLASH_PAGE_63    ((uint32_t)0x0801F800) /* Base @ of Page 63, 2 Kbytes */
static char erase_flags[64];
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
uint8_t data[128], len, sum, stype;
uint16_t buffer[64];
uint16_t offsetAddr;
uint32_t upperAddr;
int8_t I2C_BootLoader(void)
{
   uint16_t eeaddr = 0;
   int8_t flash_flag = 0; 
   int i;
   HAL_StatusTypeDef flag;
   for ( i = 0; i < 64; i ++ ) {
      erase_flags[i] = 0;
   }
   flag = EE_ReadBuffer( I2C_EE_ADDR, eeaddr, data, 11 ); // read first 9 bytes
   if ( flag != HAL_OK ) {
      Bootloader_Error_Callback();
      return 0;
   }
   eeaddr += 11;
   if ( data[0] == ':' ) {
      HAL_FLASH_Unlock();
   }
   // #1
   while (( data[0] == ':') && (flash_flag == 0)) {
      hex2bin( data+1, data+1, 5 );
      len = data[1];
      if ( len != 0 ) {
         EE_ReadBuffer( I2C_EE_ADDR, eeaddr, data + 11, len * 2 );
         eeaddr += len * 2;
         hex2bin( data+11, data+6, len );
      }
      // checksum
      for ( i = 0, sum = 0; i < (5+len); i ++ ) {
         sum += data[i+1];
      }
      if ( sum != 0 ) return 0;      
      stype = data[4];
      switch( stype ) {
         case 0: // data
            offsetAddr = data[2];
            offsetAddr = (offsetAddr << 8) + data[3];
            // data[5] ..., with length len, 
            // target address = upperAddr + offsetAddr
            // write flash
            // data[5] is not aligned, copy to buffer
            for( i = 0; i < (len/2); i ++ ) {
               buffer[i] = data[i*2+6]; // order
               buffer[i] = (buffer[i] << 8) + data[i*2+5];
            }
            flag = flash_programming( upperAddr + offsetAddr, buffer, len / 2);
            if ( flag != HAL_OK ) 
               flash_flag = -1;
            break;
         case 1: // end of file
            flash_flag = 1;
            break;
         case 2: // Extended Segment Address
         case 3: // Start Segment Address
            break;
         case 4: // Extended Linear Address
            upperAddr = data[5];
            upperAddr = (upperAddr << 8) + data[6];
            upperAddr <<= 16;
            break;
         case 5: // Start Linear Address
            break;
      }
      // seek another : marker
      while(flash_flag == 0) {
         flag = EE_ReadBuffer( I2C_EE_ADDR, eeaddr, data, 1 );
         if ( flag != HAL_OK ) {
            flash_flag = -1;
            break;
         }
         if ((data[0] == 0x0D) || (data[0] == 0x0A)) {
            eeaddr ++;
            continue;
         }
         if ( data[0] == ':' ) {
            flag = EE_ReadBuffer( I2C_EE_ADDR, eeaddr+1, data+1, 10 ); // read first 9 bytes
            if ( flag != HAL_OK ) {
               flash_flag = -1;
               break;
            }
            eeaddr += 11;
            break;
         } else {
            flash_flag = -1;
            break;
         }
      }
   }
   HAL_FLASH_Lock();
   if ( flash_flag == 1 ) return 1;
   return 0;
}
