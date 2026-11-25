🌐 **Language / Язык:** [English](modules.md) | [Русский](../ru/modules.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md) • [Modules](modules.md) • [MIDI](midi.md) • [Wizard](https://htmlpreview.github.io/?https://github.com/aroum/dmk/blob/main/editor/wizard.html)

---

# External Modules & Extensions System in DMK

DMK features a flexible external module system designed to allow adding custom hardware drivers, indicators, displays, sensors, and background tasks without modifying or polluting the core firmware codebase.

This architecture enables:

- **Zero Core Pollution**: Keep proprietary, experimental, or display-heavy code completely isolated from DMK core.
- **Seamless Upgrades**: Easily pull upstream DMK updates without merge conflicts.
- **Standalone User Config Repositories**: Maintain your personal keyboards, keymaps, and modules in a separate GitHub repository and build them with GitHub Actions.

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

### Keyboard-Internal Modules (`keyboards/<keyboard>/modules/`)

When a module belongs specifically to a given keyboard (such as `keyboards/omsk/modules/midi_jack` or custom matrix driver `keyboards/magneteno/modules/matrix_magneteno`):

- Place the module directly inside `keyboards/<keyboard>/modules/<module_name>/`.
- The DMK build system **automatically discovers and loads all submodules inside `keyboards/<keyboard>/modules/`** when building that keyboard!
- Passing `-DDMK_MODULES` or `-m` on the command line is **not required**:
  ```bash
  # Building omsk automatically includes keyboards/omsk/modules/midi_jack:
  ./build_all.sh -b omsk -c

  # Building magneteno automatically includes matrix_magneteno and hall_calibration:
  ./build_all.sh -b magneteno -c
  ```

---

## 3. Core Lifecycle & Event Hooks (Hooks API)

Modules register callbacks with `DMK_HOOK_SUBSCRIBER` in `dmk_core/include/hooks.h`; multiple subscribers can handle the same event. Legacy weak hooks remain supported and run first. Consuming hooks stop dispatch when consumed; report filters stop when they return false. Callback order between modules is unspecified:

| Hook Function                                                                                                                    | When It Is Called                                                | Typical Use Cases                                                                                 |
| :------------------------------------------------------------------------------------------------------------------------------- | :--------------------------------------------------------------- | :------------------------------------------------------------------------------------------------ |
| `void hook_early_init(void)`                                                                                                     | In `main.c` before `vTaskStartScheduler()`                       | Initializing custom GPIOs/buses, launching FreeRTOS background tasks                              |
| `void hook_layer_change(uint8_t active_layer)`                                                                                   | On every active layer switch (`layers.c`)                        | Layer LED/RGB color switching, OLED/LCD status updates                                            |
| `bool hook_matrix_change(uint8_t row, uint8_t col, bool pressed)`                                                                | On every physical switch state change (`matrix.c`)               | Keypress debug LEDs, haptic clickers, audio buzzers                                               |
| `bool hook_process_key(uint32_t keycode, bool pressed)`                                                                          | Before keycode is processed by core (`keyboard.c`)               | Custom keycodes (LED animations, toggles, triggers). Event is consumed when returning `true`      |
| `void hook_key_sent(uint16_t keycode, bool pressed)`                                                                             | When USB HID keycode is sent to host (`keys.c`)                  | Rolling WPM speed calculation, key logging, heatmaps                                              |
| `void hook_hid_led_change(uint8_t led_mask)`                                                                                     | When host updates Lock LEDs (`led.c`)                            | CapsLock (`0x02`), NumLock (`0x01`), ScrollLock (`0x04`) indicators                               |
| `bool hook_mouse_move(int8_t *dx, int8_t *dy)`                                                                                   | Before sending cursor movement report (`mouse.c`)                | Intercept trackball/mouse motion, drag-scroll (scroll wheel while holding layer/key), DPI scaling |
| `bool hook_mouse_scroll(int8_t *wheel, int8_t *pan)`                                                                             | Before sending wheel scroll report (`mouse.c`)                   | Invert or programmatically filter vertical and horizontal scrolling                               |
| `void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan)`                                        | Before transmitting composite USB HID Mouse report               | Analytics, click LED feedback, or mirroring to secondary interfaces                               |
| `void hook_gamepad_report(int8_t *x, int8_t *y, int8_t *z, int8_t *rz, int8_t *rx, int8_t *ry, uint8_t *hat, uint32_t *buttons)` | Before transmitting USB HID Gamepad report to host (`gamepad.c`) | Stick deadzone filtering, button remapping, analog axis calibration                               |
| `void hook_midi_send(const uint8_t *msg, uint8_t len)`                                                                           | When MIDI message is dispatched (`midi.c`)                       | Physical DIN-5 / TRS MIDI Jack output via UART, BLE MIDI, CV/Gate                                 |

### Hook Implementation Example:

```c
#include "hooks.h"
#include "rgb.h"
#include "hal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

#define LAYER_LED_INDEX 0
#define CAPS_LED_INDEX  1

static void on_layer_change(uint8_t active_layer) {
    const uint32_t layer_colors[] = { 0x00FF00, 0x0000FF, 0xFF00FF, 0xFFFF00 };
    uint32_t color = (active_layer < 4) ? layer_colors[active_layer] : 0xFFFFFF;
    rgb_set_pixel_raw(LAYER_LED_INDEX, color);
    rgb_show();
}

static void on_hid_led_change(uint8_t led_mask) {
    bool caps_on = (led_mask & 0x02) != 0;
    rgb_set_pixel_raw(CAPS_LED_INDEX, caps_on ? 0xFF0000 : 0x000000);
    rgb_show();
}
DMK_HOOK_SUBSCRIBER(indicator_hooks,
    .layer_change = on_layer_change,
    .hid_led_change = on_hid_led_change);
```

---

## 4. Hardware Abstraction & Core Services for Modules

Modules have full access to DMK core subsystems and FreeRTOS APIs:

### 4.1. Cross-Platform GPIO HAL (`hal_gpio.h`)

Unified GPIO operations across all supported MCUs (Milandr, RP2040, RP2350, nRF52840, Baikal):

- `void hal_gpio_init(uint32_t pin)` — Initializes pin for GPIO operation.
- `void hal_gpio_set_dir(uint32_t pin, bool out)` — Sets pin direction (`true` for output, `false` for input).
- `void hal_gpio_put(uint32_t pin, bool value)` — Drives pin high (`true`) or low (`false`).
- `bool hal_gpio_get(uint32_t pin)` — Reads current logical state of pin.

### 4.2. Cross-Platform ADC HAL (`hal_adc.h`)

Unified analog input interface for Hall effect sensors, analog joysticks, thumbsticks, and sliders:

- `void hal_adc_init(pin_t pin)` — Configures pin as an analog ADC input.
- `uint16_t hal_adc_read(pin_t pin)` — Reads normalized 12-bit analog voltage value (`0..4095`).

### 4.3. Cross-Platform I2C HAL (`hal_i2c.h`)

Universal hardware-independent bus abstraction for external sensors (IMUs, gyroscopes, trackballs), DACs, and OLEDs:

- `bool hal_i2c_init(pin_t sda, pin_t scl, uint32_t freq_hz)` — Initializes I2C master bus at `HAL_I2C_FREQ_STANDARD` (100 kHz) or `HAL_I2C_FREQ_FAST` (400 kHz). Bus operations are serialized; repeated initialization succeeds only with the same pins and frequency. Check the return value.
- `bool hal_i2c_write(uint8_t addr, const uint8_t *data, size_t len)` — Transmits data packet to a 7-bit slave address.
- `bool hal_i2c_read(uint8_t addr, uint8_t *data, size_t len)` — Receives data packet from a slave address.
- `bool hal_i2c_write_reg(uint8_t addr, uint8_t reg, const uint8_t *data, size_t len)` — Writes data into a specific register address.
- `bool hal_i2c_read_reg(uint8_t addr, uint8_t reg, uint8_t *data, size_t len)` — Reads data from a specific register address using repeated-start.

### 4.4. Cross-Platform SPI HAL (`hal_spi.h`)

High-speed hardware and bit-bang SPI master abstraction for displays (Sharp Memory LCD, ST7789), sensors, and Flash:

- `bool hal_spi_init(pin_t sck, pin_t mosi, pin_t miso, uint32_t freq_hz, uint8_t mode)` — Initializes SPI master bus in modes `HAL_SPI_MODE_0` .. `HAL_SPI_MODE_3` at speeds up to 10 MHz.
- `hal_spi_acquire()` / `hal_spi_release()` — Hold the SPI bus around chip select and all transfers in a transaction. Reinitialization with different parameters fails.
- `bool hal_spi_write(const uint8_t *tx, size_t len)` — Transmits data buffer (MISO discarded).
- `bool hal_spi_read(uint8_t rx_fill, uint8_t *rx, size_t len)` — Receives data by sending fill dummy bytes.
- `bool hal_spi_transfer(const uint8_t *tx, uint8_t *rx, size_t len)` — Full-duplex simultaneous transmit and receive.

### 4.5. FreeRTOS Tasks and Synchronization

Modules can create dedicated RTOS worker tasks during early initialization. Prefer static allocation: the task stack and control block are included in the firmware's RAM budget and do not consume the FreeRTOS heap.

```c
static void my_display_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        // Perform non-blocking updates
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static StaticTask_t display_tcb;
static StackType_t display_stack[512]; // stack depth in StackType_t words
static void my_early_init(void) {
    if (xTaskCreateStatic(my_display_task, "display", 512, NULL, tskIDLE_PRIORITY + 1,
                          display_stack, &display_tcb) == NULL) {
        return; // optional module could not start
    }
}
DMK_HOOK_SUBSCRIBER(display_hooks, .early_init = my_early_init);
```

The default FreeRTOS dynamic heap is 8 KiB (3 KiB on Baikal); override `configTOTAL_HEAP_SIZE` at build time, for example `-d 'configTOTAL_HEAP_SIZE=4096'`. The linker rejects configurations that exceed their memory regions. If a dynamic allocation fails, FreeRTOS returns `NULL`/`pdFAIL` and increments `dmk_malloc_failure_count` rather than hanging; check every dynamic allocation result. Stack overflow resets the MCU. On Baikal, the FreeRTOS heap, interrupt stack and separate 2 KiB bounded newlib heap reside in TCM; ordinary static data resides in SRAM.

### 4.6. Direct RGB LED Control (`rgb.h`)

Low-level direct pixel manipulation without interfering with standard lighting animations:

- `void rgb_set_pixel_raw(uint32_t index, uint32_t color_hex)` — Sets color in `0xRRGGBB` format.
- `void rgb_clear_pixel_raw(uint32_t index)` — Removes a persistent raw pixel overlay, returning that LED to the animation.
- `void rgb_show(void)` — Flushes buffer to hardware (WS2812 / SK6812).
- `void rgb_set_color(uint8_t hue, uint8_t sat)` — Sets global color in HSV space.
- `void rgb_set_mode(uint8_t mode)` — Selects lighting effect mode.

### 4.7. Mouse Emulation & Direct Trackball API (`mouse.h`)

DMK includes a native USB HID mouse subsystem supporting concurrent motion, wheel scrolling (vertical and horizontal via AC Pan), and 5 mouse buttons:

- `void mouse_move(int8_t dx, int8_t dy)` — Relative mouse cursor movement. Invokes `hook_mouse_move(&dx, &dy)`, allowing modules to intercept motion (e.g. redirect trackball movement to scrolling) or scale sensitivity.
- `void mouse_scroll(int8_t wheel, int8_t pan)` — Wheel scrolling (`wheel` for vertical, `pan` for horizontal). Invokes `hook_mouse_scroll(&wheel, &pan)`.
- `void mouse_button_set(uint8_t button_mask, bool pressed)` — Sets button bitmask (`MOUSE_BTN_LEFT`, `MOUSE_BTN_RIGHT`, `MOUSE_BTN_MIDDLE`, `MOUSE_BTN_BACK`, `MOUSE_BTN_FORWARD`).
- `void mouse_button_press(uint8_t button_mask)` / `mouse_button_release(uint8_t button_mask)` — Instant button press and release.

**Keymap Mousekeys:**

- **Buttons:** `K_MS_BTN1`..`K_MS_BTN5` (aliases `K_BTN1`..`K_BTN5`).
- **Cursor Movement:** `K_MS_UP`, `K_MS_DOWN`, `K_MS_LEFT`, `K_MS_RIGHT` (smooth non-linear acceleration physics configurable via `MOUSEKEY_BASE_SPEED`, `MOUSEKEY_MAX_SPEED`, `MOUSEKEY_TIME_TO_MAX`).
- **Wheel Scrolling:** `K_MS_WH_UP`, `K_MS_WH_DOWN`, `K_MS_WH_LEFT`, `K_MS_WH_RIGHT`.
- **Speed Modes:** `K_MS_ACCEL0` (Slow / precision pixel mode), `K_MS_ACCEL1` (Normal), `K_MS_ACCEL2` (Fast / turbo mode).

### 4.8. Standard USB HID Gamepad (DirectInput) (`gamepad.h`)

DMK includes native DirectInput controller support featuring 6 analog axes, an 8-way Hat Switch (D-Pad), and 32 physical buttons:

- `void gamepad_set_axis_left(int8_t x, int8_t y)` — Sets left analog stick position (`-127..127`).
- `void gamepad_set_axis_right(int8_t z, int8_t rz)` — Sets right analog stick position (`-127..127`).
- `void gamepad_set_triggers(int8_t rx, int8_t ry)` — Sets analog trigger positions (`-127..127`).
- `void gamepad_set_dpad(uint8_t hat)` — Sets D-Pad direction (`GAMEPAD_HAT_CENTER`, `UP`, `UP_RIGHT`, `RIGHT`, `DOWN_RIGHT`, `DOWN`, `DOWN_LEFT`, `LEFT`, `UP_LEFT`).
- `void gamepad_set_button(uint8_t button_num, bool pressed)` — Sets state of a button (1..32).
- `void gamepad_press_button(uint8_t button_num)` / `gamepad_release_button(uint8_t button_num)` — Instant button press and release.
- `void gamepad_send(void)` — Transmits current gamepad report to host (invokes `hook_gamepad_report(...)`).

**Keymap Gamepad Keys:**

- **Buttons:** `GP_BTN1`..`GP_BTN32`, controller aliases `GP_A`, `GP_B`, `GP_X`, `GP_Y`, `GP_LB`, `GP_RB`, `GP_SELECT`, `GP_START`, `GP_L3`, `GP_R3`.
- **D-Pad:** `GP_DPAD_UP`, `GP_DPAD_DOWN`, `GP_DPAD_LEFT`, `GP_DPAD_RIGHT` (with automatic diagonal resolution).
- **Stick Simulation:** `GP_LX_UP`, `GP_LX_DOWN`, `GP_LX_LEFT`, `GP_LX_RIGHT`, `GP_LY_UP`, `GP_LY_DOWN`, `GP_LY_LEFT`, `GP_LY_RIGHT`, `GP_RX_*`, `GP_RY_*`.
- **Triggers:** `GP_LT`, `GP_RT`.

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
       xQueueSend(matrix_queue, &event, portMAX_DELAY);
   }
   ```

### Hall Effect & Rapid Trigger Reference (`tests/modules/hall_calibration`)

The module in `tests/modules/hall_calibration` provides:

- **Dynamic Endpoints Calibration:** Automated `rest_adc` (deadzone) and `bottom_adc` (bottom-out travel).
- **Configurable Actuation Point:** Dynamic 5%..95% keystroke travel threshold.
- **Continuous Rapid Trigger:** Key deactivates instantly on upward movement (e.g. 0.1 mm release travel) and reactivates immediately on downward stroke.
- **Non-volatile Flash Persistence:** Calibration profiles saved directly to dedicated Flash memory on RP2040/RP2350.

---

## 6. Displays (U8g2 & Sharp Memory LCD)

DMK includes `lib/u8g2` as a submodule for full monochrome and grayscale display support.

- **Sharp Memory LCD (`LS011B7DH03` 160x68):**
  - Uses native driver `u8g2_Setup_ls011b7dh03_160x68_f`.
  - Background FreeRTOS task handling 1 Hz `EXTCOMIN` toggle to prevent DC bias crystallization.
  - Real-time rolling WPM (Words Per Minute) calculation with visual speedometer bar.
  - Layer name badges, `[CAPS]`, `[NUM]`, and USB connection status indicators.

---

## 7. Programmable I/O (PIO) on Raspberry Pi Pico & RP2350

Any `.pio` assembly file located inside your module directory is **automatically detected and compiled** during CMake configuration using `pioasm` and Pico SDK's `pico_generate_pio_header()`.

- Generated header files `*.pio.h` can be immediately included: `#include "my_driver.pio.h"`.
- Pico SDK hardware libraries (`hardware_pio`, `hardware_dma`, `hardware_timer`) are linked and ready to use.

