#pragma once

#include <stdbool.h>
#include <stdint.h>

void hhkb_hybrid_init(void);
bool hhkb_hybrid_on_battery(void);
bool hhkb_hybrid_button(void);
void hhkb_hybrid_report(uint8_t modifiers, const uint8_t keys[6]);
void hhkb_hybrid_service(void);
bool hhkb_hybrid_request_slot(uint8_t slot, bool pair);
