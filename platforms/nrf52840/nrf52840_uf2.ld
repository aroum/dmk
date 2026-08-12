/* Linker script to configure memory regions for nRF52840 with Adafruit UF2 Bootloader + SoftDevice S140 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x26000, LENGTH = 0xda000
  EXTFLASH (rx) : ORIGIN = 0x12000000, LENGTH = 0x8000000
  RAM (rwx) :  ORIGIN = 0x20005978, LENGTH = 0x3a688
  CODE_RAM (rwx) : ORIGIN = 0x800000, LENGTH = 0x40000
}

INCLUDE "nrf_common.ld"
