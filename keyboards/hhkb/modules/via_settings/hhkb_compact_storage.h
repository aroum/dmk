#ifndef HHKB_COMPACT_STORAGE_H
#define HHKB_COMPACT_STORAGE_H
#include "ec_settings.h"
/* Load is for startup only. Saves are queued by the USB storage task; callers
 * never retain EEPROM pointers. */
bool hhkb_compact_load_cal(hhkb_eeprom_cal_t *cal);
void hhkb_compact_save_cal(const hhkb_eeprom_cal_t *cal);
bool hhkb_compact_legacy_allowed(void);
bool hhkb_compact_writable(void);
bool hhkb_compact_reset(void);
const char *hhkb_compact_status(void);
#endif
