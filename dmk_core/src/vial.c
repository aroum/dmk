#include "vial.h"
#include "FreeRTOS.h"
#include "keyboard.h"
#include "keys.h"
#include "matrix.h"
#include "queue.h"
#include "rgb.h"
#include "task.h"

#ifdef VIAL
#include "usb.h"
#include "vial_generated.h"
#include <string.h>

// Magic includes matrix dimensions so stale flash data is ignored after config changes
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
static const pin_t _enc_a_tmp[] = ENCODER_PINS_A;
#define _NUM_ENC ((uint32_t)(sizeof(_enc_a_tmp) / sizeof(_enc_a_tmp[0])))
#else
#define _NUM_ENC 0u
#endif
#define VIAL_EEPROM_MAGIC                                                                                              \
    (0x4D4B5649u ^ ((uint32_t)(NUM_ROWS) << 24) ^ ((uint32_t)(NUM_COLS) << 16) ^ ((uint32_t)(NUM_KEYS) << 8) ^         \
     _NUM_ENC ^ 0xFEEDC00Lu)

#define DYNAMIC_KEYMAP_MACRO_COUNT 16
#define DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE 2048

uint8_t dynamic_macro_buffer[DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE];
vial_combo_entry_t vial_combos[VIAL_COMBO_ENTRIES];

#ifdef MCU_milandr
#include "MDR32FxQI_eeprom.h"
#include "MDR32FxQI_rst_clk.h"

#define EEPROM_STORAGE_ADDR 0x0801F000
#endif

// Dynamic keymap array in RAM, initialized from static keymap on startup
uint32_t dynamic_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][NUM_KEYS];
uint32_t dynamic_encoder_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][4][2];

// Declarations of static layout structures and counts from keyboard.c/config.h
extern const uint32_t keymap[][NUM_KEYS];
extern const size_t keymap_layers;
extern int16_t keyboard_get_flat_key_index(uint8_t row, uint8_t col);

#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
extern uint32_t encoder_keymap_get_static(uint8_t lyr, uint8_t encoder_idx, uint8_t direction);
extern uint8_t encoder_get_count(void);
#endif

// Enums matching the VIA protocol specification
enum {
    VIA_GET_PROTOCOL_VERSION = 0x01,
    VIA_GET_KEYBOARD_VALUE = 0x02,
    VIA_SET_KEYBOARD_VALUE = 0x03,
    VIA_DYNAMIC_KEYMAP_GET_KEYCODE = 0x04,
    VIA_DYNAMIC_KEYMAP_SET_KEYCODE = 0x05,
    VIA_DYNAMIC_KEYMAP_RESET = 0x06,
    VIA_CUSTOM_SET_VALUE = 0x07,
    VIA_CUSTOM_GET_VALUE = 0x08,
    VIA_CUSTOM_SAVE = 0x09,
    VIA_EEPROM_RESET = 0x0A,
    VIA_BOOTLOADER_JUMP = 0x0B,
    VIA_DYNAMIC_KEYMAP_MACRO_GET_COUNT = 0x0C,
    VIA_DYNAMIC_KEYMAP_MACRO_GET_BUFFER_SIZE = 0x0D,
    VIA_DYNAMIC_KEYMAP_MACRO_GET_BUFFER = 0x0E,
    VIA_DYNAMIC_KEYMAP_MACRO_SET_BUFFER = 0x0F,
    VIA_DYNAMIC_KEYMAP_MACRO_RESET = 0x10,
    VIA_DYNAMIC_KEYMAP_GET_LAYER_COUNT = 0x11,
    VIA_DYNAMIC_KEYMAP_GET_BUFFER = 0x12,
    VIA_DYNAMIC_KEYMAP_SET_BUFFER = 0x13,
    VIA_DYNAMIC_KEYMAP_GET_ENCODER = 0x14,
    VIA_DYNAMIC_KEYMAP_SET_ENCODER = 0x15,
    VIA_VIAL = 0xFE
};

// Enums matching the Vial protocol specification
enum {
    VIAL_GET_KEYBOARD_ID = 0x00,
    VIAL_GET_SIZE = 0x01,
    VIAL_GET_KEYBOARD_DEF = 0x02,
    VIAL_GET_ENCODER = 0x03,
    VIAL_SET_ENCODER = 0x04,
    VIAL_GET_UNLOCK_STATUS = 0x05,
    VIAL_UNLOCK_START = 0x06,
    VIAL_UNLOCK_POLL = 0x07,
    VIAL_LOCK = 0x08,
    VIAL_QMK_SETTINGS_QUERY = 0x09,
    VIAL_QMK_SETTINGS_GET = 0x0A,
    VIAL_QMK_SETTINGS_SET = 0x0B,
    VIAL_QMK_SETTINGS_RESET = 0x0C,
    VIAL_DYNAMIC_ENTRY_OP = 0x0D
};

// Subcommands for GET_KEYBOARD_VALUE
enum {
    VIA_SUB_UPTIME = 0x01,
    VIA_SUB_LAYOUT_OPTIONS = 0x02,
    VIA_SUB_SWITCH_MATRIX_STATE = 0x03,
    VIA_SUB_FIRMWARE_VERSION = 0x04
};

