🌐 **Language / Язык:** [English](keycodes.md) | [Русский](../ru/keycodes.md)

📖 **Documentation / Документация:** [Build](build.md) • [Config](config.md) • [Keycodes](keycodes.md) • [Keymap](keymap.md) • [Pins](pins.md) • [Vial](vial.md)

---

# DMK Firmware Keycodes

This document lists all the standard HID keycodes, consumer media keys, modifiers, system controls, mousekeys, and custom layer keys defined in the project ([keys.h](../dmk_core/include/keys.h)).

## Alphanumeric (A-Z, 0-9)

| Keycode Symbol              | Short Name / Alias | Description        | HID Code (Hex)    |
| --------------------------- | ------------------ | ------------------ | ----------------- |
| `HID_KEY_A` ... `HID_KEY_Z` | `K_A` ... `K_Z`    | Letters A to Z     | `0x04` ... `0x1D` |
| `HID_KEY_1` ... `HID_KEY_9` | `K_1` ... `K_9`    | Number keys 1 to 9 | `0x1E` ... `0x26` |
| `HID_KEY_0`                 | `K_0`              | Number key 0       | `0x27`            |

## Punctuation & Navigation

| Keycode Symbol             | Short Name / Alias                   | Description                                      | HID Code (Hex) |
| -------------------------- | ------------------------------------ | ------------------------------------------------ | -------------- |
| `HID_KEY_ESCAPE`           | `K_ESC`                              | Escape                                           | `0x29`         |
| `HID_KEY_TAB`              | `K_TAB`                              | Tab                                              | `0x2B`         |
| `HID_KEY_SPACE`            | `K_SPC`                              | Spacebar                                         | `0x2C`         |
| `HID_KEY_ENTER`            | `K_ENT`                              | Enter / Return                                   | `0x28`         |
| `HID_KEY_BACKSPACE`        | `K_BSPC`                             | Backspace                                        | `0x2A`         |
| `HID_KEY_MINUS`            | `K_MIN` / `K_MINS`                   | Minus `-` / `_`                                  | `0x2D`         |
| `HID_KEY_EQUAL`            | `K_EQU` / `K_EQL`                    | Equal `=` / `+`                                  | `0x2E`         |
| `HID_KEY_BRACKET_LEFT`     | `K_BRAL` / `K_LBRACKET` / `K_LBRC`   | Left Bracket `[` / `{`                           | `0x2F`         |
| `HID_KEY_BRACKET_RIGHT`    | `K_BRAR` / `K_RBRACKET` / `K_RBRC`   | Right Bracket `]` / `}`                          | `0x30`         |
| `HID_KEY_BACKSLASH`        | `K_BKSL` / `K_BSLASH` / `K_BSLS`     | Backslash `\` / `\|`                             | `0x31`         |
| `HID_KEY_SEMICOLON`        | `K_SCLN` / `K_SCOLON`                | Semicolon `;` / `:`                              | `0x33`         |
| `HID_KEY_APOSTROPHE`       | `K_APOS` / `K_QUOTE` / `K_QUOT`      | Apostrophe `'` / `"`                             | `0x34`         |
| `HID_KEY_GRAVE`            | `K_GRAV` / `K_GRV`                   | Grave Accent / Backtick `` ` `` / `~`            | `0x35`         |
| `HID_KEY_COMMA`            | `K_COMM`                             | Comma `,` / `<`                                  | `0x36`         |
| `HID_KEY_PERIOD`           | `K_DOT`                              | Period `.` / `>`                                 | `0x37`         |
| `HID_KEY_SLASH`            | `K_SLSH`                             | Forward Slash `/` / `?`                          | `0x38`         |
| `HID_KEY_NON_US_HASH`      | `K_HASH` / `K_NONUS_HASH` / `K_NUHS` | Non-US `#` / `~` (ISO keyboards)                 | `0x32`         |
| `HID_KEY_NON_US_BACKSLASH` | `K_NONUS_BSLASH` / `K_NUBS`          | Non-US `\` / `\|` (ISO keyboards)                | `0x64`         |
| `HID_KEY_CAPS_LOCK`        | `K_CAPS` / `K_CAPSLOCK`              | Caps Lock                                        | `0x39`         |
| -                          | `K_NO`                               | No-op (Legacy macro alignment alias to `K_SCLN`) | `0x33`         |

## Modifiers

| Keycode Symbol           | Short Name / Alias | Description                          | HID Code (Hex) |
| ------------------------ | ------------------ | ------------------------------------ | -------------- |
| `HID_KEY_CONTROL_LEFT`   | `K_LCTL`           | Left Control                         | `0xE0`         |
| `HID_KEY_SHIFT_LEFT`     | `K_LSFT`           | Left Shift                           | `0xE1`         |
| `HID_KEY_ALT_LEFT`       | `K_LALT`           | Left Alt / Option                    | `0xE2`         |
| `HID_KEY_GUI_LEFT`       | `K_LGUI`           | Left GUI (Windows/Command)           | `0xE3`         |
| `HID_KEY_CONTROL_RIGHT`  | `K_RCTL`           | Right Control                        | `0xE4`         |
| `HID_KEY_SHIFT_RIGHT`    | `K_RSFT`           | Right Shift                          | `0xE5`         |
| `HID_KEY_ALT_RIGHT`      | `K_RALT`           | Right Alt / Option                   | `0xE6`         |
| `HID_KEY_GUI_RIGHT`      | `K_RGUI`           | Right GUI (Windows/Command)          | `0xE7`         |
| `HID_KEY_SHIFT_RIGHT`    | `K_SHFT`           | Legacy Shift (Compatibility Alias)   | `0xE5`         |
| `HID_KEY_CONTROL_RIGHT`  | `K_CTRL`           | Legacy Control (Compatibility Alias) | `0xE4`         |
| `HID_KEY_ALT_LEFT`       | `K_ALT`            | Legacy Alt (Compatibility Alias)     | `0xE2`         |
| `HID_KEY_LOCKING_CAPS`   | `K_LOCKING_CAPS`   | Locking Caps Lock                    | `0x82`         |
| `HID_KEY_LOCKING_NUM`    | `K_LOCKING_NUM`    | Locking Num Lock                     | `0x83`         |
| `HID_KEY_LOCKING_SCROLL` | `K_LOCKING_SCROLL` | Locking Scroll Lock                  | `0x84`         |

## Navigation, Editing & F-Keys

| Keycode Symbol                 | Short Name / Alias                   | Description             | HID Code (Hex)    |
| ------------------------------ | ------------------------------------ | ----------------------- | ----------------- |
| `HID_KEY_ARROW_LEFT`           | `K_A_L` / `K_LEFT`                   | Arrow Left              | `0x50`            |
| `HID_KEY_ARROW_RIGHT`          | `K_A_R` / `K_RIGHT` / `K_RGHT`       | Arrow Right             | `0x4F`            |
| `HID_KEY_ARROW_UP`             | `K_A_U` / `K_UP`                     | Arrow Up                | `0x52`            |
| `HID_KEY_ARROW_DOWN`           | `K_A_D` / `K_DOWN`                   | Arrow Down              | `0x51`            |
| `HID_KEY_INSERT`               | `K_INS` / `K_INSERT`                 | Insert                  | `0x49`            |
| `HID_KEY_DELETE`               | `K_DEL` / `K_DELETE`                 | Delete                  | `0x4C`            |
| `HID_KEY_HOME`                 | `K_HOME`                             | Home                    | `0x4A`            |
| `HID_KEY_END`                  | `K_END`                              | End                     | `0x4D`            |
| `HID_KEY_PAGE_UP`              | `K_PGUP` / `K_PAGE_UP`               | Page Up                 | `0x4B`            |
| `HID_KEY_PAGE_DOWN`            | `K_PGDN` / `K_PAGE_DOWN`             | Page Down               | `0x4E`            |
| `HID_KEY_PRINT_SCREEN`         | `K_PSCR` / `K_PSCREEN`               | Print Screen            | `0x46`            |
| `HID_KEY_SCROLL_LOCK`          | `K_SLCK` / `K_SCKLOCK`               | Scroll Lock             | `0x47`            |
| `HID_KEY_PAUSE`                | `K_PAUS` / `K_PAUSE`                 | Pause / Break           | `0x48`            |
| `HID_KEY_APPLICATION`          | `K_MENU` / `K_APPLICATION` / `K_APP` | Menu / Application Key  | `0x65`            |
| `HID_KEY_F1` ... `HID_KEY_F24` | `K_F1` ... `K_F24`                   | Function Keys F1 to F24 | `0x3A` ... `0x73` |
| `HID_KEY_EXECUTE`              | `K_EXECUTE`                          | Execute                 | `0x74`            |
| `HID_KEY_HELP`                 | `K_HELP`                             | Help                    | `0x75`            |
| `HID_KEY_MENU`                 | `K_MENU_KB`                          | Keyboard Menu           | `0x76`            |
| `HID_KEY_SELECT`               | `K_SELECT`                           | Select                  | `0x77`            |
| `HID_KEY_STOP`                 | `K_STOP_KB`                          | Keyboard Stop           | `0x78`            |
| `HID_KEY_AGAIN`                | `K_AGAIN`                            | Again                   | `0x79`            |
| `HID_KEY_UNDO`                 | `K_UNDO`                             | Undo                    | `0x7A`            |
| `HID_KEY_CUT`                  | `K_CUT`                              | Cut                     | `0x7B`            |
| `HID_KEY_COPY`                 | `K_COPY`                             | Copy                    | `0x7C`            |
| `HID_KEY_PASTE`                | `K_PASTE`                            | Paste                   | `0x7D`            |
| `HID_KEY_FIND`                 | `K_FIND`                             | Find                    | `0x7E`            |

## Numpad

| Keycode Symbol               | Short Name / Alias                      | Description           | HID Code (Hex)    |
| ---------------------------- | --------------------------------------- | --------------------- | ----------------- |
| `HID_KEY_NUM_LOCK`           | `K_NLCK` / `K_NUMLOCK`                  | Num Lock              | `0x53`            |
| `HID_KEY_KEYPAD_DIVIDE`      | `K_KPSL` / `K_KP_SLASH` / `K_PSLS`      | Keypad Slash `/`      | `0x54`            |
| `HID_KEY_KEYPAD_MULTIPLY`    | `K_KPAX` / `K_KP_ASTERISK` / `K_PAST`   | Keypad Asterisk `*`   | `0x55`            |
| `HID_KEY_KEYPAD_SUBTRACT`    | `K_KPMI` / `K_KP_MINUS` / `K_PMNS`      | Keypad Minus `-`      | `0x56`            |
| `HID_KEY_KEYPAD_ADD`         | `K_KPPL` / `K_KP_PLUS` / `K_PPLS`       | Keypad Plus `+`       | `0x57`            |
| `HID_KEY_KEYPAD_ENTER`       | `K_KPEN` / `K_KP_ENTER` / `K_PENT`      | Keypad Enter          | `0x58`            |
| `HID_KEY_KEYPAD_DECIMAL`     | `K_KPDT` / `K_KP_DOT` / `K_PDOT`        | Keypad Dot `.`        | `0x63`            |
| `HID_KEY_KEYPAD_COMMA`       | `K_KP_COMMA` / `K_PCMM`                 | Keypad Comma          | `0x85`            |
| `HID_KEY_KEYPAD_EQUAL_AS400` | `K_KP_EQUAL_AS400`                      | Keypad Equal (AS400)  | `0x86`            |
| `HID_KEY_KEYPAD_EQUAL`       | `K_KP_EQUAL` / `K_PEQL`                 | Keypad Equal Sign     | `0x67`            |
| `HID_KEY_KEYPAD_0` ... `9`   | `K_KP0` ... `K_KP9` / `K_P0` ... `K_P9` | Keypad Numbers 0 to 9 | `0x62` ... `0x61` |

## International & Language Keys

| Keycode Symbol            | Short Name / Alias  | Description                             | HID Code (Hex) |
| ------------------------- | ------------------- | --------------------------------------- | -------------- |
| `HID_KEY_INTERNATIONAL_1` | `K_INT1` / `K_RO`   | International 1 (JIS `\`)               | `0x87`         |
| `HID_KEY_INTERNATIONAL_2` | `K_INT2` / `K_KANA` | International 2 (JIS Katakana/Hiragana) | `0x88`         |
| `HID_KEY_INTERNATIONAL_3` | `K_INT3` / `K_JYEN` | International 3 (JIS `¥`)               | `0x89`         |
| `HID_KEY_INTERNATIONAL_4` | `K_INT4` / `K_HENK` | International 4 (JIS Henkan)            | `0x8A`         |
| `HID_KEY_INTERNATIONAL_5` | `K_INT5` / `K_MHEN` | International 5 (JIS Muhenkan)          | `0x8B`         |
| `HID_KEY_INTERNATIONAL_6` | `K_INT6`            | International 6                         | `0x8C`         |
| `HID_KEY_INTERNATIONAL_7` | `K_INT7`            | International 7                         | `0x8D`         |
| `HID_KEY_INTERNATIONAL_8` | `K_INT8`            | International 8                         | `0x8E`         |
| `HID_KEY_INTERNATIONAL_9` | `K_INT9`            | International 9                         | `0x8F`         |
| `HID_KEY_LANG_1`          | `K_LANG1`           | LANG1 (Hangul/Hanja)                    | `0x90`         |
| `HID_KEY_LANG_2`          | `K_LANG2`           | LANG2 (Hanja conversion)                | `0x91`         |
| `HID_KEY_LANG_3`          | `K_LANG3`           | LANG3 (JIS Katakana)                    | `0x92`         |
| `HID_KEY_LANG_4`          | `K_LANG4`           | LANG4 (JIS Hiragana)                    | `0x93`         |
| `HID_KEY_LANG_5`          | `K_LANG5`           | LANG5 (JIS Zenkaku/Hankaku)             | `0x94`         |
| `HID_KEY_LANG_6`          | `K_LANG6`           | LANG6                                   | `0x95`         |
| `HID_KEY_LANG_7`          | `K_LANG7`           | LANG7                                   | `0x96`         |
| `HID_KEY_LANG_8`          | `K_LANG8`           | LANG8                                   | `0x97`         |
| `HID_KEY_LANG_9`          | `K_LANG9`           | LANG9                                   | `0x98`         |

## System Control & Power Keys

> [!WARNING]
> **Placeholder Stub**: System page keycodes (except standard `HID_KEY_POWER`) are defined for future implementation. They are not currently active as the firmware only transmits standard Keyboard Page (0x07) reports.

| Keycode Symbol                | Short Name / Alias          | Description             | HID/Usage Code (Hex)      |
| ----------------------------- | --------------------------- | ----------------------- | ------------------------- |
| `HID_KEY_POWER`               | `K_POWER`                   | Keyboard Power          | `0x66`                    |
| `HID_USAGE_SYSTEM_POWER_DOWN` | `K_SYSTEM_POWER` / `K_PWR`  | System Power Down       | `0x81` (System Page 0x01) |
| `HID_USAGE_SYSTEM_SLEEP`      | `K_SYSTEM_SLEEP` / `K_SLEP` | System Sleep            | `0x82` (System Page 0x01) |
| `HID_USAGE_SYSTEM_WAKE`       | `K_SYSTEM_WAKE` / `K_WAKE`  | System Wake             | `0x83` (System Page 0x01) |
| `HID_KEY_ROLL_OVER`           | `K_ROLL_OVER`               | Keyboard ErrorRollOver  | `0x01`                    |
| `HID_KEY_POST_FAIL`           | `K_POST_FAIL`               | Keyboard POSTFail       | `0x02`                    |
| `HID_KEY_UNDEFINED`           | `K_UNDEFINED`               | Keyboard ErrorUndefined | `0x03`                    |

## Consumer Media Keys

> [!NOTE]
> Consumer page media keycodes are fully supported. They can be enabled by defining `#define EXTRAKEY_ENABLE` in your keyboard's config file (or passing `-DEXTRAKEY=ON` to CMake). This initializes a secondary composite/consumer HID report interface.

