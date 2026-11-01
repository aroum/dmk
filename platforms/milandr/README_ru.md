# Платформа Миландр (K1986BE92FI / MDR32F9Q2I)

Поддержка отечественного 32-битного микроконтроллера ARM Cortex-M4 **К1986ВЕ92FI** (MDR32F9Q2I) производства ПКК «Миландр» в DMK Firmware.

## Бутлоадер и прошивка

- **Репозиторий DFU-бутлоадера**: [K1986BE92FI-dfu-bootloader](https://github.com/aroum/K1986BE92FI-dfu-bootloader)
- **Прошивка через USB DFU**:
  1. Переведите плату в режим DFU, удерживая `PB6` при сбросе.
  2. Прошейте с помощью `dfu-util` или скрипта `./build_all.sh ... -u`:
     ```bash
     dfu-util -a 0 -d 1209:be92 -D build/dmk_<keyboard>_milandr.bin
     ```
