#ifndef MAGNETENO_IMU_H
#define MAGNETENO_IMU_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float roll_deg;       // Left/Right tilt (-90.0 .. +90.0 degrees)
    float pitch_deg;      // Forward/Backward tilt (-90.0 .. +90.0 degrees)
    bool is_shaking;      // Momentary shake / jerk gesture detected
    uint8_t tap_detected; // Chassis tap count
    bool sensor_ready;    // True if BMI270 was successfully detected and initialized
} MagnetenoIMUState;

extern MagnetenoIMUState g_imu_state;

/**
 * @brief Initialize I2C0 and configure Bosch BMI270 (accel + gyro).
 */
void magneteno_imu_init(void);

/**
 * @brief Poll BMI270 sensor, compute tilt angles, shake gesture, and tap tempo.
 * Should be called at ~100Hz from UI task.
 */
void magneteno_imu_poll(void);

#ifdef __cplusplus
}
#endif

#endif // MAGNETENO_IMU_H
