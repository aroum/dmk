🌐 **Language / Язык:** [English](modules.md) | [Русский](../ru/modules.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md) • [Modules](modules.md) • [MIDI](midi.md)

---

# External Modules & Extensions System in DMK

DMK features a flexible external module system designed to allow adding custom hardware drivers, indicators, displays, sensors, and background tasks without modifying or polluting the core firmware codebase.

This architecture enables:
* **Zero Core Pollution**: Keep proprietary, experimental, or display-heavy code completely isolated from DMK core.
* **Seamless Upgrades**: Easily pull upstream DMK updates without merge conflicts.
* **Standalone User Config Repositories**: Maintain your personal keyboards, keymaps, and modules in a separate GitHub repository and build them with GitHub Actions.

---

## 1. Module Structure & Anatomy

An external module is a self-contained directory containing a `module.cmake` manifest file along with its source code and header files.

```text
my_custom_module/
├── module.cmake           # CMake build instructions for the module
├── include/               # Public headers (automatically added to include path)
│   └── my_module.h
├── my_module.c            # Module source code
├── custom_driver.pio      # (Optional) PIO assembly program for RP2040 / RP2350
└── README.md              # (Optional) Module documentation
```

### `module.cmake` Syntax
Inside `module.cmake`, you use standard CMake target commands to attach sources, include directories, compiler definitions, or external libraries to `${TARGET_NAME}`:

```cmake
# Add module source files
target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/my_module.c"
)

# Add private include directories (module root and include/ are added automatically by DMK)
target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Optional: compile definitions or external libraries
# target_compile_definitions(${TARGET_NAME} PRIVATE HAS_MY_MODULE=1)
# target_link_libraries(${TARGET_NAME} PRIVATE hardware_adc)
```

---

## 2. Enabling Modules during Build

Pass a semicolon-separated list of module directory paths to CMake using the `-DDMK_MODULES` flag:

```bash
# Single module (relative or absolute path)
cmake -B build -DKEYBOARD=corne -DDMK_MODULES=tests/modules/led_layer_indicator -DMCU=rp2040

# Multiple modules (enclosed in quotes and separated by semicolon)
cmake -B build -DKEYBOARD=magneteno -DDMK_MODULES="tests/modules/hall_calibration;tests/modules/sharp_memory_lcd" -DMCU=rp2350
```

Both relative paths (resolved relative to repository root) and absolute paths (e.g. `/home/user/my_modules/my_module`) are supported.

---

## 3. Core Lifecycle & Event Hooks (Hooks API)

Modules communicate with DMK through non-blocking weak hooks declared in `dmk_core/include/hooks.h`. Modules simply implement any of these functions without requiring glue code or core edits:

| Hook Function | When It Is Called | Typical Use Cases |
| :--- | :--- | :--- |
| `void hook_early_init(void)` | In `main.c` before `vTaskStartScheduler()` | Initializing custom GPIOs/buses, launching FreeRTOS background tasks |
| `void hook_layer_change(uint8_t active_layer)` | On every active layer switch (`layers.c`) | Layer LED/RGB color switching, OLED/LCD status updates |
| `void hook_matrix_change(uint8_t row, uint8_t col, bool pressed)` | On every physical switch state change (`matrix.c`) | Keypress debug LEDs, haptic clickers, audio buzzers |
| `void hook_key_sent(uint16_t keycode, bool pressed)` | When USB HID keycode is sent to host (`keys.c`) | Rolling WPM speed calculation, key logging, heatmaps |
| `void hook_hid_led_change(uint8_t led_mask)` | When host updates Lock LEDs (`led.c`) | CapsLock (`0x02`), NumLock (`0x01`), ScrollLock (`0x04`) indicators |
| `bool hook_mouse_move(int8_t *dx, int8_t *dy)` | Before sending cursor movement report (`mouse.c`) | Intercept trackball/mouse motion, drag-scroll (scroll wheel while holding layer/key), DPI scaling |
| `bool hook_mouse_scroll(int8_t *wheel, int8_t *pan)` | Before sending wheel scroll report (`mouse.c`) | Invert or programmatically filter vertical and horizontal scrolling |
| `void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan)` | Before transmitting composite USB HID Mouse report | Analytics, click LED feedback, or mirroring to secondary interfaces |

