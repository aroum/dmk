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

// Single unified storage buffer in RAM (eliminates duplicate 3.8KB staging buffer)
vial_storage_t vial_storage __attribute__((aligned(4)));

#ifdef MCU_milandr
#include "MDR32FxQI_eeprom.h"
#include "MDR32FxQI_rst_clk.h"

#define EEPROM_STORAGE_ADDR 0x0801F000
#endif

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

// Helper function to map DMK mouse keycodes to Vial Protocol v6 keycodes
static uint8_t to_via_mouse_keycode(uint8_t dmk_kc) {
    if (dmk_kc >= HID_KEY_MOUSE_UP && dmk_kc <= HID_KEY_MOUSE_BTN5)
        return 0xCD + (dmk_kc - HID_KEY_MOUSE_UP);
    if (dmk_kc >= HID_KEY_MOUSE_WH_UP && dmk_kc <= HID_KEY_MOUSE_ACCEL2)
        return 0xD9 + (dmk_kc - HID_KEY_MOUSE_WH_UP);
    return dmk_kc;
}

// Helper function to map Vial Protocol v6 mouse keycodes to DMK mouse keycodes
static uint8_t from_via_mouse_keycode(uint8_t via_kc) {
    if (via_kc >= 0xCD && via_kc <= 0xD5)
        return HID_KEY_MOUSE_UP + (via_kc - 0xCD);
    if (via_kc >= 0xD9 && via_kc <= 0xDF)
        return HID_KEY_MOUSE_WH_UP + (via_kc - 0xD9);
    return via_kc;
}

static uint8_t to_via_mods(uint8_t dmk_mod) {
    // If given an HID modifier keycode (0xE0..0xE7), map to single VIA modifier value
    switch (dmk_mod) {
    case HID_KEY_CONTROL_LEFT: return 0x01;
    case HID_KEY_SHIFT_LEFT:   return 0x02;
    case HID_KEY_ALT_LEFT:     return 0x04;
    case HID_KEY_GUI_LEFT:     return 0x08;
    case HID_KEY_CONTROL_RIGHT: return 0x11;
    case HID_KEY_SHIFT_RIGHT:   return 0x12;
    case HID_KEY_ALT_RIGHT:     return 0x14;
    case HID_KEY_GUI_RIGHT:     return 0x18;
    default: break;
    }

    uint8_t via_mod = 0;
    if (dmk_mod & MOD_LCTRL)
        via_mod |= 0x01;
    if (dmk_mod & MOD_LSHIFT)
        via_mod |= 0x02;
    if (dmk_mod & MOD_LALT)
        via_mod |= 0x04;
    if (dmk_mod & MOD_LGUI)
        via_mod |= 0x08;
    if (dmk_mod & MOD_RCTRL)
        via_mod |= 0x11;
    if (dmk_mod & MOD_RSHIFT)
        via_mod |= 0x12;
    if (dmk_mod & MOD_RALT)
        via_mod |= 0x14;
    if (dmk_mod & MOD_RGUI)
        via_mod |= 0x18;
    return via_mod;
}

static uint8_t from_via_mods(uint8_t via_mod) {
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
    return dmk_mod;
}

