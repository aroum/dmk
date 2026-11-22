#pragma once

#include <stdbool.h>
#include <stdint.h>

void hhkb_hybrid_init(void);
bool hhkb_hybrid_on_battery(void);
bool hhkb_hybrid_button(void);
void hhkb_hybrid_report(uint8_t modifiers, const uint8_t keys[6]);
// Factory BLE consumer map; zero releases all consumer keys.
bool hhkb_hybrid_consumer_report(uint16_t usage);
void hhkb_hybrid_service(void);
bool hhkb_hybrid_request_slot(uint8_t slot, bool pair);
uint8_t hhkb_hybrid_battery_level(void);
uint8_t hhkb_hybrid_active_slot(void); // 0..3, or 0xff if disconnected
bool hhkb_hybrid_pairing(void);
void hhkb_hybrid_show_battery(void);
bool hhkb_hybrid_led_override(uint8_t index, uint32_t now_ms, bool *on);
bool hhkb_hybrid_powered(void);
void hhkb_hybrid_button_poll(uint32_t now_ms);
bool hhkb_hybrid_output_bluetooth(void);
bool hhkb_hybrid_sleep_requested(void);
void hhkb_hybrid_clear_sleep_request(void);
void hhkb_hybrid_request_sleep(void);
void hhkb_hybrid_toggle_output(void);
void hhkb_hybrid_cycle_profile(void);
void hhkb_hybrid_show_profile(void);
void hhkb_hybrid_pair_last_profile(void);
void hhkb_hybrid_toggle_power(void);
bool hhkb_hybrid_take_button_wake(void);
uint32_t hhkb_hybrid_button_wait_ms(uint32_t now_ms);
void hhkb_hybrid_enter_stop(void);
void hhkb_hybrid_resume_after_stop(bool wake);
void hhkb_hybrid_off_service(uint32_t now_ms);
void hhkb_hybrid_sleep_radio_idle(void);
void hhkb_hybrid_wake_radio(void);
bool hhkb_hybrid_take_link_wake(void);
bool hhkb_hybrid_disconnected_sleep_due(uint32_t now_ms);
// Critical-battery STOP: PC5 is masked; USB wake resets STM32.
void hhkb_hybrid_enter_deep_sleep(void);
bool hhkb_hybrid_off_is_deep(void);
bool hhkb_hybrid_critical_battery(void);
void hhkb_hybrid_critical_battery_poll(uint32_t now_ms);

void hhkb_hybrid_set_sleeping(bool sleeping);

void hhkb_hybrid_set_wake_task(void *task);
void hhkb_hybrid_sleep_exti(bool enabled);
void hhkb_hybrid_notify_wake_from_isr(void);
uint32_t hhkb_hybrid_indicator_wait_ms(uint32_t now_ms);

void hhkb_hybrid_refresh_indicators(void);