### Hook Implementation Example:
```c
#include "hooks.h"
#include "rgb.h"
#include "hal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

#define LAYER_LED_INDEX 0
#define CAPS_LED_INDEX  1

void hook_layer_change(uint8_t active_layer) {
    const uint32_t layer_colors[] = { 0x00FF00, 0x0000FF, 0xFF00FF, 0xFFFF00 };
    uint32_t color = (active_layer < 4) ? layer_colors[active_layer] : 0xFFFFFF;
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

## 4. Hardware Abstraction & Core Services for Modules

Modules have full access to DMK core subsystems and FreeRTOS APIs:

### 4.1. Cross-Platform GPIO HAL (`hal_gpio.h`)
Unified GPIO operations across all supported MCUs (Milandr, RP2040, RP2350, nRF52840, Baikal):
* `void hal_gpio_init(uint32_t pin)` — Initializes pin for GPIO operation.
* `void hal_gpio_set_dir(uint32_t pin, bool out)` — Sets pin direction (`true` for output, `false` for input).
* `void hal_gpio_put(uint32_t pin, bool value)` — Drives pin high (`true`) or low (`false`).
* `bool hal_gpio_get(uint32_t pin)` — Reads current logical state of pin.

### 4.2. FreeRTOS Tasks and Synchronization
Modules can spawn dedicated RTOS worker tasks in `hook_early_init()`:
```c
static void my_display_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        // Perform non-blocking updates
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void hook_early_init(void) {
    xTaskCreate(my_display_task, "display", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}
```

### 4.3. Direct RGB LED Control (`rgb.h`)
Low-level direct pixel manipulation without interfering with standard lighting animations:
* `void rgb_set_pixel_raw(uint32_t index, uint32_t color_hex)` — Sets color in `0xRRGGBB` format.
* `void rgb_show(void)` — Flushes buffer to hardware (WS2812 / SK6812).
* `void rgb_set_color(uint8_t hue, uint8_t sat)` — Sets global color in HSV space.
* `void rgb_set_mode(uint8_t mode)` — Selects lighting effect mode.

### 4.4. Mouse Emulation & Direct Trackball API (`mouse.h`)
DMK includes a native USB HID mouse subsystem supporting concurrent motion, wheel scrolling (vertical and horizontal via AC Pan), and 5 mouse buttons:
* `void mouse_move(int8_t dx, int8_t dy)` — Relative mouse cursor movement. Invokes `hook_mouse_move(&dx, &dy)`, allowing modules to intercept motion (e.g. redirect trackball movement to scrolling) or scale sensitivity.
* `void mouse_scroll(int8_t wheel, int8_t pan)` — Wheel scrolling (`wheel` for vertical, `pan` for horizontal). Invokes `hook_mouse_scroll(&wheel, &pan)`.
* `void mouse_button_set(uint8_t button_mask, bool pressed)` — Sets button bitmask (`MOUSE_BTN_LEFT`, `MOUSE_BTN_RIGHT`, `MOUSE_BTN_MIDDLE`, `MOUSE_BTN_BACK`, `MOUSE_BTN_FORWARD`).
* `void mouse_button_press(uint8_t button_mask)` / `mouse_button_release(uint8_t button_mask)` — Instant button press and release.

**Keymap Mousekeys:**
* **Buttons:** `K_MS_BTN1`..`K_MS_BTN5` (aliases `K_BTN1`..`K_BTN5`).
* **Cursor Movement:** `K_MS_UP`, `K_MS_DOWN`, `K_MS_LEFT`, `K_MS_RIGHT` (smooth non-linear acceleration physics configurable via `MOUSEKEY_BASE_SPEED`, `MOUSEKEY_MAX_SPEED`, `MOUSEKEY_TIME_TO_MAX`).
* **Wheel Scrolling:** `K_MS_WH_UP`, `K_MS_WH_DOWN`, `K_MS_WH_LEFT`, `K_MS_WH_RIGHT`.
* **Speed Modes:** `K_MS_ACCEL0` (Slow / precision pixel mode), `K_MS_ACCEL1` (Normal), `K_MS_ACCEL2` (Fast / turbo mode).

---

## 5. Custom Matrix Drivers (Hall Effect, Rapid Trigger, MUX, Trackballs)

For non-standard keyboard matrices (e.g. analog Hall Effect switches, multiplexers, trackballs):
1. Set `#define CUSTOM_MATRIX 1` in `config.h` or pass `-DCUSTOM_MATRIX=ON` to CMake. This excludes the standard `dmk_core/drivers/matrix.c` from the build.
2. Implement custom `matrix_task(void *pvParameters)` and `bool matrix_is_pressed(uint8_t row, uint8_t col)`.
3. Push key events into DMK's `matrix_queue` using `matrix_event_t`:
   ```c
   #include "matrix.h"
   #include "queue.h"

   extern QueueHandle_t matrix_queue;

   void send_key_event(uint8_t row, uint8_t col, bool pressed) {
       matrix_event_t event = {
           .split = 0,
           .row = row,
           .col = col,
           .pressed = pressed ? 1 : 0
       };
       xQueueSend(matrix_queue, &event, 0);
   }
   ```

### Hall Effect & Rapid Trigger Reference (`tests/modules/hall_calibration`)
The module in `tests/modules/hall_calibration` provides:
* **Dynamic Endpoints Calibration:** Automated `rest_adc` (deadzone) and `bottom_adc` (bottom-out travel).
* **Configurable Actuation Point:** Dynamic 5%..95% keystroke travel threshold.
* **Continuous Rapid Trigger:** Key deactivates instantly on upward movement (e.g. 0.1 mm release travel) and reactivates immediately on downward stroke.
* **Non-volatile Flash Persistence:** Calibration profiles saved directly to dedicated Flash memory on RP2040/RP2350.

---

## 6. Displays (U8g2 & Sharp Memory LCD)

DMK includes `lib/u8g2` as a submodule for full monochrome and grayscale display support.

* **Sharp Memory LCD (`LS011B7DH03` 160x68):**
  * Uses native driver `u8g2_Setup_ls011b7dh03_160x68_f`.
  * Background FreeRTOS task handling 1 Hz `EXTCOMIN` toggle to prevent DC bias crystallization.
  * Real-time rolling WPM (Words Per Minute) calculation with visual speedometer bar.
  * Layer name badges, `[CAPS]`, `[NUM]`, and USB connection status indicators.

---

## 7. Programmable I/O (PIO) on Raspberry Pi Pico & RP2350

Any `.pio` assembly file located inside your module directory is **automatically detected and compiled** during CMake configuration using `pioasm` and Pico SDK's `pico_generate_pio_header()`.

* Generated header files `*.pio.h` can be immediately included: `#include "my_driver.pio.h"`.
* Pico SDK hardware libraries (`hardware_pio`, `hardware_dma`, `hardware_timer`) are linked and ready to use.

---

## 8. Built-in Reference Modules

The repository includes tested reference implementations in `tests/modules/`:

| Module Path | Description |
| :--- | :--- |
| `tests/modules/led_layer_indicator` | Controls discrete GPIO LEDs according to active layer index. |
| `tests/modules/rgb_layer_indicator` | Dynamic RGB underglow / indicator color switching per layer. |
| `tests/modules/debug_indicator` | Routing USB mount and keypress events to GPIO LEDs or RGB indices. |
| `tests/modules/hall_calibration` | Full Hall-effect analog calibration, continuous Rapid Trigger, and Flash storage. |
| `tests/modules/sharp_memory_lcd` | Sharp MIP LCD dashboard with WPM calculator, layers, and status indicators. |
| `tests/modules/u8g2_display` | Generic OLED/LCD display engine powered by U8g2. |
| `tests/modules/trackball_example` | Trackball / optical sensor integration with `hook_mouse_move` for drag-scroll on layers. |

## 9. Building Modules in Isolated Repositories & CI/CD

Custom external modules do not need to reside inside the DMK core source tree. You can place them in your standalone user configuration repository (e.g. inside a `modules/my_module` directory) and pass their paths to CMake using the `-DDMK_MODULES` flag.

For complete repository layout instructions and a ready-to-use **GitHub Actions CI/CD** workflow matrix template, see the [Build Guide (build.md)](build.md#automated-build-in-custom-repository-github-actions).