// Helper function to map DMK 32-bit keycodes to VIA 16-bit keycodes
uint16_t to_via_keycode(uint32_t dmk_key) {
    if (dmk_key == K_TRNS)
        return 0x0001;
    if (dmk_key >= K_RGB_TOGG && dmk_key <= K_RGB_RST)
        return 0x7820 + (dmk_key - K_RGB_TOGG);
    if (dmk_key == K_BOOTLOADER)
        return 0x7C00;
    if (dmk_key >= 0xC0 && dmk_key <= 0xDF)
        return 0x7700 + (dmk_key - 0xC0);
    if ((dmk_key & 0xFF000000) == DMK_MO)
        return 0x5220 | (dmk_key & 0xFF);
    if ((dmk_key & 0xFF000000) == DMK_TG)
        return 0x5260 | (dmk_key & 0xFF);

    if ((dmk_key & 0xFF000000) == DMK_HT) {
        uint8_t layer_or_mod = (dmk_key >> 8) & 0xFF;
        uint8_t kc = to_via_mouse_keycode(dmk_key & 0xFF);
        return (layer_or_mod < 32) ? (0x4000 | (layer_or_mod << 8) | kc)
                                   : (0x2000 | (to_via_mods(layer_or_mod) << 8) | kc);
    }
    if ((dmk_key & 0xFF000000) == DMK_MK) {
        return 0x0100 | (to_via_mods((dmk_key >> 8) & 0xFF) << 8) | to_via_mouse_keycode(dmk_key & 0xFF);
    }
    if (dmk_key >= L_0 && dmk_key <= L_15)
        return 0x5220 | (dmk_key - L_0);
    if ((dmk_key >= 0x7100 && dmk_key <= 0x71FF) || (dmk_key >= 0x7A00 && dmk_key <= 0x7AFF) ||
        (dmk_key >= 0x7E00 && dmk_key <= 0x7F3F))
        return dmk_key;
    if (dmk_key >= HID_KEY_MOUSE_UP && dmk_key <= HID_KEY_MOUSE_ACCEL2)
        return to_via_mouse_keycode((uint8_t)dmk_key);
    return (dmk_key <= 0xFF) ? dmk_key : 0x0000;
}