---

## 8. Built-in Reference Modules

The repository includes tested reference implementations in `tests/modules/`:

| Module Path                                    | Description                                                                                          |
| :--------------------------------------------- | :--------------------------------------------------------------------------------------------------- |
| `tests/modules/led_layer_indicator`            | Controls discrete GPIO LEDs according to active layer index.                                         |
| `tests/modules/rgb_layer_indicator`            | Dynamic RGB underglow / indicator color switching per layer.                                         |
| `tests/modules/debug_indicator`                | Routing USB mount and keypress events to GPIO LEDs or RGB indices.                                   |
| `tests/modules/hall_calibration`               | Full Hall-effect analog calibration, continuous Rapid Trigger, and Flash storage.                    |
| `tests/modules/sharp_memory_lcd`               | Sharp MIP LCD dashboard with WPM calculator, layers, and status indicators.                          |
| `tests/modules/u8g2_display`                   | Generic OLED/LCD display engine powered by U8g2.                                                     |
| `tests/modules/trackball_example`              | Trackball/optical sensor integration with `hook_mouse_move` for drag-scroll.                         |
| `tests/modules/bmi270_airmouse`                | Air mouse powered by Bosch BMI270 6-DoF IMU gyroscope over universal `hal_i2c.h` and `mouse_move()`. |
| `keyboards/omsk/modules/midi_jack`             | Physical DIN-5 / TRS MIDI Jack transport over hardware UART (31250 baud) via `hook_midi_send`.       |
| `keyboards/magneteno/modules/matrix_magneteno` | Custom Hall-Effect matrix scanner using SN74LV4052A analog multiplexer.                              |