// Helper function to map DMK 32-bit keycodes to VIA 16-bit keycodes
uint16_t to_via_keycode(uint32_t dmk_key) {
    if (dmk_key == K_TRNS) {
        return 0x0001; // Transparent in VIA
    }

    // Map DMK internal RGB keycodes to standard QMK underglow keycodes (0x7820 - 0x782A)
    if (dmk_key >= K_RGB_TOGG && dmk_key <= K_RGB_SPD) {
        return 0x7820 + (dmk_key - K_RGB_TOGG);
    }

    // Map DMK internal Macro keycodes (0xC0 - 0xDF) to QMK standard macro keycodes (0x7700 - 0x771F)
    if (dmk_key >= 0xC0 && dmk_key <= 0xDF) {
        return 0x7700 + (dmk_key - 0xC0);
    }

    // Momentary layer: DMK_MO
    if ((dmk_key & 0xFF000000) == DMK_MO) {
        uint8_t layer = dmk_key & 0xFF;
        return 0x5220 | layer;
    }

    // Toggle layer: DMK_TG
    if ((dmk_key & 0xFF000000) == DMK_TG) {
        uint8_t layer = dmk_key & 0xFF;
        return 0x5260 | layer;
    }

    // Tap-Hold: DMK_HT
    if ((dmk_key & 0xFF000000) == DMK_HT) {
        uint8_t layer_or_mod = (dmk_key >> 8) & 0xFF;
        uint8_t kc = dmk_key & 0xFF;
        if (layer_or_mod < 32) {
            return 0x4000 | (layer_or_mod << 8) | kc;
        } else {
            // Tap-Hold Mod
            uint8_t via_mod = 0;
            if (layer_or_mod & MOD_LCTRL)
                via_mod |= 0x01;
            if (layer_or_mod & MOD_LSHIFT)
                via_mod |= 0x02;
            if (layer_or_mod & MOD_LALT)
                via_mod |= 0x04;
            if (layer_or_mod & MOD_LGUI)
                via_mod |= 0x08;
            if (layer_or_mod & MOD_RCTRL)
                via_mod |= 0x11;
            if (layer_or_mod & MOD_RSHIFT)
                via_mod |= 0x12;
            if (layer_or_mod & MOD_RALT)
                via_mod |= 0x14;
            if (layer_or_mod & MOD_RGUI)
                via_mod |= 0x18;
            return 0x2000 | (via_mod << 8) | kc;
        }
    }

    // Key with Modifier: DMK_MK
    if ((dmk_key & 0xFF000000) == DMK_MK) {
        uint8_t mod_mask = (dmk_key >> 8) & 0xFF;
        uint8_t kc = dmk_key & 0xFF;
        uint8_t via_mod = 0;
        if (mod_mask & MOD_LCTRL)
            via_mod |= 0x01;
        if (mod_mask & MOD_LSHIFT)
            via_mod |= 0x02;
        if (mod_mask & MOD_LALT)
            via_mod |= 0x04;
        if (mod_mask & MOD_LGUI)
            via_mod |= 0x08;
        if (mod_mask & MOD_RCTRL)
            via_mod |= 0x11;
        if (mod_mask & MOD_RSHIFT)
            via_mod |= 0x12;
        if (mod_mask & MOD_RALT)
            via_mod |= 0x14;
        if (mod_mask & MOD_RGUI)
            via_mod |= 0x18;
        return 0x0100 | (via_mod << 8) | kc;
    }

    // Raw momentary activation (L_0 <= dmk_key <= L_15)
    if (dmk_key >= L_0 && dmk_key <= L_15) {
        return 0x5220 | (dmk_key - L_0);
    }

    // MIDI keycodes range (0x7100 - 0x71FF)
    if (dmk_key >= 0x7100 && dmk_key <= 0x71FF) {
        return dmk_key;
    }

    // Custom CC fixed val / toggle range (0x7A00 - 0x7AFF)
    if (dmk_key >= 0x7A00 && dmk_key <= 0x7AFF) {
        return dmk_key;
    }

    // Custom CC keycodes range (0x7E00 - 0x7E3F)
    if (dmk_key >= 0x7E00 && dmk_key <= 0x7E3F) {
        return dmk_key;
    }

    // Custom CC keycodes range (0x7E40 - 0x7F3F)
    if (dmk_key >= 0x7E40 && dmk_key <= 0x7F3F) {
        return dmk_key;
    }

    // Normal standard keycode
    if (dmk_key <= 0xFF) {
        return dmk_key;
    }

    return 0x0000; // KC_NO
}