| Keycode Symbol / Reference            | Short Name / Alias                | Description               | Usage Code (Hex)             |
| ------------------------------------- | --------------------------------- | ------------------------- | ---------------------------- |
| `HID_USAGE_CONSUMER_MUTE`             | `K_MUTE` / `K_AUDIO_MUTE`         | Mute Audio                | `0xE2` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_VOLUME_UP`        | `K_VOLU` / `K_AUDIO_VOL_UP`       | Volume Up                 | `0xE9` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_VOLUME_DOWN`      | `K_VOLD` / `K_AUDIO_VOL_DOWN`     | Volume Down               | `0xEA` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_PLAY_PAUSE`       | `K_PLAY` / `K_MEDIA_PLAY_PAUSE`   | Play / Pause Media        | `0xCD` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_SCAN_NEXT`        | `K_NEXT` / `K_MEDIA_NEXT_TRACK`   | Next Track                | `0xB5` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_SCAN_PREVIOUS`    | `K_PREV` / `K_MEDIA_PREV_TRACK`   | Previous Track            | `0xB6` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_STOP`             | `K_MSTP` / `K_MEDIA_STOP`         | Stop                      | `0xB7` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_FAST_FORWARD`     | `K_MFFD` / `K_MEDIA_FAST_FORWARD` | Fast Forward              | `0xB3` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_REWIND`           | `K_MRWD` / `K_MEDIA_REWIND`       | Rewind                    | `0xB4` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_EJECT`            | `K_EJCT` / `K_MEDIA_EJECT`        | Stop/Eject                | `0xB8` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_AL_CC_CONFIG`     | `K_MSEL` / `K_MEDIA_SELECT`       | Consumer Control Config   | `0x183` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_EMAIL`         | `K_MAIL`                          | Email Reader              | `0x18A` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_CALCULATOR`    | `K_CALC` / `K_CALCULATOR`         | Calculator                | `0x192` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_LOCAL_BROWSER` | `K_MYCM` / `K_MY_COMPUTER`        | Local Machine Browser     | `0x194` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_SEARCH`        | `K_WSCH` / `K_WWW_SEARCH`         | Web Search                | `0x221` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_HOME`          | `K_WHOM` / `K_WWW_HOME`           | Web Home Page             | `0x223` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_BACK`          | `K_WBAK` / `K_WWW_BACK`           | Web Back                  | `0x224` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_FORWARD`       | `K_WFWD` / `K_WWW_FORWARD`        | Web Forward               | `0x225` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_STOP`          | `K_WSTP` / `K_WWW_STOP`           | Web Stop                  | `0x226` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_REFRESH`       | `K_WREF` / `K_WWW_REFRESH`        | Web Refresh               | `0x227` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_BOOKMARKS`     | `K_WFAV` / `K_WWW_FAVORITES`      | Web Bookmarks             | `0x22A` (Consumer Page 0x0C) |
| `HID_KEY_MUTE`                        | `K__MUTE`                         | Keyboard Page Mute        | `0x7F` (Keyboard Page 0x07)  |
| `HID_KEY_VOLUME_UP`                   | `K__VOLUP`                        | Keyboard Page Volume Up   | `0x80` (Keyboard Page 0x07)  |
| `HID_KEY_VOLUME_DOWN`                 | `K__VOLDOWN`                      | Keyboard Page Volume Down | `0x81` (Keyboard Page 0x07)  |

