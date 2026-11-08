#include "magneteno_imu.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"
#include "hal_i2c.h"

#include "../sequencer/sequencer.h"
#include "../synth/synth.h"

#include <math.h>
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
#define BMI270_REG_DATA_ACC 0x12
#define BMI270_REG_PWR_CONF 0x7C
#define BMI270_REG_PWR_CTRL 0x7D
#define BMI270_CHIP_ID_VAL  0x24

MagnetenoIMUState g_imu_state = {
    .roll_deg = 0.0f,
    .pitch_deg = 0.0f,
    .is_shaking = false,
    .tap_detected = 0,
    .sensor_ready = false
};

static float s_prev_acc_mag = 1.0f;
static uint32_t s_last_tap_time_ms = 0;

void magneteno_imu_init(void) {
    hal_i2c_init(BMI270_SDA_PIN, BMI270_SCL_PIN, HAL_I2C_FREQ_FAST);

    vTaskDelay(pdMS_TO_TICKS(50));

    uint8_t chip_id = 0;
    if (hal_i2c_read_reg(BMI270_I2C_ADDR, BMI270_REG_CHIP_ID, &chip_id, 1)) {
        if (chip_id == BMI270_CHIP_ID_VAL) {
            g_imu_state.sensor_ready = true;
        }
    }

    if (!g_imu_state.sensor_ready) {
        // Sensor not detected; exit early so keyboard and synth run with zero delays
        return;
    }

    // Disable power-save mode (PWR_CONF = 0x00)
    uint8_t pwr_conf = 0x00;
    hal_i2c_write_reg(BMI270_I2C_ADDR, BMI270_REG_PWR_CONF, &pwr_conf, 1);
    vTaskDelay(pdMS_TO_TICKS(10));

    // Enable Accelerometer and Gyroscope (PWR_CTRL: bit 1=acc_en, bit 2=gyr_en, bit 3=temp => 0x0E)
    uint8_t pwr_ctrl = 0x0E;
    hal_i2c_write_reg(BMI270_I2C_ADDR, BMI270_REG_PWR_CTRL, &pwr_ctrl, 1);
    vTaskDelay(pdMS_TO_TICKS(20));
}

void magneteno_imu_poll(void) {
    if (!g_imu_state.sensor_ready) {
        return;
    }

    uint8_t acc_raw[6];
    uint8_t gyr_raw[6];

    // 1. Read Accelerometer data (X, Y, Z - 6 bytes)
    if (hal_i2c_read_reg(BMI270_I2C_ADDR, BMI270_REG_DATA_ACC, acc_raw, 6)) {
        int16_t ax = (int16_t)((acc_raw[1] << 8) | acc_raw[0]);
        int16_t ay = (int16_t)((acc_raw[3] << 8) | acc_raw[2]);
        int16_t az = (int16_t)((acc_raw[5] << 8) | acc_raw[4]);

        float fx = (float)ax / 16384.0f; // in g
        float fy = (float)ay / 16384.0f;
        float fz = (float)az / 16384.0f;

        // Calculate Roll and Pitch angles in degrees
        float roll = atan2f(fy, fz) * (180.0f / (float)M_PI);
        float pitch = atan2f(-fx, sqrtf(fy * fy + fz * fz)) * (180.0f / (float)M_PI);

        // Exponential smoothing (low-pass filter)
        g_imu_state.roll_deg = 0.8f * g_imu_state.roll_deg + 0.2f * roll;
        g_imu_state.pitch_deg = 0.8f * g_imu_state.pitch_deg + 0.2f * pitch;

        // Total acceleration magnitude
        float mag = sqrtf(fx * fx + fy * fy + fz * fz);

        // Shake detection: total magnitude > 2.2g
        if (mag > 2.2f) {
            g_imu_state.is_shaking = true;
        } else {
            g_imu_state.is_shaking = false;
        }

        // Tap tempo detection: sharp jerk pulse
        float jerk = fabsf(mag - s_prev_acc_mag);
        s_prev_acc_mag = mag;

        uint32_t now_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (jerk > 0.85f && (now_ms - s_last_tap_time_ms > 180)) {
            uint32_t delta_ms = now_ms - s_last_tap_time_ms;
            s_last_tap_time_ms = now_ms;
            g_imu_state.tap_detected++;

            // If tap interval corresponds to musical tempo 40..240 BPM (250ms .. 1500ms)
            if (delta_ms >= 250 && delta_ms <= 1500) {
                uint8_t bpm = (uint8_t)(60000 / delta_ms);
                if (bpm >= 40 && bpm <= 240) {
                    sequencer_set_bpm(bpm);
                }
            }
        }
    }

    // 2. Read Gyroscope data for rapid vibrato gestures
    if (hal_i2c_read_reg(BMI270_I2C_ADDR, BMI270_REG_DATA_GYR, gyr_raw, 6)) {
        // Gyro data read successfully
    }
}
