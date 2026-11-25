#include "hooks.h"
#include "keyboard.h"
#include "keys.h"
#include "oneshot.h"
#include "usb.h"
#include "ec_settings.h"
#include <stdint.h>

// QMK/VIA QK_GRAVE_ESCAPE (QK_GESC). Multiple bindings share one held action.
#define HHKB_GESC 0x7c16u
static uint16_t held;
static uint16_t selected;

static bool grave_escape_process(uint32_t key, bool pressed) {
    if (key != HHKB_GESC) return false;
    if (pressed) {
        if (held++ == 0) {
            // GESC defers One-Shot consumption until its output is released.
            oneshot_send_lazy_mods();
            uint8_t mods = usb_get_effective_modifiers();
            uint8_t overrides = hhkb_gesc_overrides();
            bool force_escape = ((overrides & 1) && (mods & (MOD_LALT | MOD_RALT))) ||
                                ((overrides & 2) && (mods & (MOD_LCTRL | MOD_RCTRL))) ||
                                ((overrides & 4) && (mods & (MOD_LGUI | MOD_RGUI))) ||
                                ((overrides & 8) && (mods & (MOD_LSHIFT | MOD_RSHIFT)));
            selected = !force_escape && mods
                     ? K_GRAV : K_ESC;
            static const uint8_t ignored_masks[] = {0, MOD_LALT|MOD_RALT,
                MOD_LCTRL|MOD_RCTRL, MOD_LGUI|MOD_RGUI};
            usb_press_key_suppressed(selected,ignored_masks[hhkb_gesc_ignore()]);
        }
    } else if (held && --held == 0) {
        keyboard_send_key(selected, false);
        oneshot_on_tap_key();
        usb_set_suppressed_modifiers(0);
    }
    return true;
}

DMK_HOOK_SUBSCRIBER(hhkb_grave_escape, .process_key = grave_escape_process);