// Helper function to map VIA 16-bit keycodes to DMK 32-bit keycodes
uint32_t from_via_keycode(uint16_t via_key) {
    if (via_key == 0x0000)
        return K_NULL;
    if (via_key == 0x0001)
        return K_TRNS;

    // MIDI keycodes range (0x7100 - 0x71FF)
    if (via_key >= 0x7100 && via_key <= 0x71FF) {
        return via_key;
    }

    // Custom CC fixed val / toggle range (0x7A00 - 0x7AFF)
    if (via_key >= 0x7A00 && via_key <= 0x7AFF) {
        return via_key;
    }

    // Custom CC keycodes range (0x7E00 - 0x7E3F)
    if (via_key >= 0x7E00 && via_key <= 0x7E3F) {
        return via_key;
    }

    // Custom CC keycodes range (0x7E40 - 0x7F3F)
    if (via_key >= 0x7E40 && via_key <= 0x7F3F) {
        return via_key;
    }

    // Map QMK standard underglow keycodes (0x7820 - 0x782A) to DMK internal RGB keycodes
    if (via_key >= 0x7820 && via_key <= 0x782A) {
        return K_RGB_TOGG + (via_key - 0x7820);
    }

    // Map QMK standard macro keycodes (0x7700 - 0x771F) to DMK internal Macro keycodes
    if (via_key >= 0x7700 && via_key <= 0x771F) {
        return 0xC0 + (via_key - 0x7700);
    }

    // Normal standard keycode
    if (via_key >= 0x0002 && via_key <= 0x00FF) {
        return via_key;
    }

    // Key with modifier
    if (via_key >= 0x0100 && via_key <= 0x1FFF) {
        uint8_t via_mod = (via_key >> 8) & 0xFF;
        uint8_t kc = via_key & 0xFF;
        uint8_t dmk_mod = 0;
        if (via_mod & 0x01)
            dmk_mod |= MOD_LCTRL;
        if (via_mod & 0x02)
            dmk_mod |= MOD_LSHIFT;
        if (via_mod & 0x04)
            dmk_mod |= MOD_LALT;
        if (via_mod & 0x08)
            dmk_mod |= MOD_LGUI;
        if (via_mod & 0x11)
            dmk_mod |= MOD_RCTRL;
        if (via_mod & 0x12)
            dmk_mod |= MOD_RSHIFT;
        if (via_mod & 0x14)
            dmk_mod |= MOD_RALT;
        if (via_mod & 0x18)
            dmk_mod |= MOD_RGUI;
        return MK(dmk_mod, kc);
    }

    // Tap-Hold Mod
    if (via_key >= 0x2000 && via_key <= 0x3FFF) {
        uint8_t via_mod = (via_key >> 8) & 0x1F;
        uint8_t kc = via_key & 0xFF;
        uint8_t dmk_mod = 0;
        if (via_mod & 0x01)
            dmk_mod |= MOD_LCTRL;
        if (via_mod & 0x02)
            dmk_mod |= MOD_LSHIFT;
        if (via_mod & 0x04)
            dmk_mod |= MOD_LALT;
        if (via_mod & 0x08)
            dmk_mod |= MOD_LGUI;
        if (via_mod & 0x11)
            dmk_mod |= MOD_RCTRL;
        if (via_mod & 0x12)
            dmk_mod |= MOD_RSHIFT;
        if (via_mod & 0x14)
            dmk_mod |= MOD_RALT;
        if (via_mod & 0x18)
            dmk_mod |= MOD_RGUI;
        return HT(dmk_mod, kc);
    }

    // Tap-Hold Layer
    if (via_key >= 0x4000 && via_key <= 0x4FFF) {
        uint8_t layer = (via_key >> 8) & 0x0F;
        uint8_t kc = via_key & 0xFF;
        return HT(layer, kc);
    }

    // Layer Toggle
    if (via_key >= 0x5260 && via_key <= 0x527F) {
        return TG(via_key & 0x0F);
    }

    // Layer Momentary
    if (via_key >= 0x5220 && via_key <= 0x523F) {
        return MO(via_key & 0x0F);
    }

    return K_NULL;
}

#ifdef MCU_milandr
static void vial_eeprom_load(void) {
    uint32_t magic = *(volatile uint32_t *)EEPROM_STORAGE_ADDR;
    if (magic == VIAL_EEPROM_MAGIC) {
        // Load keymap from EEPROM
        uint32_t addr = EEPROM_STORAGE_ADDR + 4;
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t k = 0; k < NUM_KEYS; k++) {
                dynamic_keymap[l][k] = *(volatile uint32_t *)addr;
                addr += 4;
            }
        }
        // Load encoder keymap from EEPROM
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t e = 0; e < 4; e++) {
                for (size_t d = 0; d < 2; d++) {
                    dynamic_encoder_keymap[l][e][d] = *(volatile uint32_t *)addr;
                    addr += 4;
                }
            }
        }
        // Load RGB config from EEPROM
        uint8_t rgb_cfg[8];
        uint32_t w1 = *(volatile uint32_t *)addr;
        addr += 4;
        uint32_t w2 = *(volatile uint32_t *)addr;
        addr += 4;
        memcpy(&rgb_cfg[0], &w1, 4);
        memcpy(&rgb_cfg[4], &w2, 4);
        rgb_set_config(rgb_cfg);

        // Load macro buffer
        for (size_t i = 0; i < DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE; i += 4) {
            uint32_t val = *(volatile uint32_t *)addr;
            memcpy(&dynamic_macro_buffer[i], &val, 4);
            addr += 4;
        }

        // Load combos
        for (size_t i = 0; i < sizeof(vial_combos); i += 4) {
            uint32_t val = *(volatile uint32_t *)addr;
            memcpy(((uint8_t *)vial_combos) + i, &val, 4);
            addr += 4;
        }
    } else {
        // Initialize dynamic keymap from static keymap
        size_t layers = keymap_layers;
        if (layers > DYNAMIC_KEYMAP_MAX_LAYERS) {
            layers = DYNAMIC_KEYMAP_MAX_LAYERS;
        }
        for (size_t l = 0; l < layers; l++) {
            for (size_t k = 0; k < NUM_KEYS; k++) {
                dynamic_keymap[l][k] = keymap[l][k];
            }
        }
        // Initialize dynamic encoder keymap from static encoder keymap
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t e = 0; e < 4; e++) {
                for (size_t d = 0; d < 2; d++) {
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
                    if (l < layers && e < encoder_get_count()) {
                        dynamic_encoder_keymap[l][e][d] = encoder_keymap_get_static(l, e, d);
                    } else {
                        dynamic_encoder_keymap[l][e][d] = K_TRNS;
                    }
#else
                    dynamic_encoder_keymap[l][e][d] = K_TRNS;
#endif
                }
            }
        }
        memset(dynamic_macro_buffer, 0, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
        memset(vial_combos, 0, sizeof(vial_combos));
    }
}

