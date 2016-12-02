#include "main.h"
#include "stm32f0xx_hal.h"
#include "bootconfig.h"
#include "util_flash.h"

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
extern HAL_StatusTypeDef StreamRead( uint16_t ReadAddr, uint8_t *pBuffer, uint16_t numByteToRead );
void Bootloader_Error_Callback(void)
{
   extern void jump_to_application(void);
   jump_to_application();
}

static uint8_t data[128], len, sum, stype;
static uint16_t buffer[64];
static uint16_t offsetAddr;
static uint32_t upperAddr;

int8_t Intel_BootLoader(void)
{
   
   uint16_t eeaddr = 0;
   int8_t flash_flag = 0; 
   int i;
   HAL_StatusTypeDef flag;
   init_flash();
   flag = StreamRead( eeaddr, data, 11 ); // read first 9 bytes
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
         StreamRead( eeaddr, data + 11, len * 2 );
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
         flag = StreamRead( eeaddr, data, 1 );
         if ( flag != HAL_OK ) {
            flash_flag = -1;
            break;
         }
         if ((data[0] == 0x0D) || (data[0] == 0x0A)) {
            eeaddr ++;
            continue;
         }
         if ( data[0] == ':' ) {
            flag = StreamRead( eeaddr+1, data+1, 10 ); // read first 9 bytes
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
