#include "magneteno_leds.h"
#include "rgb.h"
#include "config.h"
#include "hall_calibration.h"
#include "../sequencer/sequencer.h"

#define COLOR_BLACK                 0x000000
#define COLOR_WHITE                 0xFFFFFF
#define COLOR_RED                   0xFF0000
#define COLOR_GREEN                 0x00FF00
#define COLOR_BLUE                  0x0064FF
#define COLOR_CYAN                  0x00FFFF
#define COLOR_MAGENTA               0xFF00FF
#define COLOR_YELLOW                0xFFFF00
#define COLOR_AMBER                 0xFF8C00
#define COLOR_LIME                  0x64FF00
#define COLOR_VIOLET                0xB432FF

// Sequencer specific colors (matching omsk_wave)
#define COLOR_SEQ_PLAYHEAD          0xFFFFFF
#define COLOR_SEQ_STOP_STEP         0xFF0000
#define COLOR_SEQ_MUTE_STEP         0x78143C
#define COLOR_SEQ_TRANSPORT_PLAY    0x00FF00
#define COLOR_SEQ_TRANSPORT_STOP    0x500000

// Page indicator colors for Key 10
static const uint32_t PAGE_COLORS[4] = {
    0x00FFFF, // Page 0 (Steps 1-8):  Cyan
    0xFF00FF, // Page 1 (Steps 9-16): Magenta
    0xFFFF00, // Page 2 (Steps 17-24): Yellow
    0xFFFFFF  // Page 3 (Steps 25-32): White
};

static inline uint32_t rgb_to_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | (uint32_t)b;
}

static inline uint32_t scale_color(uint32_t color, float brightness) {
    if (brightness > 1.0f) brightness = 1.0f;
    if (brightness < 0.0f) brightness = 0.0f;
    uint8_t r = (uint8_t)(((color >> 16) & 0xFF) * brightness);
    uint8_t g = (uint8_t)(((color >> 8) & 0xFF) * brightness);
    uint8_t b = (uint8_t)((color & 0xFF) * brightness);
    return rgb_to_u32(r, g, b);
}

void magneteno_leds_init(void) {
    // DMK core rgb_init() takes care of hardware initialization
    magneteno_leds_set_all(0, 0, 0);
    magneteno_leds_show();
}

void magneteno_leds_set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < MAGNETENO_NUM_LEDS) {
        rgb_set_pixel_raw(index, rgb_to_u32(r, g, b));
    }
}

void magneteno_leds_set_all(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t c = rgb_to_u32(r, g, b);
    for (int i = 0; i < MAGNETENO_NUM_LEDS; i++) {
        rgb_set_pixel_raw(i, c);
    }
}

void magneteno_leds_show(void) {
    rgb_show();
}