static __RAMFUNC void vial_eeprom_save(void) {
    // 1. Get the RGB config and prepare data before entering critical section/flash access
    uint8_t rgb_cfg[8];
    rgb_get_config(rgb_cfg);
    uint32_t w1 =
        ((uint32_t)rgb_cfg[3] << 24) | ((uint32_t)rgb_cfg[2] << 16) | ((uint32_t)rgb_cfg[1] << 8) | rgb_cfg[0];
    uint32_t w2 =
        ((uint32_t)rgb_cfg[7] << 24) | ((uint32_t)rgb_cfg[6] << 16) | ((uint32_t)rgb_cfg[5] << 8) | rgb_cfg[4];

    // 2. Enable EEPROM clock and latency before critical section (while executing from Flash is still okay)
    RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
    EEPROM_SetLatency(EEPROM_Latency_3);

    // 3. Enter critical section - disable interrupts completely using PRIMASK
    __disable_irq();

    // 4. Erase the page first (runs from RAM)
    EEPROM_ErasePage(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select);

    // 5. Program the magic word (runs from RAM)
    EEPROM_ProgramWord(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select, VIAL_EEPROM_MAGIC);

    // 6. Program the keymap (runs from RAM)
    uint32_t addr = EEPROM_STORAGE_ADDR + 4;
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t k = 0; k < NUM_KEYS; k++) {
            EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, dynamic_keymap[l][k]);
            addr += 4;
        }
    }
    // Program the encoder keymap (runs from RAM)
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t e = 0; e < 4; e++) {
            for (size_t d = 0; d < 2; d++) {
                EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, dynamic_encoder_keymap[l][e][d]);
                addr += 4;
            }
        }
    }

    // 7. Program the RGB config (runs from RAM)
    EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, w1);
    addr += 4;
    EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, w2);
    addr += 4;

    // 8. Program the macro buffer (runs from RAM)
    for (size_t i = 0; i < DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE; i += 4) {
        uint32_t val;
        memcpy(&val, &dynamic_macro_buffer[i], 4);
        EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, val);
        addr += 4;
    }

    // 9. Program combos
    for (size_t i = 0; i < sizeof(vial_combos); i += 4) {
        uint32_t val;
        memcpy(&val, ((uint8_t *)vial_combos) + i, 4);
        EEPROM_ProgramWord(addr, EEPROM_Main_Bank_Select, val);
        addr += 4;
    }

    // 10. Update cache (runs from RAM)
    EEPROM_UpdateDCache();

    // 11. Re-enable interrupts
    __enable_irq();
}
#endif

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/flash.h"
#include "hardware/sync.h"

#pragma pack(push, 1)
typedef struct {
    uint32_t magic;
    uint32_t keymap[DYNAMIC_KEYMAP_MAX_LAYERS][NUM_KEYS];
    uint32_t encoder_keymap[DYNAMIC_KEYMAP_MAX_LAYERS][4][2];
    uint8_t rgb_cfg[8];
    uint8_t macro_buffer[DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE];
    vial_combo_entry_t combos[VIAL_COMBO_ENTRIES];
} vial_storage_data_t;
#pragma pack(pop)

typedef union {
    vial_storage_data_t data;
    uint8_t padding[((sizeof(vial_storage_data_t) + 255) / 256) * 256];
} vial_storage_t;

#define FLASH_STORAGE_OFFSET (PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE)

static void vial_eeprom_load(void) {
    const vial_storage_t *flash_storage = (const vial_storage_t *)(XIP_BASE + FLASH_STORAGE_OFFSET);
    if (flash_storage->data.magic == VIAL_EEPROM_MAGIC) {
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t k = 0; k < NUM_KEYS; k++) {
                dynamic_keymap[l][k] = flash_storage->data.keymap[l][k];
            }
        }
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t e = 0; e < 4; e++) {
                for (size_t d = 0; d < 2; d++) {
                    dynamic_encoder_keymap[l][e][d] = flash_storage->data.encoder_keymap[l][e][d];
                }
            }
        }
        rgb_set_config(flash_storage->data.rgb_cfg);
        memcpy(dynamic_macro_buffer, flash_storage->data.macro_buffer, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
        memcpy(vial_combos, flash_storage->data.combos, sizeof(vial_combos));
    } else {
        size_t layers = keymap_layers;
        if (layers > DYNAMIC_KEYMAP_MAX_LAYERS) {
            layers = DYNAMIC_KEYMAP_MAX_LAYERS;
        }
        for (size_t l = 0; l < layers; l++) {
            for (size_t k = 0; k < NUM_KEYS; k++) {
                dynamic_keymap[l][k] = keymap[l][k];
            }
        }
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t e = 0; e < 4; e++) {
                for (size_t d = 0; d < 2; d++) {
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
                    if (l < layers && e < encoder_get_count()) {
                        dynamic_encoder_keymap[l][e][d] = encoder_keymap_get_static(l, e, d);
                    } else {
                        dynamic_encoder_keymap[l][e][d] = K_TRNS;
                    }
#else
                    dynamic_encoder_keymap[l][e][d] = K_TRNS;
#endif
                }
            }
        }
        memset(dynamic_macro_buffer, 0, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
        memset(vial_combos, 0, sizeof(vial_combos));
    }
}

