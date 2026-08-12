🌐 **Language / Язык:** [English](keymap.md) | [Русский](../ru/keymap.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

---

# Keymap and Behaviors Guide

## Logical Keymaps and Layers

To avoid duplicate variable allocation across multiple compilation units, keymaps, macros, and layer sizes are defined within a `#ifdef DEFINE_KEYMAP` block inside `config.h`. The core framework defines `DEFINE_KEYMAP` prior to including `config.h` inside `brain.c` to instantiate the variables.

### Inlining Keymaps

Keymaps are defined directly as a 3D array: `const uint32_t keymap[][NUM_ROWS][NUM_COLS]`.

```c
#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

const uint32_t keymap[][NUM_ROWS][NUM_COLS] = {
    [DEF] = {
       { K_Q,    K_W,    K_E,    K_R,    K_T,         K_Y,    K_U,    K_I,    K_O,   K_P },
       { K_A,    K_S,    K_D,    K_F,    K_G,         K_H,    K_J,    K_K,    K_L,   K_ENT },
       { K_Z,    K_X,    K_C,    K_V,    K_B,         K_N,    K_M,    K_ALT,  K_CTRL,K_SHFT },
       { K_NO,   K_NO,   K_NO,   K_BSPC, FN1,         K_SPC,  K_TRNS, K_NO,  K_NO,  K_NO }
    },
    [FN1] = {
       { K_ESC,  K_APOS, K_A_U,  K_BRAL, K_BRAR,      K_7,    K_8,    K_9,    K_MIN, K_SLSH },
       { K_TAB,  K_A_L,  K_A_D,  K_A_R,  K_HASH,      K_4,    K_5,    K_6,    K_EQU, K_ENT },
       { K_SHFT, K_BKSL, K_SCLN, K_COMM, K_DOT,       K_1,    K_2,    K_3,    K_ALT, K_SHFT },
       { K_NO,   K_NO,   K_NO,   K_BSPC, K_TRNS,      K_0,    K_TRNS, K_NO,  K_NO,  K_NO }
    }
};

const size_t keymap_layers = sizeof(keymap) / sizeof(keymap[0]);
#endif
```

> [!NOTE]
> For simple layer switching without modifiers, you can use the `L_0` to `L_15` keycodes directly (e.g. `L_1` to momentarily switch to layer 1). These are a lighter alternative to the `MO(layer)` macro and process instantly in the matrix loop.
>
> See [keycodes_en.md](keycodes_en.md) for a list of all keycodes.

### Optional External Keymap

If you want to keep keymaps in a separate file, the framework supports a hook to include `keymap_external.h` if it is present in the keyboard's directory:

```c
#ifdef DEFINE_KEYMAP
#if __has_include("keymap_external.h")
    #include "keymap_external.h"
#else
    // Inline keymap definitions...
#endif
#endif
```

---

## Macros

The firmware implements a macro execution engine that allows simulating sequences of key presses and releases with delays (in milliseconds).

### Defining Macro Steps

Each macro step (macro action) is described by the `MacroStep` structure and consists of an action type (`MacroAction`) and an associated value (keycode or delay time in ms):

- **`KEY_DOWN`** (or the helper macro `M_DN(key)`) — press and hold a key.
- **`KEY_UP`** (or the helper macro `M_UP(key)`) — release a key.
- **`DELAY`** (or the helper macro `M_D(ms)`) — delay execution of the next step in milliseconds.

#### Example of macro step declaration:

```c
#include "keys.h"

// Macro to emulate Alt + Tab sequence
const MacroStep MACRO_ALT_TAB[] = {
    M_DN(K_ALT),   // Hold Alt
    M_D(200),      // Wait 200 ms
    M_DN(K_TAB),   // Hold Tab
    M_D(200),      // Wait 200 ms
    M_UP(K_TAB),   // Release Tab
    M_UP(K_ALT)    // Release Alt
};
```

### Registering Macros in config.h

For the firmware to recognize your macros, they must be registered in the global `keyboard_macros` array in your keyboard's [config.h](config_en.md) file.

```c
// 1. Declare macro steps arrays
const MacroStep MACRO_ALT_TAB[] = {
    M_DN(K_ALT), M_D(200), M_DN(K_TAB), M_D(200), M_UP(K_TAB), M_UP(K_ALT)
};

const MacroStep MACRO_HELLO[] = {
    M_DN(K_H), M_UP(K_H),
    M_DN(K_E), M_UP(K_E),
    M_DN(K_L), M_UP(K_L),
    M_DN(K_L), M_UP(K_L),
    M_DN(K_O), M_UP(K_O)
};

// 2. Register them in keyboard_macros array
const Macro keyboard_macros[] = {
    [0] = { MACRO_ALT_TAB, sizeof(MACRO_ALT_TAB) / sizeof(MACRO_ALT_TAB[0]) },
    [1] = { MACRO_HELLO, sizeof(MACRO_HELLO) / sizeof(MACRO_HELLO[0]) }
};

// 3. Define total macro count
const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
```

### Using Macros in the Keymap

To bind a macro to a specific key, use the `M(index)` macro, where `index` is the sequential index of the macro in the `keyboard_macros` array (starting from `0`):

```c
const uint32_t keymap[][NUM_ROWS][NUM_COLS] = {
    [0] = {
        { K_Q, K_W, K_E, K_R, K_T, K_Y, K_U, K_I, K_O, M(0) }, // M(0) runs Alt+Tab
        { K_A, K_S, K_D, K_F, K_G, K_H, K_J, K_K, K_L, M(1) }, // M(1) types "hello"
        ...
    }
};
```

> [!TIP]
> **Language layout switching via macros and layers:**
> You can use a macro to set up a dedicated layout for another OS language. The macro sends an OS language shortcut (e.g., `Shift + Alt` or `Win + Space`) while simultaneously toggling a firmware layer (e.g., via `TG(1)` or `MO(1)`):
>
> ```c
> // Macro: Toggle OS language (Shift + Alt) and activate Layer 1
> const MacroStep MACRO_SWITCH_LANG_RU[] = {
>     M_DN(K_LSFT),
>     M_DN(K_LALT),
>     M_UP(K_LALT),
>     M_UP(K_LSFT),
>     M_DN(TG(1)), // Toggle Layer 1
>     M_UP(TG(1))
> };
> ```
>
> This enables separate layers customized for specific OS language keymaps while keeping the OS and firmware layer state synchronized.

## Chords

```c
// Pack three coordinates into a single byte (8 bits)
typedef struct {
    uint8_t row   : 2; // 2 bits for rows (0..3)
    uint8_t col   : 4; // 4 bits for columns (0..15)
    uint8_t layer : 2; // 2 bits for layers (0..3)
} PackedKey;
```

```c
#include <stdint.h>
#include <stdbool.h>

// Structure for a physical key on a specific layer
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
} KeyCoord;

// Maximum number of keys held simultaneously in a single chord
#define MAX_CHORD_KEYS 4 

// Chord structure
typedef struct {
    KeyCoord keys[MAX_CHORD_KEYS]; // Array of keys composing the chord
    uint8_t key_count;             // Actual number of keys in this chord
    void (*action)(void);          // Pointer to the function executed on trigger
} Chord;

// Macro to create a key coordinate: K(Row, Col, Layer)
#define K(r, c, l) {r, c, l}

// Action functions triggered on chord activation
void do_copy(void) { /* Code for Ctrl+C */ }
void do_paste(void) { /* Code for Ctrl+V */ }

// List of all chords in the firmware
const Chord my_chords[] = {
    // Chord 1: Keys (0,1) and (0,2) held on layer 0 -> triggers do_copy
    {  
        .keys = { K(0,1,0), K(0,2,0) }, 
        .key_count = 2, 
        .action = do_copy 
    },
    
    // Chord 2: Three keys on layer 1
    {  
        .keys = { K(1,1,1), K(1,2,1), K(2,0,1) }, 
        .key_count = 3, 
        .action = do_paste 
    },
};

#define CHORDS_COUNT (sizeof(my_chords) / sizeof(my_chords[0]))
```

## Modifiers and Layers (Mod/Layer)

Supports up to 16 layers.

### Momentary activates layer/mod

- FN1/LSHIFT > Momentarily activates a layer or modifier. As soon as you let go of the key, the layer is deactivated.
- `L_0` ... `L_15` > Raw Layer Momentary Switch. Instantly toggles the specific layer bit (e.g. `L_1` for Layer 1) while held. This is a lighter alternative to `MO(layer)` and maps directly to layer indexes.

```c
// --------------------------------------------------------------------------
// PURE LAYERS AND OTHER MODES
// --------------------------------------------------------------------------

// Layer identifiers
enum layers {
    BASE_LAYER = 0, 
    FN1,            
    NUM_LAYER       
};

// ==========================================================================
// 3. KEYMAP WITH UPDATED MO (keymap.c)
// ==========================================================================

const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    
    // LAYER 0: Base layout
    [BASE_LAYER] = LAYOUT(
        // FN1 -> Now simply the name of the layer. The firmware sees it and knows:
        //        "Activate layer 1 while the key is held" (pure Momentary).
        FN1, 
    )
};
```

### Modified key

```c
// --------------------------------------------------------------------------
// MACROS FOR MODIFIED KEYS (MK)
// Allow packaging modifiers and a keycode into a single bind.
// --------------------------------------------------------------------------

// Packaging: High byte — modifier mask, Low byte — keycode
#define MK(mod_mask, kc) (DMK_MK | ((mod_mask) << 8) | (kc))

// Custom binds:
// MK_S_A     -> Pressing sends Shift + A
// MK_SA_A    -> Pressing sends Shift + Alt + A
#define MK_S_A    MK(MOD_LSHIFT, K_A)
#define MK_SA_A   MK(MOD_LSHIFT | MOD_LALT, K_A)
```

### Sticky Keys (ONE SHOT)

See [ZMK Sticky Key Documentation](https://zmk.dev/docs/keymaps/behaviors/sticky-key) for concept details.

- OS > Momentarily activates a layer or modifier until the next key is pressed.

```c
// ==========================================================================
// 1. CONFIGURATION PARAMETERS (Usually located in config.h)
// ==========================================================================
// Timeout (in ms). If no key is pressed within 1.5s after tapping 
// an OS key, the modifier is automatically released.
#define ONESHOT_TIMEOUT 1500

// 1 - Enabled: Modifier is deactivated as soon as the next key is PRESSED.
//             Prevents accidental double capitalization on fast typing.
// 0 - Disabled: Modifier remains active until the next key is RELEASED.
#define ONESHOT_QUICK_RELEASE 1

// [lazy]
// 1 - Enabled: Keyboard does NOT send the modifier to the host immediately on tap.
//             It is sent to the OS only BEFORE the next regular key is pressed.
//             Prevents launching the Windows Start menu on accidental OS_GUI tap.
// 0 - Disabled: Modifier is sent to the host immediately upon tapping the OS key.
#define ONESHOT_LAZY 1

// Note: Callum-style combining is natively active by default. If one OS mod is tapped 
//       while another is active, they stack together (e.g. OS_SHIFT + OS_CTRL).
#define ONESHOT_COMBINE_MODIFIERS 1

// Additional mode: double tapping the OS key locks it permanently active 
// (until the third tap). [Not implemented yet]
#define ONESHOT_TAP_TOGGLE 0

// Note: Natively active by default. Holding the key longer than TAPPING_TERM
//       turns it temporarily into a regular held modifier/layer which deactivates on release.
#define ONESHOT_HOLD_TRIGGER 1
// ==========================================================================
// 2. BINDS AND MACROS FOR KEYMAPS (Utilities)
// ==========================================================================

// System modifier masks
#define MOD_LCTRL  (1 << 0)
#define MOD_LSHIFT (1 << 1)
#define MOD_LALT   (1 << 2)
#define MOD_LGUI   (1 << 3)

// Mask for Hyper Key (equivalent to &sk LG(LS(LA(LCTRL))) in ZMK)
#define MOD_HYPER (MOD_LCTRL | MOD_LSHIFT | MOD_LALT | MOD_LGUI)

// One-Shot (OS) universal macro.
// If the argument is < 32, it behaves as a Layer One-Shot.
// If the argument is >= 32, it behaves as a Modifier One-Shot.
#define OS(val) (DMK_OS | (val))

// Quick shortcuts for OSM (One Shot Modifiers)
#define OS_SHIFT   OS(MOD_LSHIFT)
#define OS_CTRL    OS(MOD_LCTRL)
#define OS_ALT     OS(MOD_LALT)
#define OS_GUI     OS(MOD_LGUI)
#define OS_HYPER   OS(MOD_HYPER) // The Hyper Key shortcut


// ==========================================================================
// 3. KEYMAP USAGE EXAMPLE (keymap.c)
// ==========================================================================


// Two-layer keyboard matrix
const uint32_t keymap[][NUM_ROWS][NUM_COLS] = {
    
    // LAYER 0: Base layout
    [BASE_LAYER] = {
        { K_Q,     K_W,     K_E,    K_R,    K_T,
        
        // OS(NUM_LAYER)   -> Tapped and released: next keypress will be a number,
        //                        after which the keyboard automatically returns to BASE_LAYER.
        // OS_HYPER            -> Tapped and released: activates Ctrl+Shift+Alt+Cmd
        //                        for the next keypress (OS shortcut launcher).
        OS(NUM_LAYER), OS_HYPER, K_S, K_D, K_SPC }
    },

    // LAYER 1: Numbers
    [NUM_LAYER] = {
        { K_1,    K_2,    K_3,    K_TRNS, K_TRNS,
        
        // OS_SHIFT            -> Tapped and released: temporarily enables Shift.
        //                        Useful for typing a symbol on the number layer.
        OS_SHIFT, K_TRNS, K_TRNS, K_TRNS, K_TRNS }
    }
};
```

### Hold-Tap

- HT > Layer-tap/Mod-tap. Momentarily activates a layer or modifier when held, and sends a regular keycode when tapped. Only supports layers 0-15.

In C firmware syntax, the distinction between `Mod-Tap` (`&mt`) and `Layer-Tap` (`&lt`) is merged into a single universal **Hold-Tap (HT)** function. The firmware automatically determines what to do: if the passed index is less than 16, it activates a layer; if it's a modifier mask, it activates a modifier.

```c
// Duration (in ms) to hold the key to recognize it as "Hold" 
// (activation of layer or modifier).
#define TAPPING_TERM_DEFAULT 200

// Analogous to "Flavors" (Priority modes for Hold-Tap):
// ZMK has hold-preferred, tap-preferred, and balanced. In C:

// 1 - hold-preferred: Modifier/layer activates immediately upon hold.
// 0 - tap-preferred: If pressed and released within TAPPING_TERM,
//     sends keycode (kc) even if a neighboring key is pressed.
#define HOLD_TAP_TAP_PREFERRED 1 

// Permissive Hold:
// If enabled (1), pressing and releasing another key during hold
// of our HT key activates "Hold" immediately without waiting 200ms.
#define PERMISSIVE_HOLD 1

// [Universal Hold-Tap Macro]
// Replaces both &mt and &lt from ZMK.
// Argument layer_or_mod: if 0-15 is passed, it is Layer-Tap.
//                        if modifier mask (e.g. MOD_LSHIFT) is passed, it is Mod-Tap.
#define HT(layer_or_mod, kc)    (DMK_HT | ((layer_or_mod) << 8) | (kc))

// Custom hold-tap definition with individual timeout
#define HT_T(layer_or_mod, kc, ms) (DMK_HT | ((layer_or_mod) << 8) | (kc) | ((uint32_t)(ms) << 16))


// ==========================================================================
// 3. KEYMAP USAGE EXAMPLE (keymap.c)
// ==========================================================================

enum layers {
    BASE_LAYER = 0, 
    FN1             // Layer 1 (fits in 0-15 range)
};


const uint32_t keymap[][NUM_ROWS][NUM_COLS] = {
    
    [BASE_LAYER] = {
        // 1. LAYER-TAP EQUIVALENT (&lt 1 A)
        // Held: activates layer 1 (FN1). Tapped: sends regular key 'A'.
        { HT(FN1, K_A),

        // 2. MOD-TAP EQUIVALENT (&mt LSHIFT F)
        // Held: acts as Left Shift. Tapped: sends key 'F'.
        // Firmware detects MOD_LSHIFT and handles it as Mod-Tap, not Layer-Tap.
        HT(MOD_LSHIFT, K_F),

        // 3. CUSTOM HOLD-TAP WITH INDIVIDUAL TIMEOUT
        // For this key (Space / Ctrl), hold timing is increased to 300ms.
        HT_T(MOD_LCTRL, K_SPC, 300) }
    }
};
```

### Toggle

- TG > Layer Toggle, activating it if it's inactive and vice versa.

```c
#define TG(layer)    (DMK_TG | (layer))

// ==========================================================================
// 3. KEYMAP USAGE EXAMPLE (keymap.c)
// ==========================================================================

enum layers {
    BASE_LAYER = 0, 
    FN1             // Layer 1
};


const uint32_t keymap[][NUM_ROWS][NUM_COLS] = {
    
    [BASE_LAYER] = {
        { TG(FN1) }
    }
};
```

### Consumer Media Keys

To use Consumer Page media keys in your keymap (e.g., `K_MUTE`, `K_VOLU`, `K_VOLD`, `K_PLAY`, etc.), you must enable `EXTRAKEY_ENABLE` in your keyboard's config.h. See [config_en.md](config_en.md) and [keycodes_en.md](keycodes_en.md) for details.

---

## Encoders Configuration

If your keyboard features rotary encoders, you can configure their pins, resolution, and actions for each rotation direction. Encoders support keyboard layers: when switching the keyboard layer, the encoder actions automatically switch to the corresponding layer.

If encoder pins are not defined in `config.h`, the encoder support code is automatically excluded from the build.

### Pin Assignment

To configure encoders, specify arrays of pins `A` and `B` in the `config.h` file. The number of pins in both arrays must match:

```c
#define ENCODER_PINS_A  { GPIO9, GPIO10 }
#define ENCODER_PINS_B  { GPIO11, GPIO12 }
```

### Resolution

Resolution specifies the number of quadrature state changes (steps) required to dispatch a single keypress event. By default, the resolution is `4`. You can configure it as follows:

```c
// Set a fixed resolution for all encoders:
#define ENCODER_RESOLUTION 2

// Or individually for each encoder:
#define ENCODER_RESOLUTIONS { 2, 4 }

// Alternatively, specify detent and pulse counts to calculate resolution automatically (detents / pulses):
#define ENCODER_DETENTS { 30, 24 }
#define ENCODER_PULSES  { 15, 24 }
```

### Encoder Keymap

The keymap mapping for encoders is defined as a 3D array inside the `DEFINE_KEYMAP` block in `config.h`. The array must have the same number of layers as the primary keyboard `keymap`:

```c
#ifdef DEFINE_KEYMAP
#define DEF 0
#define FN1 1

const uint32_t encoder_keymap[][NUM_ENCODERS][2] = {
    [DEF] = {
        { K_VOLU, K_VOLD }, // Encoder 1: clockwise (CW), counter-clockwise (CCW)
        { K_LEFT, K_RIGHT } // Encoder 2: CW, CCW
    },
    [FN1] = {
        { K_MNXT, K_MPRV }, // Encoder 1 on FN1 layer
        { K_TRNS, K_TRNS }  // Encoder 2 (transparent fall-through)
    }
};
#endif
```