// Helper function to map VIA 16-bit keycodes to DMK 32-bit keycodes
uint32_t from_via_keycode(uint16_t via_key) {
    if (via_key == 0x0000)
        return K_NULL;
    if (via_key == 0x0001)
        return K_TRNS;
    if ((via_key >= 0x7100 && via_key <= 0x71FF) || (via_key >= 0x7A00 && via_key <= 0x7AFF) ||
        (via_key >= 0x7E00 && via_key <= 0x7F3F))
        return via_key;
    if (via_key >= 0x7820 && via_key <= 0x782B)
        return K_RGB_TOGG + (via_key - 0x7820);
    if (via_key == 0x7C00 || via_key == 0x5C00)
        return K_BOOTLOADER;
    if (via_key >= 0x7700 && via_key <= 0x771F)
        return 0xC0 + (via_key - 0x7700);
    if ((via_key >= 0x00CD && via_key <= 0x00D5) || (via_key >= 0x00D9 && via_key <= 0x00DF))
        return from_via_mouse_keycode((uint8_t)via_key);
    if (via_key >= 0x0002 && via_key <= 0x00FF)
        return via_key;
    if (via_key >= 0x0100 && via_key <= 0x1FFF)
        return MK(from_via_mods((via_key >> 8) & 0xFF), from_via_mouse_keycode(via_key & 0xFF));
    if (via_key >= 0x2000 && via_key <= 0x3FFF)
        return HT(from_via_mods((via_key >> 8) & 0x1F), from_via_mouse_keycode(via_key & 0xFF));
    if (via_key >= 0x4000 && via_key <= 0x4FFF)
        return HT((via_key >> 8) & 0x0F, from_via_mouse_keycode(via_key & 0xFF));

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

#define VIAL_EEPROM_SAVE_DELAY_MS 500
static volatile bool s_eeprom_dirty = false;
static volatile TickType_t s_eeprom_dirty_time = 0;

static void vial_storage_init_default(void) {
    memset(&vial_storage, 0, sizeof(vial_storage));
    vial_storage.data.magic = VIAL_EEPROM_MAGIC;
    size_t layers = keymap_layers > DYNAMIC_KEYMAP_MAX_LAYERS ? DYNAMIC_KEYMAP_MAX_LAYERS : keymap_layers;
    for (size_t l = 0; l < layers; l++) {
        for (size_t k = 0; k < NUM_KEYS; k++) {
            dynamic_keymap[l][k] = keymap[l][k];
        }
    }
    for (size_t l = 0; l < DYNAMIC_KEYMAP_MAX_LAYERS; l++) {
        for (size_t e = 0; e < 4; e++) {
            for (size_t d = 0; d < 2; d++) {
#if defined(ENCODER_PINS_A) && defined(ENCODER_PINS_B)
                dynamic_encoder_keymap[l][e][d] =
                    (l < layers && e < encoder_get_count()) ? encoder_keymap_get_static(l, e, d) : K_TRNS;
#else
                dynamic_encoder_keymap[l][e][d] = K_TRNS;
#endif
            }
        }
    }
}

#ifdef MCU_milandr
static void vial_eeprom_load(void) {
    const vial_storage_data_t *flash_storage = (const vial_storage_data_t *)EEPROM_STORAGE_ADDR;
    if (flash_storage->magic == VIAL_EEPROM_MAGIC) {
        memcpy(&vial_storage.data, flash_storage, sizeof(vial_storage_data_t));
        rgb_set_config(vial_storage.data.rgb_cfg);
    } else {
        vial_storage_init_default();
    }
}

static __RAMFUNC void vial_eeprom_write_flash(void) {
    RST_CLK_PCLKcmd(RST_CLK_PCLK_EEPROM, ENABLE);
    EEPROM_SetLatency(EEPROM_Latency_3);
    __disable_irq();

    EEPROM_ErasePage(EEPROM_STORAGE_ADDR, EEPROM_Main_Bank_Select);
    const uint32_t *src_words = (const uint32_t *)&vial_storage.data;
    for (size_t i = 0; i < sizeof(vial_storage_data_t) / 4; i++) {
        EEPROM_ProgramWord(EEPROM_STORAGE_ADDR + (i * 4), EEPROM_Main_Bank_Select, src_words[i]);
    }
    EEPROM_UpdateDCache();
    __enable_irq();
}

static void vial_eeprom_save_now(void) {
    vial_storage.data.magic = VIAL_EEPROM_MAGIC;
    rgb_get_config(vial_storage.data.rgb_cfg);
    if (memcmp((const void *)EEPROM_STORAGE_ADDR, &vial_storage.data, sizeof(vial_storage_data_t)) == 0) {
        return;
    }
    vial_eeprom_write_flash();
}
#endif

#if defined(MCU_rp2040) || defined(MCU_rp2350)
#include "hardware/flash.h"
#include "hardware/sync.h"

#include "flash_storage.h"
#define FLASH_STORAGE_OFFSET DMK_FLASH_OFFSET(DMK_FLASH_SECTOR_VIAL)
_Static_assert(sizeof(vial_storage_t) <= FLASH_SECTOR_SIZE, "Vial storage exceeds its reserved sector");
_Static_assert(sizeof(vial_storage_t) % FLASH_PAGE_SIZE == 0, "Vial storage must fill whole flash pages");

static void vial_eeprom_load(void) {
    const vial_storage_t *flash_storage = (const vial_storage_t *)(XIP_BASE + FLASH_STORAGE_OFFSET);
    if (flash_storage->data.magic == VIAL_EEPROM_MAGIC) {
        memcpy(&vial_storage.data, &flash_storage->data, sizeof(vial_storage_data_t));
        rgb_set_config(vial_storage.data.rgb_cfg);
    } else {
        vial_storage_init_default();
    }
}

static void vial_eeprom_save_now(void) {
    vial_storage.data.magic = VIAL_EEPROM_MAGIC;
    rgb_get_config(vial_storage.data.rgb_cfg);
    const vial_storage_t *flash_storage = (const vial_storage_t *)(XIP_BASE + FLASH_STORAGE_OFFSET);
    if (memcmp(&flash_storage->data, &vial_storage.data, sizeof(vial_storage_data_t)) == 0) {
        return;
    }
    uint32_t ints = save_and_disable_interrupts();
    flash_range_erase(FLASH_STORAGE_OFFSET, FLASH_SECTOR_SIZE);
    restore_interrupts(ints);
    // Give interrupts an opportunity to run between page programs.
    for (size_t offset = 0; offset < sizeof(vial_storage); offset += FLASH_PAGE_SIZE) {
        ints = save_and_disable_interrupts();
        flash_range_program(FLASH_STORAGE_OFFSET + offset, (const uint8_t *)&vial_storage + offset, FLASH_PAGE_SIZE);
        restore_interrupts(ints);
    }
}
#endif

#if defined(MCU_baikal)
#include "bmcu_cru.h"
#include "bmcu_eflash.h"

// eFlash layout on Baikal BE-U1000:
// 256 KB total (0xA0000000 - 0xA003FFFF).
// TinyUF2 bootloader is at 0xA0000000 - 0xA0003FFF (16 KB).
// Application Flash is at 0xA0004000 - 0xA003FFFF (240 KB).
// We reserve the top 4 KB block (0xA003F000 - 0xA003FFFF) for non-volatile Vial/RGB config.
#define BAIKAL_FLASH_STORAGE_ADDR 0xA003F000UL

static void vial_eeprom_load(void) {
    const vial_storage_data_t *flash_storage = (const vial_storage_data_t *)BAIKAL_FLASH_STORAGE_ADDR;
    if (flash_storage->magic == VIAL_EEPROM_MAGIC) {
        memcpy(&vial_storage.data, flash_storage, sizeof(vial_storage_data_t));
        rgb_set_config(vial_storage.data.rgb_cfg);
    } else {
        vial_storage_init_default();
    }
}

static void vial_eeprom_save_now(void) {
    vial_storage.data.magic = VIAL_EEPROM_MAGIC;
    rgb_get_config(vial_storage.data.rgb_cfg);
    if (memcmp((const void *)BAIKAL_FLASH_STORAGE_ADDR, &vial_storage.data, sizeof(vial_storage_data_t)) == 0) {
        return;
    }

    CRU_Clocks_TypeDef cru_clocks;
    CRU_GetSystemClocksFreq(&cru_clocks);
    uint32_t cpu_freq = cru_clocks.CCLK_Frequency ? cru_clocks.CCLK_Frequency : 200000000;
    EFLASH_Init(cpu_freq);

    taskENTER_CRITICAL();
    // Erase the 4 sectors (1024 bytes each = 4096 bytes block)
    for (uint32_t addr = BAIKAL_FLASH_STORAGE_ADDR; addr < BAIKAL_FLASH_STORAGE_ADDR + 4096; addr += EFLASH_SECTOR_SIZE) {
        EFLASH_EraseSector(addr, EFLASH_MAIN_ARRAY);
    }

    const uint32_t *src_words = (const uint32_t *)&vial_storage.data;
    size_t word_count = sizeof(vial_storage_data_t) / 4;
    for (size_t i = 0; i < word_count; i++) {
        EFLASH_ProgramWord(BAIKAL_FLASH_STORAGE_ADDR + (i * 4), src_words[i], EFLASH_MAIN_ARRAY);
    }
    taskEXIT_CRITICAL();
}
#endif

#if defined(MCU_nrf52840)
#include "nrf_nvmc.h"

// Flash layout on nRF52840 (1 MB total: 0x00000000 - 0x00100000, 4 KB page size):
// SoftDevice / MBR: 0x00000000 - 0x00026000
// Application: 0x00026000 - 0x000E0000
// Bootloader + Bootloader settings: top pages (0x000F4000 - 0x00100000)
// Reserved page for Vial / RGB persistent storage: 0x000EE000 (1 page = 4096 bytes)
#define NRF_FLASH_STORAGE_PAGE_ADDR 0x000EE000UL

static void vial_eeprom_load(void) {
    const vial_storage_data_t *flash_storage = (const vial_storage_data_t *)NRF_FLASH_STORAGE_PAGE_ADDR;
    if (flash_storage->magic == VIAL_EEPROM_MAGIC) {
        memcpy(&vial_storage.data, flash_storage, sizeof(vial_storage_data_t));
        rgb_set_config(vial_storage.data.rgb_cfg);
    } else {
        vial_storage_init_default();
    }
}

static void vial_eeprom_save_now(void) {
    vial_storage.data.magic = VIAL_EEPROM_MAGIC;
    rgb_get_config(vial_storage.data.rgb_cfg);
    if (memcmp((const void *)NRF_FLASH_STORAGE_PAGE_ADDR, &vial_storage.data, sizeof(vial_storage_data_t)) == 0) {
        return;
    }

    taskENTER_CRITICAL();
    nrf_nvmc_page_erase(NRF_FLASH_STORAGE_PAGE_ADDR);
    nrf_nvmc_write_words(NRF_FLASH_STORAGE_PAGE_ADDR, (const uint32_t *)&vial_storage.data, sizeof(vial_storage_data_t) / 4);
    taskEXIT_CRITICAL();
}
#endif

#if !defined(MCU_milandr) && !defined(MCU_rp2040) && !defined(MCU_rp2350) && !defined(MCU_baikal) && !defined(MCU_nrf52840)
static inline void vial_eeprom_load(void) {
    vial_storage_init_default();
}
static inline void vial_eeprom_save_now(void) {}
#endif

void vial_eeprom_save(void) {
    s_eeprom_dirty_time = xTaskGetTickCount();
    s_eeprom_dirty = true;
}

void vial_eeprom_task(void) {
    if (s_eeprom_dirty) {
        TickType_t now = xTaskGetTickCount();
        if ((now - s_eeprom_dirty_time) >= pdMS_TO_TICKS(VIAL_EEPROM_SAVE_DELAY_MS)) {
            s_eeprom_dirty = false;
            vial_eeprom_save_now();
        }
    }
}

void vial_init(void) {
    s_eeprom_dirty = false;
    vial_eeprom_load();
}

__attribute__((weak)) bool via_custom_value_command_kb(uint8_t const *request, uint8_t *response) {
    (void)request;
    (void)response;
    return false;
}

void vial_process_packet(uint8_t const *request, uint8_t *response) {
    // Fill response with request values as a default/fallback
    memcpy(response, request, 32);

    uint8_t command_id = request[0];
    switch (command_id) {
    case VIA_GET_PROTOCOL_VERSION: {
#if defined(VIA_PROTOCOL_VERSION)
        // Explicit protocol version override
        response[1] = (uint8_t)(((uint16_t)(VIA_PROTOCOL_VERSION) >> 8) & 0xFF);
        response[2] = (uint8_t)((uint16_t)(VIA_PROTOCOL_VERSION) & 0xFF);
#elif defined(VIA_V3) || defined(USE_VIA_V3) || (defined(VIA) && !defined(VIAL))
        // VIA v3 (Protocol 12) for usevia.app Custom UI menus
        response[1] = 0x00;
        response[2] = 0x0C;
#else
        // Default: Vial GUI compatibility (VIA Protocol 9)
        response[1] = 0x00;
        response[2] = 0x09;
#endif
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
                        uint8_t offset = 2 + (r * row_size) + byte_idx;
                        if (offset < 32) {
                            response[offset] |= (uint8_t)(1U << (c % 8));
                        }
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
            vial_eeprom_save();
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_GET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        if (size > 28) {
            size = 28;
        }
        uint16_t start = offset / 2;
        uint8_t count = size / 2;
        uint16_t matrix_size = (uint16_t)NUM_ROWS * (uint16_t)NUM_COLS;
        for (uint8_t i = 0; i < count; i++) {
            uint16_t idx = start + i;
            uint8_t layer = matrix_size ? (idx / matrix_size) : 0;
            uint16_t m_pos = matrix_size ? (idx % matrix_size) : 0;
            uint8_t row = m_pos / NUM_COLS;
            uint8_t col = m_pos % NUM_COLS;
            int16_t ki = keyboard_get_flat_key_index(row, col);
            uint16_t via_key = 0x0001; // transparent default
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && ki >= 0 && ki < (int16_t)NUM_KEYS) {
                via_key = to_via_keycode(dynamic_keymap[layer][ki]);
            }
            response[4 + i * 2] = (via_key >> 8) & 0xFF;
            response[5 + i * 2] = via_key & 0xFF;
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_SET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        if (size > 28) {
            size = 28;
        }
        uint16_t start = offset / 2;
        uint8_t count = size / 2;
        uint16_t matrix_size = (uint16_t)NUM_ROWS * (uint16_t)NUM_COLS;
        for (uint8_t i = 0; i < count; i++) {
            uint16_t idx = start + i;
            uint8_t layer = matrix_size ? (idx / matrix_size) : 0;
            uint16_t m_pos = matrix_size ? (idx % matrix_size) : 0;
            uint8_t row = m_pos / NUM_COLS;
            uint8_t col = m_pos % NUM_COLS;
            int16_t ki = keyboard_get_flat_key_index(row, col);
            uint16_t via_key = ((uint16_t)request[4 + i * 2] << 8) | request[5 + i * 2];
            if (layer < DYNAMIC_KEYMAP_MAX_LAYERS && ki >= 0 && ki < (int16_t)NUM_KEYS) {
                dynamic_keymap[layer][ki] = from_via_keycode(via_key);
            }
        }
        vial_eeprom_save();
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
            vial_eeprom_save();
        }
#endif
        break;
    }

    case VIA_EEPROM_RESET: {
        vial_storage_init_default();
        vial_eeprom_save();
        break;
    }

    case VIA_BOOTLOADER_JUMP: {
        bootloader_jump();
        break;
    }

    case VIA_CUSTOM_GET_VALUE: { // 0x08
        if (via_custom_value_command_kb(request, response)) {
            break;
        }
        uint8_t channel_or_id = request[1];
        if (channel_or_id == 2) { // VIA v3 qmk_rgblight channel
            uint8_t qmk_val = request[2];
            switch (qmk_val) {
            case 1: // brightness
                response[3] = rgb_get_brightness();
                break;
            case 2: // effect
                response[3] = rgb_get_mode();
                break;
            case 3: // speed
                response[3] = rgb_get_speed();
                break;
            case 4: // color
                response[3] = rgb_get_hue();
                response[4] = rgb_get_sat();
                break;
            }
            break;
        }
        // Legacy VIA v2 fallback
        switch (channel_or_id) {
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
        if (via_custom_value_command_kb(request, response)) {
            break;
        }
        uint8_t channel_or_id = request[1];
        if (channel_or_id == 2) { // VIA v3 qmk_rgblight channel
            uint8_t qmk_val = request[2];
            switch (qmk_val) {
            case 1: // brightness
                rgb_set_brightness(request[3]);
                break;
            case 2: // effect
                rgb_set_mode(request[3]);
                break;
            case 3: // speed
                rgb_set_speed(request[3]);
                break;
            case 4: // color
                rgb_set_color(request[3], request[4]);
                break;
            }
            break;
        }
        // Legacy VIA v2 fallback
        switch (channel_or_id) {
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
        if (via_custom_value_command_kb(request, response)) {
            break;
        }
        vial_eeprom_save();
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
        if (size > 28) {
            size = 28;
        }
        if ((uint32_t)offset + size <= DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE) {
            memcpy(&response[4], &dynamic_macro_buffer[offset], size);
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_SET_BUFFER: {
        uint16_t offset = ((uint16_t)request[1] << 8) | request[2];
        uint8_t size = request[3];
        if (size > 28) {
            size = 28;
        }
        if ((uint32_t)offset + size <= DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE) {
            memcpy(&dynamic_macro_buffer[offset], &request[4], size);
            vial_eeprom_save();
        }
        break;
    }

    case VIA_DYNAMIC_KEYMAP_MACRO_RESET: {
        memset(dynamic_macro_buffer, 0, DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE);
        vial_eeprom_save();
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

        case VIAL_LOCK:
        case VIAL_QMK_SETTINGS_QUERY:
        case VIAL_QMK_SETTINGS_GET:
        case VIAL_QMK_SETTINGS_SET:
        case VIAL_QMK_SETTINGS_RESET:
            memset(response, 0xFF, 32);
            break;

        case VIAL_GET_UNLOCK_STATUS:
        case VIAL_UNLOCK_START:
        case VIAL_UNLOCK_POLL:
            memset(response, 0xFF, 32);
            response[0] = 1;
            response[1] = 0;
            if (vial_cmd == VIAL_UNLOCK_POLL)
                response[2] = 0;
            break;

        case VIAL_DYNAMIC_ENTRY_OP: {
            uint8_t op = request[2];
            memset(response, 0x00, 32);
            if (op == 0) {                        // DynamicVialGetNumberOfEntries
                response[0] = 0;                  // Tap dance entries
                response[1] = VIAL_COMBO_ENTRIES; // Combo entries
                response[2] = 0;                  // Key override entries
                response[3] = 0;                  // Alt repeat key entries
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
                    vial_eeprom_save();
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
                vial_eeprom_save();
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

    static const struct {
        char c;
        uint8_t kc;
        bool s;
    } tbl[] = {{' ', 0x2C, 0},  {'\n', 0x28, 0}, {'\t', 0x2B, 0}, {'-', 0x2D, 0}, {'_', 0x2D, 1},
               {'=', 0x2E, 0},  {'+', 0x2E, 1},  {'[', 0x2F, 0},  {'{', 0x2F, 1}, {']', 0x30, 0},
               {'}', 0x30, 1},  {'\\', 0x31, 0}, {'|', 0x31, 1},  {';', 0x33, 0}, {':', 0x33, 1},
               {'\'', 0x34, 0}, {'"', 0x34, 1},  {'`', 0x35, 0},  {'~', 0x35, 1}, {',', 0x36, 0},
               {'<', 0x36, 1},  {'.', 0x37, 0},  {'>', 0x37, 1},  {'/', 0x38, 0}, {'?', 0x38, 1}};
    for (size_t i = 0; i < sizeof(tbl) / sizeof(tbl[0]); i++) {
        if (tbl[i].c == c) {
            *shift = tbl[i].s;
            return tbl[i].kc;
        }
    }
    return 0;
}

static void send_event(uint16_t kc, bool pressed) {
    keyboard_send_key(kc, pressed);
}

typedef struct {
    uint16_t kc;
    uint8_t pressed;
    uint8_t delay_ms;
} vial_macro_uop_t;

static uint16_t s_macro_offset = DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE;
static TickType_t s_macro_deadline = 0;
static vial_macro_uop_t s_uops[4];
static uint8_t s_uop_head = 0;
static uint8_t s_uop_count = 0;

static void push_uop(uint16_t kc, uint8_t pressed, uint8_t delay_ms) {
    if (s_uop_count < 4) {
        s_uops[s_uop_count++] = (vial_macro_uop_t){.kc = kc, .pressed = pressed, .delay_ms = delay_ms};
    }
}

static void queue_tap_key(uint16_t kc) {
    push_uop(kc, 1, 10);
    push_uop(kc, 0, 10);
}

static void queue_ascii(char c) {
    bool shift = false;
    uint16_t kc = ascii_to_keycode(c, &shift);
    if (kc) {
        if (shift) {
            push_uop(0xE1, 1, 5);
        }
        queue_tap_key(kc);
        if (shift) {
            push_uop(0xE1, 0, 5);
        }
    }
}

bool vial_macro_has_active(void) {
    return (s_uop_head < s_uop_count) ||
           (s_macro_offset < DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE && dynamic_macro_buffer[s_macro_offset] != 0) ||
           (s_macro_deadline != 0);
}

TickType_t vial_macro_check_timeouts(TickType_t now) {
    if (s_macro_deadline != 0) {
        if ((int32_t)(now - s_macro_deadline) < 0) {
            return (s_macro_deadline - now);
        }
        s_macro_deadline = 0;
    }

    const uint16_t end = DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE;
    while (s_uop_head < s_uop_count || (s_macro_offset < end && dynamic_macro_buffer[s_macro_offset] != 0)) {
        if (s_uop_head < s_uop_count) {
            vial_macro_uop_t uop = s_uops[s_uop_head++];
            if (s_uop_head >= s_uop_count) {
                s_uop_head = 0;
                s_uop_count = 0;
            }
            send_event(uop.kc, uop.pressed != 0);
            if (uop.delay_ms > 0) {
                TickType_t ticks = pdMS_TO_TICKS(uop.delay_ms);
                if (ticks == 0) {
                    ticks = 1;
                }
                s_macro_deadline = now + ticks;
                return ticks;
            }
            continue;
        }

        uint8_t c = dynamic_macro_buffer[s_macro_offset++];
        if (c == 0) {
            s_macro_offset = end;
            break;
        }

        if (c == 1) { // SS_QMK_PREFIX
            if (s_macro_offset >= end) {
                s_macro_offset = end;
                break;
            }
            uint8_t action = dynamic_macro_buffer[s_macro_offset++];
            if (action == 0) {
                s_macro_offset = end;
                break;
            }

            if (action == 1 || action == 2 || action == 3) { // SS_TAP_CODE, SS_DOWN_CODE, SS_UP_CODE
                if (s_macro_offset >= end) {
                    s_macro_offset = end;
                    break;
                }
                uint8_t kc = dynamic_macro_buffer[s_macro_offset++];
                if (kc == 0) {
                    s_macro_offset = end;
                    break;
                }
                if (action == 1) {
                    queue_tap_key(kc);
                } else if (action == 2) {
                    send_event(kc, true);
                } else {
                    send_event(kc, false);
                }
            } else if (action == 5 || action == 6 || action == 7) { // VIAL_MACRO_EXT_TAP, DOWN, UP
                if (s_macro_offset + 1 >= end) {
                    s_macro_offset = end;
                    break;
                }
                uint16_t kc = dynamic_macro_buffer[s_macro_offset] |
                              ((uint16_t)dynamic_macro_buffer[s_macro_offset + 1] << 8);
                s_macro_offset += 2;
                if (kc > 0xFF00) {
                    kc = (kc & 0xFF) << 8;
                }
                if (action == 5) {
                    queue_tap_key(kc);
                } else if (action == 6) {
                    send_event(kc, true);
                } else {
                    send_event(kc, false);
                }
            } else if (action == 4) { // SS_DELAY_CODE
                if (s_macro_offset + 1 >= end) {
                    s_macro_offset = end;
                    break;
                }
                uint8_t d0 = dynamic_macro_buffer[s_macro_offset++];
                uint8_t d1 = dynamic_macro_buffer[s_macro_offset++];
                if (d0 == 0 || d1 == 0) {
                    s_macro_offset = end;
                    break;
                }
                uint32_t ms = (uint32_t)(d0 - 1) + (uint32_t)(d1 - 1) * 255u;
                if (ms > 0) {
                    TickType_t ticks = pdMS_TO_TICKS(ms);
                    if (ticks == 0) {
                        ticks = 1;
                    }
                    s_macro_deadline = now + ticks;
                    return ticks;
                }
            }
        } else {
            queue_ascii((char)c);
        }
    }

    s_macro_offset = end;
    return portMAX_DELAY;
}

void vial_macro_send(uint8_t id) {
    if (id >= DYNAMIC_KEYMAP_MACRO_COUNT) {
        return;
    }

    while (s_uop_head < s_uop_count) {
        vial_macro_uop_t uop = s_uops[s_uop_head++];
        if (!uop.pressed) {
            send_event(uop.kc, false);
        }
    }
    s_uop_head = 0;
    s_uop_count = 0;
    s_macro_deadline = 0;

    uint16_t offset = 0;
    const uint16_t end = DYNAMIC_KEYMAP_MACRO_BUFFER_SIZE;
    while (id > 0) {
        if (offset >= end) {
            s_macro_offset = end;
            return;
        }
        if (dynamic_macro_buffer[offset] == 0) {
            --id;
        }
        ++offset;
    }

    if (offset >= end || dynamic_macro_buffer[offset] == 0) {
        s_macro_offset = end;
        return;
    }

    s_macro_offset = offset;
    vial_macro_check_timeouts(xTaskGetTickCount());
}

#endif // VIAL