## RGB Backlight Controls

| Keycode Symbol | Short Name / Alias | Description                              | Custom Code (Hex) |
| -------------- | ------------------ | ---------------------------------------- | ----------------- |
| `K_RGB_TOGG`   | `RGB_TOGG`         | Toggle RGB backlight on or off           | `0xA7`            |
| `K_RGB_NEXT`   | `RGB_NEXT`         | Cycle to next animation mode / theme     | `0xA8`            |
| `K_RGB_PREV`   | `RGB_PREV`         | Cycle to previous animation mode / theme | `0xA9`            |
| `K_RGB_HUI`    | `RGB_HUI`          | Increase Hue (step: 8)                   | `0xAA`            |
| `K_RGB_HUD`    | `RGB_HUD`          | Decrease Hue (step: 8)                   | `0xAB`            |
| `K_RGB_SAI`    | `RGB_SAI`          | Increase Saturation (step: 17)           | `0xAC`            |
| `K_RGB_SAD`    | `RGB_SAD`          | Decrease Saturation (step: 17)           | `0xAD`            |
| `K_RGB_VAI`    | `RGB_VAI`          | Increase Value/Brightness (step: 17)     | `0xAE`            |
| `K_RGB_VAD`    | `RGB_VAD`          | Decrease Value/Brightness (step: 17)     | `0xAF`            |
| `K_RGB_SPI`    | `RGB_SPI`          | Increase Animation Speed (step: 16)      | `0xB0`            |
| `K_RGB_SPD`    | `RGB_SPD`          | Decrease Animation Speed (step: 16)      | `0xB1`            |