## 9. Building Modules in Isolated Repositories & CI/CD

Custom external modules do not need to reside inside the DMK core source tree. You can place them in your standalone user configuration repository (e.g. inside a `modules/my_module` directory) and pass their paths to CMake using the `-DDMK_MODULES` flag.

For complete repository layout instructions and a ready-to-use **GitHub Actions CI/CD** workflow matrix template, see the [Build Guide (build.md)](build.md#automated-build-in-custom-repository-github-actions).

## External MCU platform packages

A new MCU can live outside DMK, including in a separate keyboard-configuration repository. A **platform package** is loaded before CMake `project()` so it can select the compiler, SDK, startup and FreeRTOS port. An ordinary `module.cmake` runs after the firmware target exists and is still the right place for optional keyboard features.

### Repository layout

```text
my-configs/
├── keyboards/my_keyboard/
│   ├── config.h
│   ├── matrix_custom.c                 # optional custom matrix
│   └── modules/my_feature/module.cmake # ordinary modules still work
└── platforms/my_mcu/
    ├── platform.cmake                  # required, early platform manifest
    ├── platform.c                      # clock/init/reset/bootloader functions
    ├── startup.S
    ├── memory.ld
    ├── freertos_port/                  # matching FreeRTOS CPU/compiler port
    ├── include/
    │   ├── pin_defs.h                  # pin_t and board pin names
    │   ├── FreeRTOSConfig.h            # port-specific scheduler configuration
    │   └── tusb_config.h               # when using TinyUSB
    ├── drivers/
    │   ├── gpio.c                      # implements DMK's hal_gpio.h API
    │   ├── adc.c                       # optional, if ADC is used
    │   ├── i2c.c                       # optional, if I2C is used
    │   └── spi.c                       # optional, if SPI is used
    └── sdk/                            # SDK/submodule or other dependency
```

In `keyboards/my_keyboard/config.h`, select an MCU identifier and optionally the platform path:

```c
#define DEFAULT_MCU my_mcu
#define DMK_PLATFORM_DIR "../../platforms/my_mcu"
```

Use a single unconditional literal define for MCU/platform selection; conditional preprocessor branches are not evaluated by this discovery step. The quoted path is relative to that `config.h` directory, not to the DMK checkout or build directory. It is used by CMake; there is no runtime platform loader. Keep the MCU identifier a C identifier, such as `my_mcu`.

Selection order is:

1. Explicit CMake `DMK_PLATFORM_DIR` / CLI `--platform-dir`.
2. The literal `#define DMK_PLATFORM_DIR "..."` line in the selected config.
3. `<keyboard>/platforms/<MCU>`, then `<keyboard>/platform`.
4. `<configuration Git repository root>/platforms/<MCU>`.
5. DMK's built-in platform directory.

An explicitly selected package without `platform.cmake` fails early; it does not silently fall back. Auto-discovery at the Git root works for checked-out repositories. For archives without Git metadata, use the config-relative define or explicit path. A repository containing several keyboards should select the desired keyboard directory explicitly.

### Platform manifest contract

`platform.cmake` runs **before** `project()`. Set the toolchain/compiler and all platform source paths here; use absolute paths based on `${PLATFORM_DIR}`, `${DMK_ROOT}` or `${CMAKE_CURRENT_LIST_DIR}`. Do not create a second firmware executable.

Provide these variables:

- `PLATFORM_SRC`: startup, clock/init code, SDK drivers, FreeRTOS port, USB stack/DCD and transport sources. `${FREERTOS_COMMON_SRC}` supplies DMK's common FreeRTOS kernel files; add it exactly once if using that kernel.
- `PLATFORM_INC`: SDK, port and configuration include directories. They precede DMK's common include paths, so the package can supply `pin_defs.h`, `FreeRTOSConfig.h` and USB-stack configuration.
- `PLATFORM_DEFS`: target compiler definitions, if needed.
- `PLATFORM_LIBS`: SDK libraries and linker requirements, including the linker script where appropriate.

A real port must implement the interfaces used by the selected features, not just provide a manifest. GPIO and platform entry points are declared in `dmk_core/include/hal_gpio.h` (`platform_init`, reset/bootloader, GPIO, delays and snapshots). ADC/I2C/SPI contracts are in the corresponding `hal_*.h` headers. Keep those shared API signatures and types consistent. Supply correct interrupt handlers, startup/vector table, clocks and USB DCD. The FreeRTOS configuration must match the selected port, CPU clock, interrupt priorities, memory budget and DMK allocation/hooks; the built-in MCU-specific defaults are not a substitute for a new port.

If using DMK's TinyUSB transport, include `dmk_core/drivers/app_usb_tinyusb.c`, `usb_descriptors.c` and the required TinyUSB/MCU sources in `PLATFORM_SRC`. If replacing that transport, provide its `app_usb_hid.h` API and any enabled VIA/console functionality yourself.

Optional hooks:

```cmake
# After the firmware target and its normal modules are configured:
function(platform_configure_target target)
    target_compile_definitions(${target} PRIVATE MY_PORT_OPTION=1)
    # target_link_libraries(${target} PRIVATE my_sdk)
endfunction()

# Attach your artifact generation (objcopy, UF2, etc.):
function(platform_post_build target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary
                $<TARGET_FILE:${target}> ${CMAKE_BINARY_DIR}/${target}.bin)
endfunction()
```

### Replacing hardware drivers

For an external package, `drivers/` is used automatically. A file with the same name replaces a selected source under `dmk_core/drivers/`, in both core and platform source lists. For example, `drivers/gpio.c` replaces DMK's MCU-dispatching `gpio.c`; `drivers/app_usb_tinyusb.c` can replace the default transport. Features such as layers and HT are not replaced by this mechanism. Optional bus drivers are selected only when the keyboard/modules use them.

To use another directory, set `DMK_PLATFORM_DRIVER_DIR` in the manifest (relative to the platform package, or absolute). To supply several implementations in one source/library, set `DMK_PLATFORM_EXCLUDE_DRIVERS` to a list of default driver filenames and include your replacement code in `PLATFORM_SRC`/`PLATFORM_LIBS`:

```cmake
set(DMK_PLATFORM_EXCLUDE_DRIVERS gpio.c adc.c)
list(APPEND PLATFORM_SRC "${PLATFORM_DIR}/hal_combined.c")
```

Excluding a driver does not disable the feature: its API must still be implemented. Implement storage, RGB and split support if the configuration enables them; they are not supplied automatically for arbitrary silicon. Driver selection, SDK initialization and feature dependencies belong to the platform package. Preserve the licenses of SDK/RTOS/borrowed port sources.

### Local and remote-config builds

No copying into DMK's `keyboards/` or `platforms/` is required:

```bash
./build_all.sh -b /path/to/my-configs/keyboards/my_keyboard \
  --build-dir /path/to/build-my-mcu

# Explicit package override:
./build_all.sh -b /path/to/my-configs/keyboards/my_keyboard \
  --platform-dir /path/to/my-platform --build-dir /path/to/build-my-mcu

# Direct CMake works too:
cmake -S /path/to/dmk -B /path/to/build-my-mcu \
  -DKEYBOARD_DIR=/path/to/my-configs/keyboards/my_keyboard -DMCU=my_mcu
cmake --build /path/to/build-my-mcu --parallel
```

Use a fresh/separate build directory when changing toolchains. The platform's post-build hook controls output formats. Built-in USB-flash/programmer commands in `build_all.sh` do not automatically support a new MCU; use the port's own flashing procedure.

In a remote configuration repository, check out both repositories and build directly from the config checkout. For example, the core steps of its GitHub Actions job are:

```yaml
- uses: actions/checkout@v7
  with:
    path: configs
    submodules: recursive
- uses: actions/checkout@v7
  with:
    repository: aroum/dmk
    ref: dev # pin a tested commit for reproducible builds
    path: dmk
    submodules: recursive
# Install the toolchain/SDK dependencies required by your platform, plus uv.
- uses: astral-sh/setup-uv@v10.3.3
- run: |
    bash dmk/build_all.sh \
      -b "$GITHUB_WORKSPACE/configs/keyboards/my_keyboard" \
      --build-dir "$GITHUB_WORKSPACE/build-my-mcu"
```

A separately maintained platform repository can also be checked out/submoduled and selected with `--platform-dir`; DMK does not implicitly download SDKs or execute remote manifests by URL.

The tested example in `tests/platform_repo_example/` uses the new identifier `external_demo` and a reused STM32 SDK only as a build fixture. Do not flash it to a real keyboard. CI clones it as an independent config repository and builds an ARM ELF, `.bin` and `.hex`, including a replacement HAL and an ordinary module. Run that integration test locally with:

```bash
DMK_TEST_EXTERNAL_ARM=1 uv run python3 -m unittest tests.test_external_platform
```

Install Arm GCC/newlib and initialize DMK's SDK submodules first. Host tests cover selection, path precedence, driver replacement and missing-manifest errors without requiring an ARM compiler. A successful build does not verify a new MCU electrically or validate its USB/clock/flash timing.
