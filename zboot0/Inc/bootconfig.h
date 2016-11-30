/** 
 * bootconfig.h
 * configuration for bootloader
 */
#ifndef _BOOTCONFIG_H
#define _BOOTCONFIG_H

#define APPLICATION_ADDRESS     (uint32_t)0x08003000
extern int8_t Intel_BootLoader(void);
#define BootLoader() Intel_BootLoader()
#define FLASH_PAGE_NO 64
#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_6   /* Start @ of user Flash area */
//#define FLASH_USER_END_ADDR     ADDR_FLASH_PAGE_63 + FLASH_PAGE_SIZE   /* End @ of user Flash area */
#define ADDR_FLASH_PAGE_0     ((uint32_t)0x08000000) /* Base @ of Page 0, 2 Kbytes */
#define ADDR_FLASH_PAGE_6     ((uint32_t)0x08003000) /* Base @ of Page 6, 2 Kbytes */
#define ADDR_FLASH_PAGE_63    ((uint32_t)0x0801F800) /* Base @ of Page 63, 2 Kbytes */

// define I2C
extern I2C_HandleTypeDef hi2c1;
#define I2C_EE_ADDR 0xA0
#define I2C_EE2_ADDR 0xA2
#define EE_I2C hi2c1

// dummy main
#define Uses_DUMMY_MAIN

#endif