## Virtual Mousekeys

> [!WARNING]
> **Placeholder Stub**: Virtual mouse keycodes are defined for future implementation. The firmware currently does not transmit Mouse HID reports, and these keycodes are commented out in `keys.h`.

| Virtual Keycode Reference | Short Name / Alias         | Description                   | Virtual Code (Hex) |
| ------------------------- | -------------------------- | ----------------------------- | ------------------ |
| `HID_KEY_MOUSE_UP`        | `K_MS_UP` / `K_MS_U`       | Mouse Cursor Up               | `0xF0`             |
| `HID_KEY_MOUSE_DOWN`      | `K_MS_DOWN` / `K_MS_D`     | Mouse Cursor Down             | `0xF1`             |
| `HID_KEY_MOUSE_LEFT`      | `K_MS_LEFT` / `K_MS_L`     | Mouse Cursor Left             | `0xF2`             |
| `HID_KEY_MOUSE_RIGHT`     | `K_MS_RIGHT` / `K_MS_R`    | Mouse Cursor Right            | `0xF3`             |
| `HID_KEY_MOUSE_BTN1`      | `K_MS_BTN1` / `K_BTN1`     | Mouse Button 1 (Left Click)   | `0xF4`             |
| `HID_KEY_MOUSE_BTN2`      | `K_MS_BTN2` / `K_BTN2`     | Mouse Button 2 (Right Click)  | `0xF5`             |
| `HID_KEY_MOUSE_BTN3`      | `K_MS_BTN3` / `K_BTN3`     | Mouse Button 3 (Middle Click) | `0xF6`             |
| `HID_KEY_MOUSE_BTN4`      | `K_MS_BTN4` / `K_BTN4`     | Mouse Button 4                | `0xF7`             |
| `HID_KEY_MOUSE_BTN5`      | `K_MS_BTN5` / `K_BTN5`     | Mouse Button 5                | `0xF8`             |
| `HID_KEY_MOUSE_WH_UP`     | `K_MS_WH_UP` / `K_WH_U`    | Mouse Wheel Up                | `0xF9`             |
| `HID_KEY_MOUSE_WH_DOWN`   | `K_MS_WH_DOWN` / `K_WH_D`  | Mouse Wheel Down              | `0xFA`             |
| `HID_KEY_MOUSE_WH_LEFT`   | `K_MS_WH_LEFT` / `K_WH_L`  | Mouse Wheel Left              | `0xFB`             |
| `HID_KEY_MOUSE_WH_RIGHT`  | `K_MS_WH_RIGHT` / `K_WH_R` | Mouse Wheel Right             | `0xFC`             |
| `HID_KEY_MOUSE_ACCEL0`    | `K_MS_ACCEL0` / `K_ACL0`   | Mouse Acceleration 0          | `0xFD`             |
| `HID_KEY_MOUSE_ACCEL1`    | `K_MS_ACCEL1` / `K_ACL1`   | Mouse Acceleration 1          | `0xFE`             |
| `HID_KEY_MOUSE_ACCEL2`    | `K_MS_ACCEL2` / `K_ACL2`   | Mouse Acceleration 2          | `0xFF`             |

## Custom Layer & System Keys

| Keycode Symbol       | Short Name / Alias | Description                                                      | Custom Code (Hex)     |
| -------------------- | ------------------ | ---------------------------------------------------------------- | --------------------- |
| -                    | `K_LYRUP`          | Switch to Next Layer (Layer Up)                                  | `0xA5`                |
| -                    | `K_LYRDWN`         | Switch to Previous Layer (Layer Down)                            | `0xA6`                |
| `HID_KEY_NONE`       | `K_NULL`           | Null / No action                                                 | `0x00`                |
| `K_TRNS`             | `K_TRNS`           | Transparent key (behaves as transparent in layer stack)          | `0x00000000`          |
| `L_0` ... `L_15`     | -                  | Raw Layer Momentary Switch (Layer 0 ... 15)                      | `0x0100` ... `0x010F` |
| `K_FN0` ... `K_FN15` | -                  | Fn Keys (Bound to momentary layer switches `MO(0)` ... `MO(15)`) | Encoded Layer         |
