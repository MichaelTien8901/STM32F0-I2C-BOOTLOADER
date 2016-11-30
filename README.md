# STM32F0-I2C-BOOTLOADER
STM32F072RB Discovery board bootloader using I2C EEPROM
## Circuit

### I2C EEPROM Microchip 24C512

#### Pins

| EEPROM 24C512 | STM32F072RB PINS | 
|:--------------|:-----------------|
| 1  A0         | GND |
| 2  A1         | GND |
| 3  A2         | GND |
| 4  GND        | GND |
| 5  SDA        | PB9, PULL-UP 4.7K to 3V |
| 6  SCL        | PB8 PULL-UP 4.7K to 3V  |
| 7  WP         | GND |
| 8  VCC        | 3V  |

## I2C EEPROM data format

Using INTEL hex format directly from KEIL compiler.
