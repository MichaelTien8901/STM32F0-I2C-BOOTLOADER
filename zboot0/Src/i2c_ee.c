#include "main.h"
#include "stm32f0xx_hal.h"

extern I2C_HandleTypeDef hi2c1;

#define I2C_EE_ADDR 0xA0
#define I2C_EE2_ADDR 0xA2
#define EE_I2C hi2c1

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
//
// the Generic Stream Reader using I2C
//
HAL_StatusTypeDef StreamRead( uint16_t ReadAddr, uint8_t *pBuffer, uint16_t numByteToRead ) 
{
   return EE_ReadBuffer( I2C_EE_ADDR, ReadAddr, pBuffer, numByteToRead );
}   