void magneteno_leds_update(MagnetenoUIMode mode) {
    switch (mode) {
        case UI_MODE_SEQ: {
            uint8_t page = g_seq_state.current_page;
            TrackSequence *track = &g_seq_state.tracks[g_seq_state.active_track];

            // Steps 1..8 (Keys 0..7)
            for (int i = 0; i < SEQ_STEPS_PER_PAGE; i++) {
                uint8_t step_idx = (page * SEQ_STEPS_PER_PAGE) + i;
                SeqStep *step = &track->steps[step_idx];
                uint32_t c = COLOR_BLACK;

                if (step->stop_flag) {
                    c = COLOR_SEQ_STOP_STEP; // Red for stop step
                } else if (!step->note.enabled) {
                    if (step->note.note > 0) {
                        c = COLOR_SEQ_MUTE_STEP; // Muted note
                    } else {
                        c = COLOR_BLACK; // Empty
                    }
                } else {
                    // Active note: Green scaled by velocity/probability
                    float br = (float)step->note.velocity / 127.0f;
                    c = scale_color(COLOR_GREEN, br);
                }

                // Playhead takes highest priority (bright white)
                if (g_seq_state.is_playing && g_seq_state.current_step == step_idx) {
                    c = COLOR_SEQ_PLAYHEAD;
                }

                // Step edit mode: pulse/blink the edited step
                if (g_seq_state.step_edit_mode && g_seq_state.edit_step == step_idx) {
                    c = COLOR_YELLOW;
                }

                rgb_set_pixel_raw(i, c);
            }

            // Key 9 (Index 8): Transport Play / Stop
            if (g_seq_state.is_playing) {
                rgb_set_pixel_raw(8, COLOR_SEQ_TRANSPORT_PLAY);
            } else {
                rgb_set_pixel_raw(8, COLOR_SEQ_TRANSPORT_STOP);
            }

            // Key 10 (Index 9): Page Indicator (Page 0..3)
            rgb_set_pixel_raw(9, PAGE_COLORS[page % 4]);
            break;
        }

        case UI_MODE_MENU_MAIN: {
            // Main Menu: Display module colors for 10 choices
            rgb_set_pixel_raw(0, COLOR_WHITE);   // [1] OSC (Warm White)
            rgb_set_pixel_raw(1, COLOR_CYAN);    // [2] VCF (Cyan / Deep Pink)
            rgb_set_pixel_raw(2, COLOR_AMBER);   // [3] EG (Amber)
            rgb_set_pixel_raw(3, COLOR_LIME);    // [4] LFO (Lime)
            rgb_set_pixel_raw(4, COLOR_BLUE);    // [5] Advanced (Blue)
            rgb_set_pixel_raw(5, COLOR_MAGENTA); // [6] Presets (Magenta)
            rgb_set_pixel_raw(6, COLOR_VIOLET);  // [7] FX (Violet)
            rgb_set_pixel_raw(7, COLOR_AMBER);   // [8] Mixer (Amber)
            rgb_set_pixel_raw(8, COLOR_WHITE);   // [9] Piano (White)
            rgb_set_pixel_raw(9, COLOR_GREEN);   // [10] Sequencer (Green)
            break;
        }

        case UI_MODE_PIANO: {
            // Notes 1..8: Root note (Blue), Scale notes (Green)
            // Brightness dynamically scales with physical Hall aftertouch depth!
            for (int i = 0; i < 8; i++) {
                uint8_t at = hall_get_pressure(i);
                float br = 0.3f + 0.7f * ((float)at / 127.0f);
                if (i == 0 || i == 7) {
                    rgb_set_pixel_raw(i, scale_color(COLOR_BLUE, br)); // Tonic / Octave
                } else {
                    rgb_set_pixel_raw(i, scale_color(COLOR_GREEN, br)); // In-scale degree
                }
            }
            // Key 9 & 10: Octave Shift Down / Up (White at center, Blue when shifted)
            rgb_set_pixel_raw(8, scale_color(COLOR_WHITE, 0.4f));
            rgb_set_pixel_raw(9, scale_color(COLOR_WHITE, 0.4f));
            break;
        }

        case UI_MODE_MIXER: {
            // Faders: Level indicator
            for (int i = 0; i < 8; i++) {
                rgb_set_pixel_raw(i, scale_color(COLOR_AMBER, 0.5f));
            }
            // Key 9: Faders mode indicator (Amber)
            rgb_set_pixel_raw(8, COLOR_AMBER);
            // Key 10: Bank A/B (Cyan / Magenta)
            rgb_set_pixel_raw(9, COLOR_CYAN);
            break;
        }

        case UI_MODE_LOOPER: {
            // Looper: Tracks 1..4 (or 5..8)
            for (int i = 0; i < 4; i++) {
                TrackSequence *t = &g_seq_state.tracks[i];
                if (t->is_audio_loop) {
                    rgb_set_pixel_raw(i, COLOR_GREEN); // Playing loop
                } else {
                    rgb_set_pixel_raw(i, scale_color(COLOR_WHITE, 0.2f)); // Empty
                }
            }
            // Mute / Clear keys 5..8
            for (int i = 4; i < 8; i++) {
                rgb_set_pixel_raw(i, COLOR_SEQ_MUTE_STEP);
            }
            // Key 9: Looper mode indicator (Blue)
            rgb_set_pixel_raw(8, COLOR_BLUE);
            // Key 10: Bank A/B
            rgb_set_pixel_raw(9, COLOR_CYAN);
            break;
        }

        case UI_MODE_MENU_OSC:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_WHITE, 0.3f));
            rgb_set_pixel_raw(8, COLOR_WHITE);
            rgb_set_pixel_raw(9, COLOR_WHITE);
            break;

        case UI_MODE_MENU_VCF:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_CYAN, 0.3f));
            rgb_set_pixel_raw(8, COLOR_CYAN);
            rgb_set_pixel_raw(9, COLOR_CYAN);
            break;

        case UI_MODE_MENU_EG:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_AMBER, 0.3f));
            rgb_set_pixel_raw(8, COLOR_AMBER);
            rgb_set_pixel_raw(9, COLOR_AMBER);
            break;

        case UI_MODE_MENU_LFO:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_LIME, 0.3f));
            rgb_set_pixel_raw(8, COLOR_LIME);
            rgb_set_pixel_raw(9, COLOR_LIME);
            break;

        case UI_MODE_MENU_ADV:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_BLUE, 0.3f));
            rgb_set_pixel_raw(8, COLOR_BLUE);
            rgb_set_pixel_raw(9, COLOR_BLUE);
            break;

        case UI_MODE_MENU_PRESETS:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_MAGENTA, 0.3f));
            rgb_set_pixel_raw(8, COLOR_MAGENTA);
            rgb_set_pixel_raw(9, COLOR_MAGENTA);
            break;

        case UI_MODE_MENU_FX:
            for (int i = 0; i < 8; i++) rgb_set_pixel_raw(i, scale_color(COLOR_VIOLET, 0.3f));
            rgb_set_pixel_raw(8, COLOR_VIOLET);
            rgb_set_pixel_raw(9, COLOR_VIOLET);
            break;

        default:
            break;
    }

    magneteno_leds_show();
}