static void vial_eeprom_save(void) {
    static vial_storage_t temp_buf;
    memset(&temp_buf, 0, sizeof(temp_buf));

    temp_buf.data.magic = VIAL_EEPROM_MAGIC;
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t k = 0; k < NUM_KEYS; k++) {
            temp_buf.data.keymap[l][k] = dynamic_keymap[l][k];
        }
    }
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t e = 0; e < 4; e++) {
            for (size_t d = 0; d < 2; d++) {
                temp_buf.data.encoder_keymap[l][e][d] = dynamic_encoder_keymap[l][e][d];
            }
        }
    }
    rgb_get_config(temp_buf.data.rgb_cfg);
    memcpy(temp_buf.data.macro_buffer, dynamic_macro_buffer, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
    memcpy(temp_buf.data.combos, vial_combos, sizeof(vial_combos));

    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(FLASH_STORAGE_OFFSET, (const uint8_t *)&temp_buf, sizeof(temp_buf));
    restore_interrupts(ints);
}
#endif

void vial_init(void) {
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
    vial_eeprom_load();
#else
    size_t layers = keymap_layers;
    if (layers > DYNAMIC_KEYMAP_MAX_LAYERS) {
        layers = DYNAMIC_KEYMAP_MAX_LAYERS;
    }
    for (size_t l = 0; l < layers; l++) {
        for (size_t k = 0; k < NUM_KEYS; k++) {
            dynamic_keymap[l][k] = keymap[l][k];
        }
    }
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t e = 0; e < 4; e++) {
            for (size_t d = 0; d < 2; d++) {
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
                if (l < layers && e < encoder_get_count()) {
                    dynamic_encoder_keymap[l][e][d] = encoder_keymap_get_static(l, e, d);
                } else {
                    dynamic_encoder_keymap[l][e][d] = K_TRNS;
                }
#else
                dynamic_encoder_keymap[l][e][d] = K_TRNS;
#endif
            }
        }
    }
#endif
}

