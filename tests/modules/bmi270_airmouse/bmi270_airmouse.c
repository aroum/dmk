/**
 * @file bmi270_airmouse.c
 * @brief Bosch BMI270 6-DoF IMU Air Mouse Module for DMK.
 *
 * Demonstrates:
 * 1. Initializing and configuring BMI270 over cross-platform HAL I2C (hal_i2c.h).
 * 2. Reading angular velocity data from gyroscope registers at 100 Hz in a FreeRTOS task.
 * 3. Translating IMU rotation into relative cursor deltas (dx, dy).
 * 4. Feeding motion directly into DMK's USB HID mouse subsystem via mouse_move().
 */

#include "FreeRTOS.h"
#include "task.h"
#include "config.h"
#include "hal_i2c.h"
#include "hooks.h"
#include "mouse.h"
#include <stdbool.h>
#include <stdint.h>

#ifndef BMI270_I2C_ADDR
#define BMI270_I2C_ADDR 0x68
#endif

#ifndef BMI270_SDA_PIN
#define BMI270_SDA_PIN 4
#endif

#ifndef BMI270_SCL_PIN
#define BMI270_SCL_PIN 5
#endif

#define BMI270_REG_CHIP_ID  0x00
#define BMI270_REG_DATA_GYR 0x0C
#define BMI270_REG_PWR_CONF 0x7C
#define BMI270_REG_PWR_CTRL 0x7D
#define BMI270_CHIP_ID_VAL  0x24

// Deadband threshold for gyro noise suppression (in raw LSB units)
#ifndef BMI270_DEADBAND
#define BMI270_DEADBAND 120
#endif

// Sensitivity divisor (smaller = faster cursor speed)
#ifndef BMI270_SENSITIVITY_DIV
#define BMI270_SENSITIVITY_DIV 400
#endif

static bool s_airmouse_enabled = true;

void bmi270_airmouse_set_enabled(bool enabled) {
    s_airmouse_enabled = enabled;
}

bool bmi270_airmouse_is_enabled(void) {
    return s_airmouse_enabled;
}

static void bmi270_airmouse_task(void *pvParameters) {
    (void)pvParameters;

    // Small delay to let sensor power-on reset complete
    vTaskDelay(pdMS_TO_TICKS(50));

    // Verify chip ID
    uint8_t chip_id = 0;
    if (hal_i2c_read_reg(BMI270_I2C_ADDR, BMI270_REG_CHIP_ID, &chip_id, 1)) {
        if (chip_id != BMI270_CHIP_ID_VAL) {
            // Chip ID mismatch or device not responding; continue anyway in case of clone/derivative
        }
    }

    // Disable power-save mode (PWR_CONF = 0x00)
    uint8_t pwr_conf = 0x00;
    hal_i2c_write_reg(BMI270_I2C_ADDR, BMI270_REG_PWR_CONF, &pwr_conf, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Enable Accelerometer and Gyroscope (PWR_CTRL: bit 1=acc_en, bit 2=gyr_en => 0x06)
    uint8_t pwr_ctrl = 0x06;
    hal_i2c_write_reg(BMI270_I2C_ADDR, BMI270_REG_PWR_CTRL, &pwr_ctrl, 1);
    vTaskDelay(pdMS_TO_TICKS(20));

    while (1) {
        if (s_airmouse_enabled) {
            uint8_t gyro_raw[6];
            // Read 6 bytes starting from DATA_8 (GYR_X_LSB, GYR_X_MSB, GYR_Y_LSB, GYR_Y_MSB, GYR_Z_LSB, GYR_Z_MSB)
            if (hal_i2c_read_reg(BMI270_I2C_ADDR, BMI270_REG_DATA_GYR, gyro_raw, 6)) {
                int16_t gyr_x = (int16_t)((gyro_raw[1] << 8) | gyro_raw[0]);
                int16_t gyr_y = (int16_t)((gyro_raw[3] << 8) | gyro_raw[2]);
                int16_t gyr_z = (int16_t)((gyro_raw[5] << 8) | gyro_raw[4]);
                (void)gyr_x;

                // Yaw rotation (Z axis) maps to horizontal cursor delta (dx)
                // Pitch rotation (Y or X axis depending on mounting) maps to vertical cursor delta (dy)
                int8_t dx = 0;
                int8_t dy = 0;

                if (gyr_z > BMI270_DEADBAND) {
                    dx = -(int8_t)((gyr_z - BMI270_DEADBAND) / BMI270_SENSITIVITY_DIV);
                } else if (gyr_z < -BMI270_DEADBAND) {
                    dx = -(int8_t)((gyr_z + BMI270_DEADBAND) / BMI270_SENSITIVITY_DIV);
                }

                if (gyr_y > BMI270_DEADBAND) {
                    dy = (int8_t)((gyr_y - BMI270_DEADBAND) / BMI270_SENSITIVITY_DIV);
                } else if (gyr_y < -BMI270_DEADBAND) {
                    dy = (int8_t)((gyr_y + BMI270_DEADBAND) / BMI270_SENSITIVITY_DIV);
                }

                if (dx != 0 || dy != 0) {
                    mouse_move(dx, dy);
                }
            }
        }
        // Sample at 100 Hz (10 ms polling rate)
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void hook_early_init(void) {
    // Initialize I2C bus at 400 kHz (Fast mode)
    hal_i2c_init(BMI270_SDA_PIN, BMI270_SCL_PIN, HAL_I2C_FREQ_FAST);

    // Spawn IMU air mouse background task
    xTaskCreate(bmi270_airmouse_task, "bmi270", 256, NULL, tskIDLE_PRIORITY + 2, NULL);
}
