# External Modules & Extensions System in DMK

DMK features a flexible external module system that allows adding custom hardware drivers, indicators, displays, and algorithms without modifying the core firmware codebase.

---

## 1. Module Structure

An external module is a directory containing a `module.cmake` file along with its source and header files.

```
my_custom_module/
├── module.cmake           # CMake build instructions for the module
├── include/               # Public headers
│   └── my_module.h
├── my_module.c            # Module implementation
└── custom_driver.pio      # (Optional) Raspberry Pi Pico / RP2350 PIO program
```

### `module.cmake` Example
```cmake
target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/my_module.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
```

---

## 2. Enabling Modules during Build

Pass a semicolon-separated list of module directories to CMake using `-DDMK_MODULES`:

```bash
# Single module
cmake -B build -DKEYBOARD=corne -DDMK_MODULES=/path/to/my_custom_module -DMCU=rp2040

# Multiple modules
cmake -B build -DKEYBOARD=magneteno -DDMK_MODULES="tests/modules/hall_calibration;tests/modules/sharp_memory_lcd" -DMCU=rp2350
```

Both relative (to repository root) and absolute paths are supported.

---

## 3. Core Lifecycle & Event Hooks

Modules communicate with DMK via weak hooks declared in `dmk_core/include/hooks.h`. Modules can override any of these hooks without boilerplate:

| Hook Function | When It Is Called | Typical Use Cases |
| :--- | :--- | :--- |
| `void hook_early_init(void)` | In `main.c` before `vTaskStartScheduler()` | Starting custom FreeRTOS background tasks (display tasks, sensor polling, etc.) |
| `void hook_layer_change(uint8_t active_layer)` | On every active layer change (`layers.c`) | Layer LED / RGB indicators, OLED/LCD layer display |
| `void hook_matrix_change(uint8_t row, uint8_t col, bool pressed)` | On every physical key matrix state change | Matrix debug LEDs, custom audio feedback |
| `void hook_key_sent(uint16_t keycode, bool pressed)` | When USB HID keycode is queued to host | Real-time WPM calculation, keypress counters |
| `void hook_hid_led_change(uint8_t led_mask)` | When host updates keyboard LEDs (`led.c`) | CapsLock (`0x02`), NumLock (`0x01`), ScrollLock (`0x04`) indicators |

### Hook Implementation Example:
```c
#include "hooks.h"
#include "rgb.h"
#include <stdint.h>
#include <stdbool.h>

#define LAYER_LED_INDEX 0
#define CAPS_LED_INDEX  1

void hook_layer_change(uint8_t active_layer) {
    uint32_t colors[] = { 0x00FF00, 0x0000FF, 0xFF00FF, 0xFFFF00 };
    uint32_t color = (active_layer < 4) ? colors[active_layer] : 0xFFFFFF;
    rgb_set_pixel_raw(LAYER_LED_INDEX, color);
    rgb_show();
}

void hook_hid_led_change(uint8_t led_mask) {
    bool caps_on = (led_mask & 0x02) != 0;
    rgb_set_pixel_raw(CAPS_LED_INDEX, caps_on ? 0xFF0000 : 0x000000);
    rgb_show();
}
```

---

## 4. Direct RGB Control API

DMK provides low-level functions in `dmk_core/drivers/rgb.h` for direct module control over addressable RGB LEDs:

* `void rgb_set_pixel_raw(uint32_t index, uint32_t color_hex)` — Sets the color of a specific LED index in `0xRRGGBB` format.
* `void rgb_show(void)` — Instantly transmits the RGB frame buffer to hardware.

---

## 5. Custom Matrix Drivers (Hall Effect, MUX, Trackballs)

To replace the standard DMK matrix scanner with a custom implementation:
1. Define `#define CUSTOM_MATRIX 1` in your keyboard's `config.h` or pass `-DCUSTOM_MATRIX=ON` to CMake.
2. Provide your custom `matrix_task(void *pvParameters)` and `matrix_is_pressed(row, col)` functions.
3. Queue events to `matrix_queue` using `matrix_event_t`:
   ```c
   matrix_event_t event = { .split = 0, .row = r, .col = c, .pressed = 1 };
   xQueueSend(matrix_queue, &event, 0);
   ```

### Hall Effect & Rapid Trigger Module (`hall_calibration`)
The module in `tests/modules/hall_calibration` provides:
* **Analog Endpoints Calibration:** `rest_adc` (top deadzone) and `bottom_adc` (bottom-out).
* **Configurable Actuation Point:** Dynamic 5%..95% travel distance.
* **Continuous Rapid Trigger:** Instant key release upon upward stroke detection (e.g. 0.1 mm) and immediate repeat actuation.
* **Non-volatile Flash Persistence:** Calibration profiles saved to dedicated Flash sector on RP2040/RP2350.

---

## 6. Displays (U8g2 & Sharp Memory LCD)

DMK includes `lib/u8g2` as a submodule for full monochrome and grayscale display support.

* **Sharp Memory LCD (`LS011B7DH03` 160x68):**
  * Uses native `u8g2_Setup_ls011b7dh03_160x68_f`.
  * Automatic `EXTCOMIN` 1 Hz polarity toggling task.
  * Real-time rolling WPM (Words Per Minute) calculation with speedometer bar.
  * Active layer name, CapsLock / NumLock badges, and USB status.

---

## 7. Programmable I/O (PIO) on Raspberry Pi Pico & RP2350

Any `.pio` assembly files placed in a module directory or custom keyboard directory are **automatically compiled** by DMK using `pioasm` and the Pico SDK `pico_generate_pio_header()`.

* Generated header files `*.pio.h` are immediately available for inclusion: `#include "my_driver.pio.h"`.
* All Pico SDK PIO APIs from `hardware/pio.h` are linked and ready to use.

---

## 8. Built-in Module Reference Examples

You can find reference implementations in the `tests/modules/` directory:

| Module Path | Description |
| :--- | :--- |
| `tests/modules/led_layer_indicator` | Controls discrete GPIO LEDs based on active layer. |
| `tests/modules/rgb_layer_indicator` | Dynamic RGB underglow/indicator color switching per layer. |
| `tests/modules/debug_indicator` | Arbitrary routing of USB mount and keypress events to GPIO LEDs or RGB indices. |
| `tests/modules/hall_calibration` | Full Hall-effect analog calibration, continuous rapid trigger, and Flash storage. |
| `tests/modules/sharp_memory_lcd` | Sharp MIP LCD dashboard with WPM calculator, layers, and USB status. |
| `tests/modules/u8g2_display` | Generic OLED/LCD display engine powered by U8g2. |
