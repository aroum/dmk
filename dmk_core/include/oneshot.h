#ifndef ONESHOT_H
#define ONESHOT_H

#include "FreeRTOS.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint8_t mod_mask;
    uint8_t layer;
    bool is_mod;
    bool active;
    bool pending_release;
    TickType_t activate_time;
    bool key_pressed;
} OneShotState;

#ifndef MAX_OS_TRACKERS
#define MAX_OS_TRACKERS 4
#endif

// Initialize the One-Shot subsystem
void oneshot_init(void);

// Process a One-Shot key event (returns true if handled by One-Shot subsystem)
bool oneshot_process_event(uint32_t key, bool pressed);

// Check if key should consume/trigger active One-Shot modifiers/layers
bool oneshot_should_consume(uint32_t key);

// Called when any normal key is pressed
void oneshot_on_key_press(uint32_t key);

// Called when any normal key is released
void oneshot_on_key_release(void);

// Helper for Hold-Tap tap resolution
void oneshot_send_lazy_mods(void);
void oneshot_on_tap_key(void);

// Check timeouts for all active One-Shot keys; returns ticks until next deadline or portMAX_DELAY
TickType_t oneshot_check_timeouts(TickType_t now);

#ifdef __cplusplus
}
#endif

#endif // ONESHOT_H
