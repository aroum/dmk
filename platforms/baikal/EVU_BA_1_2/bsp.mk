# Define the MCU model for the specified development board
MCU_MODEL := BMCU_U

CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_cru.c
CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_gpio.c
CSRC += $(SDK_DIR)/Drivers/HAL/Src/bmcu_uart.c
CSRC += $(BSP_ROOT_DIR)/Components/button/button.c
CSRC += $(BSP_ROOT_DIR)/Components/led/led.c
CSRC += $(BSP_ROOT_DIR)/Components/serial/serial.c