void vial_process_packet(uint8_t const *request, uint8_t *response) {
    // Fill response with request values as a default/fallback
    memcpy(response, request, 32);

    uint8_t command_id = request[0];
    switch (command_id) {
    case VIA_GET_PROTOCOL_VERSION: {
        // Return VIA protocol version 9 in Big Endian (0x0009)
        response[1] = 0x00;
        response[2] = 0x09;
        break;
    }

    case VIA_GET_KEYBOARD_VALUE: {
        uint8_t sub = request[1];
        switch (sub) {
        case VIA_SUB_UPTIME: {
            uint32_t uptime = (uint32_t)(xTaskGetTickCount() * (1000 / configTICK_RATE_HZ));
            response[2] = (uptime >> 24) & 0xFF;
            response[3] = (uptime >> 16) & 0xFF;
            response[4] = (uptime >> 8) & 0xFF;
            response[5] = uptime & 0xFF;
            break;
        }
        case VIA_SUB_LAYOUT_OPTIONS: {
            response[2] = 0;
            response[3] = 0;
            response[4] = 0;
            response[5] = 0;
            break;
        }
        case VIA_SUB_SWITCH_MATRIX_STATE: {
            uint8_t row_size = (NUM_COLS + 7) / 8;
            memset(&response[2], 0, 30);
            for (uint8_t r = 0; r < NUM_ROWS; r++) {
                for (uint8_t c = 0; c < NUM_COLS; c++) {
                    if (matrix_is_pressed(r, c)) {
                        uint8_t byte_idx = row_size - 1 - (c / 8);
                        uint8_t bit_idx = c % 8;
                        response[2 + (r * row_size) + byte_idx] |= (1 << bit_idx);
                    }
                }
            }
            break;
        }
        case VIA_SUB_FIRMWARE_VERSION: {
            // 0x00000001
            response[2] = 0;
            response[3] = 0;
            response[4] = 0;
            response[5] = 1;
            break;
        }
        default:
            break;
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_GET_LAYER_COUNT: {
        response[1] = (uint8_t)keymap_layers;
        break;
    }

    case VIA_DYNAMIC_KEYMAP_GET_KEYCODE: {
        uint8_t layer = request[1];
        uint8_t row = request[2];
        uint8_t col = request[3];
        int16_t ki = keyboard_get_flat_key_index(row, col);
        uint16_t via_key = 0x0001; // transparent default
        if (ki >= 0 && layer < DYNAMIC_KEYMAP_MAX_LAYERS) {
            via_key = to_via_keycode(dynamic_keymap[layer][ki]);
        }
        response[4] = (via_key >> 8) & 0xFF;
        response[5] = via_key & 0xFF;
        break;
    }

    case VIA_DYNAMIC_KEYMAP_SET_KEYCODE: {
        uint8_t layer = request[1];
        uint8_t row = request[2];
        uint8_t col = request[3];
        uint16_t via_key = ((uint16_t)request[4] << 8) | request[5];
        int16_t ki = keyboard_get_flat_key_index(row, col);
        if (ki >= 0 && layer < DYNAMIC_KEYMAP_MAX_LAYERS) {
            dynamic_keymap[layer][ki] = from_via_keycode(via_key);
#ifdef MCU_milandr
            vial_eeprom_save();
#endif
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_GET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        uint16_t start = offset / 2;
        uint8_t count = size / 2;
        for (uint8_t i = 0; i < count; i++) {
            uint16_t idx = start + i;
            uint8_t layer = idx / NUM_KEYS;
            uint8_t key_idx = idx % NUM_KEYS;
            uint16_t via_key = 0x0001; // transparent default
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS) {
                via_key = to_via_keycode(dynamic_keymap[layer][key_idx]);
            }
            response[4 + i * 2] = (via_key >> 8) & 0xFF;
            response[5 + i * 2] = via_key & 0xFF;
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_SET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        uint16_t start = offset / 2;
        uint8_t count = size / 2;
        for (uint8_t i = 0; i < count; i++) {
            uint16_t idx = start + i;
            uint8_t layer = idx / NUM_KEYS;
            uint8_t key_idx = idx % NUM_KEYS;
            uint16_t via_key = ((uint16_t)request[4 + i * 2] << 8) | request[5 + i * 2];
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS) {
                dynamic_keymap[layer][key_idx] = from_via_keycode(via_key);
            }
        }
#ifdef MCU_milandr
        vial_eeprom_save();
#endif
        break;
    }

    case VIA_DYNAMIC_KEYMAP_GET_ENCODER: {
        uint8_t layer = request[1];
        uint8_t idx = request[2];
        uint8_t dir = request[3];
        uint16_t via_key = 0x0001; // transparent default
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
        if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && idx < encoder_get_count() && dir < 2) {
            via_key = to_via_keycode(dynamic_encoder_keymap[layer][idx][dir]);
        }
#endif
        response[4] = (via_key >> 8) & 0xFF;
        response[5] = via_key & 0xFF;
        break;
    }

    case VIA_DYNAMIC_KEYMAP_SET_ENCODER: {
        uint8_t layer = request[1];
        uint8_t idx = request[2];
        uint8_t dir = request[3];
        uint16_t via_key = ((uint16_t)request[4] << 8) | request[5];
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
        if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && idx < encoder_get_count() && dir < 2) {
            dynamic_encoder_keymap[layer][idx][dir] = from_via_keycode(via_key);
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
            vial_eeprom_save();
#endif
        }
#endif
        break;
    }

    case VIA_EEPROM_RESET: {
        size_t layers = keymap_layers;
        if (layers > DYNAMIC_KEYMAP_MAX_LAYERS) {
            layers = DYNAMIC_KEYMAP_MAX_LAYERS;
        }
        for (size_t l = 0; l < layers; l++) {
            for (size_t k = 0; k < NUM_KEYS; k++) {
                dynamic_keymap[l][k] = keymap[l][k];
            }
        }
        for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
            for (size_t e = 0; e < 4; e++) {
                for (size_t d = 0; d < 2; d++) {
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
                    if (l < layers && e < encoder_get_count()) {
                        dynamic_encoder_keymap[l][e][d] = encoder_keymap_get_static(l, e, d);
                    } else {
                        dynamic_encoder_keymap[l][e][d] = K_TRNS;
                    }
#else
                    dynamic_encoder_keymap[l][e][d] = K_TRNS;
#endif
                }
            }
        }
#ifdef MCU_milandr
        vial_eeprom_save();
#endif
        break;
    }

    case VIA_CUSTOM_GET_VALUE: { // 0x08
        uint8_t value_id = request[1];
        switch (value_id) {
        case 0x80: // id_qmk_rgblight_brightness
            response[2] = rgb_get_brightness();
            break;
        case 0x81: // id_qmk_rgblight_effect
            response[2] = rgb_get_mode();
            break;
        case 0x82: // id_qmk_rgblight_effect_speed
            response[2] = rgb_get_speed();
            break;
        case 0x83: // id_qmk_rgblight_color
            response[2] = rgb_get_hue();
            response[3] = rgb_get_sat();
            break;
        default:
            break;
        }
        break;
    }

    case VIA_CUSTOM_SET_VALUE: { // 0x07
        uint8_t value_id = request[1];
        switch (value_id) {
        case 0x80: // id_qmk_rgblight_brightness
            rgb_set_brightness(request[2]);
            break;
        case 0x81: // id_qmk_rgblight_effect
            rgb_set_mode(request[2]);
            break;
        case 0x82: // id_qmk_rgblight_effect_speed
            rgb_set_speed(request[2]);
            break;
        case 0x83: // id_qmk_rgblight_color
            rgb_set_color(request[2], request[3]);
            break;
        default:
            break;
        }
        break;
    }

    case VIA_CUSTOM_SAVE: {
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
        vial_eeprom_save();
#endif
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_GET_COUNT: {
        response[1] = DYNAMIC_KEYMAP_MACRO_COUNT;
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_GET_BUFFER_SIZE: {
        response[1] = (DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE >> 8) & 0xFF;
        response[2] = DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE & 0xFF;
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_GET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        if (offset + size <= DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE) {
            memcpy(&response[4], &dynamic_macro_buffer[offset], size);
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_SET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        if (offset + size <= DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE) {
            memcpy(&dynamic_macro_buffer[offset], &request[4], size);
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
            vial_eeprom_save();
#endif
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_RESET: {
        memset(dynamic_macro_buffer, 0, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
        vial_eeprom_save();
#endif
        break;
    }

    case VIA_VIAL: {
        uint8_t vial_cmd = request[1];
        switch (vial_cmd) {
        case VIAL_GET_KEYBOARD_ID: {
            // Protocol version (4 bytes, Little Endian, e.g. v6)
            response[0] = 0x06;
            response[1] = 0x00;
            response[2] = 0x00;
            response[3] = 0x00;
            // Keyboard ID (8 bytes)
            memcpy(&response[4], vial_keyboard_id, 8);
            break;
        }

        case VIAL_GET_SIZE: {
            // Size of compressed keyboard layout JSON definition (4 bytes, Little Endian)
            uint32_t def_size = (uint32_t)vial_keyboard_def_size;
            response[0] = def_size & 0xFF;
            response[1] = (def_size >> 8) & 0xFF;
            response[2] = (def_size >> 16) & 0xFF;
            response[3] = (def_size >> 24) & 0xFF;
            break;
        }

        case VIAL_GET_KEYBOARD_DEF: {
            uint16_t page = ((uint16_t)request[3] << 8) | request[2]; // Little Endian
            size_t start = (size_t)page * 32;
            size_t bytes_to_copy = 32;

            memset(response, 0xFF, 32);
            if (start < vial_keyboard_def_size) {
                if (start + bytes_to_copy > vial_keyboard_def_size) {
                    bytes_to_copy = vial_keyboard_def_size - start;
                }
                memcpy(response, &vial_keyboard_def[start], bytes_to_copy);
            }
            break;
        }

        case VIAL_GET_UNLOCK_STATUS: {
            memset(response, 0xFF, 32);
            response[0] = 1; // 1 = Unlocked
            response[1] = 0; // 0 = Unlock not in progress
            break;
        }

        case VIAL_UNLOCK_START: {
            memset(response, 0xFF, 32);
            response[0] = 1; // Unlocked
            response[1] = 0; // Not in progress
            break;
        }

        case VIAL_UNLOCK_POLL: {
            memset(response, 0xFF, 32);
            response[0] = 1; // Unlocked
            response[1] = 0; // Not in progress
            response[2] = 0; // Success code
            break;
        }

        case VIAL_LOCK: {
            memset(response, 0xFF, 32);
            break;
        }

        case VIAL_QMK_SETTINGS_QUERY:
        case VIAL_QMK_SETTINGS_GET:
        case VIAL_QMK_SETTINGS_SET:
        case VIAL_QMK_SETTINGS_RESET: {
            memset(response, 0xFF, 32);
            break;
        }

        case VIAL_DYNAMIC_ENTRY_OP: {
            uint8_t op = request[2];
            memset(response, 0x00, 32);
            if (op == 0) {                        // DynamicVialGetNumberOfEntries
                response[0] = 0;                  // Tap dance entries
                response[1] = VIAL_COMBO_ENTRIES; // Combo entries
                response[2] = 0;                  // Key override entries
            } else if (op == 3) {                 // dynamic_vial_combo_get
                uint8_t idx = request[3];
                response[0] = (idx < VIAL_COMBO_ENTRIES) ? 0 : 1;
                if (idx < VIAL_COMBO_ENTRIES) {
                    const uint8_t *src = (const uint8_t *)&vial_combos[idx];
                    for (size_t i = 0; i < sizeof(vial_combo_entry_t); i++) {
                        response[1 + i] = src[i];
                    }
                }
            } else if (op == 4) { // dynamic_vial_combo_set
                uint8_t idx = request[3];
                response[0] = (idx < VIAL_COMBO_ENTRIES) ? 0 : 1;
                if (idx < VIAL_COMBO_ENTRIES) {
                    uint8_t *dst = (uint8_t *)&vial_combos[idx];
                    for (size_t i = 0; i < sizeof(vial_combo_entry_t); i++) {
                        dst[i] = request[4 + i];
                    }
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
                    vial_eeprom_save();
#endif
                }
            } else {
                memset(response, 0xFF, 32);
            }
            break;
        }

        case VIAL_GET_ENCODER: {
            uint8_t layer = request[2];
            uint8_t idx = request[3];
            uint16_t kc1 = 0x0001; // transparent default
            uint16_t kc2 = 0x0001;
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && idx < encoder_get_count()) {
                kc1 = to_via_keycode(dynamic_encoder_keymap[layer][idx][0]);
                kc2 = to_via_keycode(dynamic_encoder_keymap[layer][idx][1]);
            }
#endif
            response[0] = (kc1 >> 8) & 0xFF;
            response[1] = kc1 & 0xFF;
            response[2] = (kc2 >> 8) & 0xFF;
            response[3] = kc2 & 0xFF;
            break;
        }

        case VIAL_SET_ENCODER: {
            uint8_t layer = request[2];
            uint8_t idx = request[3];
            uint8_t dir = request[4];
            uint16_t via_key = ((uint16_t)request[5] << 8) | request[6];
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && idx < encoder_get_count() && dir < 2) {
                dynamic_encoder_keymap[layer][idx][dir] = from_via_keycode(via_key);
#if defined(MCU_milandr) || defined(MCU_rp2040) || defined(MCU_rp2350)
                vial_eeprom_save();
#endif
            }
#endif
            break;
        }

        default:
            // Return unhandled response (filled with 0xFF in QMK/Vial)
            memset(response, 0xFF, 32);
            break;
        }
        break;
    }

    default:
        response[0] = 0xFF; // Unhandled command
        break;
    }
}

static uint16_t ascii_to_keycode(char c, bool *shift) {
    *shift = false;
    if (c >= 'a' && c <= 'z')
        return (c - 'a' + 0x04);
    if (c >= 'A' && c <= 'Z') {
        *shift = true;
        return (c - 'A' + 0x04);
    }
    if (c >= '1' && c <= '9')
        return (c - '1' + 0x1E);
    if (c == '0')
        return 0x27;

    switch (c) {
    case ' ':
        return 0x2C;
    case '\n':
        return 0x28;
    case '\t':
        return 0x2B;
    case '-':
        return 0x2D;
    case '_':
        *shift = true;
        return 0x2D;
    case '=':
        return 0x2E;
    case '+':
        *shift = true;
        return 0x2E;
    case '[':
        return 0x2F;
    case '{':
        *shift = true;
        return 0x2F;
    case ']':
        return 0x30;
    case '}':
        *shift = true;
        return 0x30;
    case '\\':
        return 0x31;
    case '|':
        *shift = true;
        return 0x31;
    case ';':
        return 0x33;
    case ':':
        *shift = true;
        return 0x33;
    case '\'':
        return 0x34;
    case '"':
        *shift = true;
        return 0x34;
    case '`':
        return 0x35;
    case '~':
        *shift = true;
        return 0x35;
    case ',':
        return 0x36;
    case '<':
        *shift = true;
        return 0x36;
    case '.':
        return 0x37;
    case '>':
        *shift = true;
        return 0x37;
    case '/':
        return 0x38;
    case '?':
        *shift = true;
        return 0x38;
    default:
        return 0;
    }
}

static void send_event(uint16_t kc, bool pressed) {
    extern QueueHandle_t usb_queue;
    key_event_t event = {kc, pressed ? 1 : 0};
    xQueueSend(usb_queue, &event, 0);
}

static void tap_key(uint16_t kc) {
    send_event(kc, true);
    vTaskDelay(pdMS_TO_TICKS(10));
    send_event(kc, false);
    vTaskDelay(pdMS_TO_TICKS(10));
}

static void send_ascii(char c) {
    bool shift = false;
    uint16_t kc = ascii_to_keycode(c, &shift);
    if (kc) {
        if (shift) {
            send_event(0xE1, true); // Left Shift
            vTaskDelay(pdMS_TO_TICKS(5));
        }
        tap_key(kc);
        if (shift) {
            send_event(0xE1, false);
            vTaskDelay(pdMS_TO_TICKS(5));
        }
    }
}

void vial_macro_send(uint8_t id) {
    if (id >= DYNAMIC_KEYMAP_MACRO_COUNT) {
        return;
    }

    uint32_t offset = 0;
    uint32_t end = DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE;
    while (id > 0) {
        if (offset >= end) {
            return;
        }
        if (dynamic_macro_buffer[offset] == 0) {
            --id;
        }
        ++offset;
    }

    if (offset >= end || dynamic_macro_buffer[offset] == 0) {
        return;
    }

    while (offset < end) {
        uint8_t c = dynamic_macro_buffer[offset++];
        if (c == 0) {
            break;
        }

        if (c == 1) { // SS_QMK_PREFIX
            if (offset >= end)
                break;
            uint8_t action = dynamic_macro_buffer[offset++];
            if (action == 0)
                break;

            if (action == 1 || action == 2 || action == 3) { // SS_TAP_CODE, SS_DOWN_CODE, SS_UP_CODE
                if (offset >= end)
                    break;
                uint8_t kc = dynamic_macro_buffer[offset++];
                if (kc == 0)
                    break;
                if (action == 1)
                    tap_key(kc);
                else if (action == 2)
                    send_event(kc, true);
                else if (action == 3)
                    send_event(kc, false);
            } else if (action == 5 || action == 6 ||
                       action == 7) { // VIAL_MACRO_EXT_TAP, VIAL_MACRO_EXT_DOWN, VIAL_MACRO_EXT_UP
                if (offset + 1 >= end)
                    break;
                uint16_t kc = dynamic_macro_buffer[offset] | ((uint16_t)dynamic_macro_buffer[offset + 1] << 8);
                offset += 2;
                if (kc > 0xFF00) {
                    kc = (kc & 0xFF) << 8;
                }
                if (action == 5)
                    tap_key(kc);
                else if (action == 6)
                    send_event(kc, true);
                else if (action == 7)
                    send_event(kc, false);
            } else if (action == 4) { // SS_DELAY_CODE
                if (offset + 1 >= end)
                    break;
                uint8_t d0 = dynamic_macro_buffer[offset++];
                uint8_t d1 = dynamic_macro_buffer[offset++];
                if (d0 == 0 || d1 == 0)
                    break;
                uint32_t ms = (d0 - 1) + (d1 - 1) * 255;
                vTaskDelay(pdMS_TO_TICKS(ms));
            }
        } else {
            send_ascii((char)c);
        }
    }
}

#endif // VIAL
