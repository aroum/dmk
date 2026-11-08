#include "magneteno_display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"

#include "../hall_calibration/include/hall_calibration.h"
#include "../imu/magneteno_imu.h"
#include "../sequencer/sequencer.h"
#include "../synth/synth.h"
#include "../ui/magneteno_ui.h"

#include "hardware/gpio.h"
#include "hardware/spi.h"
#include "pico/stdlib.h"
#include "u8g2.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

#define PIN_LCD_MOSI 11 // SPI1 TX
#define PIN_LCD_SCK  10 // SPI1 SCK
#define PIN_LCD_CS   9  // SPI1 CS (Active High for Sharp Memory LCD)
#define LCD_SPI_INST spi1

static u8g2_t g_u8g2;
static bool g_display_ready = false;
static float g_wave_phase = 0.0f;

// Note names lookup table
static const char *s_note_names[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// u8g2 SPI byte callback for RP2350 hardware SPI1
static uint8_t u8x8_byte_pico_hw_spi(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    (void)u8x8;
    switch (msg) {
        case U8X8_MSG_BYTE_SEND:
            spi_write_blocking(LCD_SPI_INST, (const uint8_t *)arg_ptr, arg_int);
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            gpio_put(PIN_LCD_CS, 1); // Active HIGH for Sharp Memory LCD
            sleep_us(2);
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            sleep_us(2);
            gpio_put(PIN_LCD_CS, 0);
            break;
        default:
            return 0;
    }
    return 1;
}

// u8g2 delay and GPIO callback
static uint8_t u8x8_gpio_and_delay_pico(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    (void)u8x8;
    (void)arg_ptr;
    switch (msg) {
        case U8X8_MSG_DELAY_MILLI:
            sleep_ms(arg_int);
            break;
        case U8X8_MSG_DELAY_10MICRO:
            sleep_us(arg_int * 10);
            break;
        case U8X8_MSG_DELAY_NANO:
            sleep_us(1);
            break;
        default:
            return 0;
    }
    return 1;
}

void magneteno_display_init(void) {
    // 1. Initialize hardware SPI1 at 2MHz
    spi_init(LCD_SPI_INST, 2000000);
    gpio_set_function(PIN_LCD_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI, GPIO_FUNC_SPI);

    // 2. Configure CS pin as GPIO output (default LOW)
    gpio_init(PIN_LCD_CS);
    gpio_set_dir(PIN_LCD_CS, GPIO_OUT);
    gpio_put(PIN_LCD_CS, 0);

    // 3. Initialize u8g2 for Sharp Memory LCD LS011B7DH03 (160x68)
    u8g2_Setup_ls011b7dh03_160x68_f(
        &g_u8g2,
        U8G2_R0,
        u8x8_byte_pico_hw_spi,
        u8x8_gpio_and_delay_pico
    );

    u8g2_InitDisplay(&g_u8g2);
    u8g2_SetPowerSave(&g_u8g2, 0);
    u8g2_ClearBuffer(&g_u8g2);
    u8g2_SendBuffer(&g_u8g2);

    g_display_ready = true;
}

static void draw_status_bar(MagnetenoUIMode mode) {
    u8g2_SetFont(&g_u8g2, u8g2_font_5x7_tr);
    u8g2_SetDrawColor(&g_u8g2, 1);

    const char *mode_str = "PIANO";
    if (!magneteno_ui_is_synth_enabled()) {
        mode_str = "KBD (SYNTH OFF)";
    } else {
        switch (mode) {
            case UI_MODE_PIANO: mode_str = "PIANO"; break;
            case UI_MODE_SEQ: mode_str = "SEQ"; break;
            case UI_MODE_MIXER: mode_str = "MIXER"; break;
            case UI_MODE_LOOPER: mode_str = "LOOPER"; break;
            case UI_MODE_MENU_MAIN: mode_str = "MENU"; break;
            case UI_MODE_MENU_OSC: mode_str = "OSC"; break;
            case UI_MODE_MENU_VCF: mode_str = "VCF"; break;
            case UI_MODE_MENU_EG: mode_str = "EG"; break;
            case UI_MODE_MENU_LFO: mode_str = "LFO"; break;
            case UI_MODE_MENU_ADV: mode_str = "ADV"; break;
            case UI_MODE_MENU_PRESETS: mode_str = "PRESETS"; break;
            case UI_MODE_MENU_FX: mode_str = "FX"; break;
            default: break;
        }
    }

    char buf[48];
    // Mode and Octave
    snprintf(buf, sizeof(buf), "%s | OCT %+d", mode_str, g_piano_octave);
    u8g2_DrawStr(&g_u8g2, 2, 9, buf);

    // BPM and IMU tilt
    int roll = (int)g_imu_state.roll_deg;
    int pitch = (int)g_imu_state.pitch_deg;
    snprintf(buf, sizeof(buf), "%dBPM | R:%d P:%d", g_seq_state.bpm, roll, pitch);
    int w = u8g2_GetStrWidth(&g_u8g2, buf);
    u8g2_DrawStr(&g_u8g2, 158 - w, 9, buf);

    // Horizontal separator
    u8g2_DrawHLine(&g_u8g2, 0, 11, 160);
}

static void draw_10key_travel_bars(void) {
    // Draw 10 vertical travel bars (width 11, spacing 4, total 146px centered in 160px)
    int start_x = 7;
    int y = 13;
    int h = 18;

    for (int i = 0; i < 10; i++) {
        int x = start_x + i * 15;
        uint16_t raw = hall_get_raw_adc(i);
        const hall_key_calib_t *cal = hall_calibration_get_key(i);

        uint32_t range = (cal->bottom_adc > cal->rest_adc) ? (cal->bottom_adc - cal->rest_adc) : 1000;
        int32_t offset = (int32_t)raw - (int32_t)cal->rest_adc;
        if (offset < 0) offset = 0;

        int fill_h = (int)((offset * (h - 2)) / range);
        if (fill_h > (h - 2)) fill_h = h - 2;

        // Outer frame
        u8g2_DrawFrame(&g_u8g2, x, y, 11, h);

        // Filled travel box
        if (fill_h > 0) {
            u8g2_DrawBox(&g_u8g2, x + 1, y + h - 1 - fill_h, 9, fill_h);
        }

        // Dotted actuation line
        int act_y = y + h - 1 - (int)(((h - 2) * cal->actuation_percent) / 100);
        u8g2_DrawHLine(&g_u8g2, x + 2, act_y, 7);
    }

    // Horizontal separator
    u8g2_DrawHLine(&g_u8g2, 0, 33, 160);
}

static void draw_bottom_section(MagnetenoUIMode mode) {
    int y_start = 35;

    if (mode == UI_MODE_PIANO) {
        // Piano Mode: show active note name, velocity, aftertouch, and live waveform
        u8g2_SetFont(&g_u8g2, u8g2_font_6x10_tr);
        char note_buf[32] = "No Note";
        int active_note = -1;
        uint8_t at_press = 0;

        for (int i = 0; i < 8; i++) {
            uint8_t p = hall_get_pressure(i);
            if (p > at_press) {
                at_press = p;
                active_note = 60 + (g_piano_octave * 12) + i; // Approximate note
            }
        }

        if (active_note >= 0) {
            const char *name = s_note_names[active_note % 12];
            int oct = (active_note / 12) - 1;
            snprintf(note_buf, sizeof(note_buf), "%s%d AT:%d", name, oct, at_press);
        }

        u8g2_DrawStr(&g_u8g2, 4, y_start + 12, note_buf);

        // Live oscilloscope waveform on right side
        g_wave_phase += 0.25f;
        if (g_wave_phase > 6.28f) g_wave_phase -= 6.28f;

        for (int x = 0; x < 70; x++) {
            float amp = (active_note >= 0) ? 10.0f : 2.0f;
            int wy = y_start + 16 + (int)(amp * sinf(g_wave_phase + x * 0.15f));
            if (wy >= 35 && wy < 68) {
                u8g2_DrawPixel(&g_u8g2, 85 + x, wy);
            }
        }
    } else if (mode == UI_MODE_SEQ) {
        // Sequencer Mode: 8 step boxes for current page
        uint8_t page = g_seq_state.current_page;
        TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];

        for (int i = 0; i < 8; i++) {
            int x = 6 + i * 19;
            int box_y = y_start + 5;
            int w = 15;
            int h = 18;
            uint8_t step_idx = page * 8 + i;
            SeqStep *step = &track->steps[step_idx];

            // Outer frame
            u8g2_DrawFrame(&g_u8g2, x, box_y, w, h);

            if (step->note.enabled) {
                // Active note: filled box
                u8g2_DrawBox(&g_u8g2, x + 2, box_y + 2, w - 4, h - 4);
            } else if (step->stop_flag) {
                // Stop step: diagonal cross
                u8g2_DrawLine(&g_u8g2, x, box_y, x + w - 1, box_y + h - 1);
                u8g2_DrawLine(&g_u8g2, x + w - 1, box_y, x, box_y + h - 1);
            }

            // Playhead indicator
            if (g_seq_state.is_playing && g_seq_state.current_step == step_idx) {
                u8g2_DrawTriangle(&g_u8g2, x + 4, box_y - 3, x + 10, box_y - 3, x + 7, box_y);
            }
        }

        // Status text at bottom
        u8g2_SetFont(&g_u8g2, u8g2_font_5x7_tr);
        char seq_info[48];
        snprintf(seq_info, sizeof(seq_info), "Trk %d | P%d/4 | Step %d/32 | %s",
                 g_seq_state.active_track + 1, page + 1, g_seq_state.current_step + 1,
                 g_seq_state.is_playing ? "PLAY" : "STOP");
        u8g2_DrawStr(&g_u8g2, 4, 66, seq_info);
    } else {
        // Menu Mode: show parameter and value slider
        u8g2_SetFont(&g_u8g2, u8g2_font_6x10_tr);
        char param_buf[48];
        snprintf(param_buf, sizeof(param_buf), "PARAM EDIT");
        u8g2_DrawStr(&g_u8g2, 4, y_start + 12, param_buf);

        // Parameter slider bar
        u8g2_DrawFrame(&g_u8g2, 10, y_start + 18, 140, 8);
        u8g2_DrawBox(&g_u8g2, 12, y_start + 20, 70, 4);
    }
}

void magneteno_display_update(MagnetenoUIMode mode) {
    if (!g_display_ready) return;

    u8g2_ClearBuffer(&g_u8g2);

    // 1. Top status bar (0..11 px)
    draw_status_bar(mode);

    // 2. 10-Key Analog Travel Bar Chart (13..33 px)
    draw_10key_travel_bars();

    // 3. Bottom Context View (35..67 px)
    draw_bottom_section(mode);

    u8g2_SendBuffer(&g_u8g2);
}
