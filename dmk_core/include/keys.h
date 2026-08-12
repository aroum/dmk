#ifndef KEYS_H
#define KEYS_H

// ==========================================
// Standard HID Keycodes (Usage Page 0x07)
// ==========================================
#define HID_KEY_NONE 0x00
#define HID_KEY_ROLL_OVER 0x01
#define HID_KEY_POST_FAIL 0x02
#define HID_KEY_UNDEFINED 0x03
#define HID_KEY_A 0x04
#define HID_KEY_B 0x05
#define HID_KEY_C 0x06
#define HID_KEY_D 0x07
#define HID_KEY_E 0x08
#define HID_KEY_F 0x09
#define HID_KEY_G 0x0A
#define HID_KEY_H 0x0B
#define HID_KEY_I 0x0C
#define HID_KEY_J 0x0D
#define HID_KEY_K 0x0E
#define HID_KEY_L 0x0F
#define HID_KEY_M 0x10
#define HID_KEY_N 0x11
#define HID_KEY_O 0x12
#define HID_KEY_P 0x13
#define HID_KEY_Q 0x14
#define HID_KEY_R 0x15
#define HID_KEY_S 0x16
#define HID_KEY_T 0x17
#define HID_KEY_U 0x18
#define HID_KEY_V 0x19
#define HID_KEY_W 0x1A
#define HID_KEY_X 0x1B
#define HID_KEY_Y 0x1C
#define HID_KEY_Z 0x1D
#define HID_KEY_1 0x1E
#define HID_KEY_2 0x1F
#define HID_KEY_3 0x20
#define HID_KEY_4 0x21
#define HID_KEY_5 0x22
#define HID_KEY_6 0x23
#define HID_KEY_7 0x24
#define HID_KEY_8 0x25
#define HID_KEY_9 0x26
#define HID_KEY_0 0x27
#define HID_KEY_ENTER 0x28
#define HID_KEY_ESCAPE 0x29
#define HID_KEY_BACKSPACE 0x2A
#define HID_KEY_TAB 0x2B
#define HID_KEY_SPACE 0x2C
#define HID_KEY_MINUS 0x2D
#define HID_KEY_EQUAL 0x2E
#define HID_KEY_BRACKET_LEFT 0x2F
#define HID_KEY_BRACKET_RIGHT 0x30
#define HID_KEY_BACKSLASH 0x31
#define HID_KEY_NON_US_HASH 0x32
#define HID_KEY_SEMICOLON 0x33
#define HID_KEY_APOSTROPHE 0x34
#define HID_KEY_GRAVE 0x35
#define HID_KEY_COMMA 0x36
#define HID_KEY_PERIOD 0x37
#define HID_KEY_SLASH 0x38
#define HID_KEY_CAPS_LOCK 0x39
#define HID_KEY_F1 0x3A
#define HID_KEY_F2 0x3B
#define HID_KEY_F3 0x3C
#define HID_KEY_F4 0x3D
#define HID_KEY_F5 0x3E
#define HID_KEY_F6 0x3F
#define HID_KEY_F7 0x40
#define HID_KEY_F8 0x41
#define HID_KEY_F9 0x42
#define HID_KEY_F10 0x43
#define HID_KEY_F11 0x44
#define HID_KEY_F12 0x45
#define HID_KEY_PRINT_SCREEN 0x46
#define HID_KEY_SCROLL_LOCK 0x47
#define HID_KEY_PAUSE 0x48
#define HID_KEY_INSERT 0x49
#define HID_KEY_HOME 0x4A
#define HID_KEY_PAGE_UP 0x4B
#define HID_KEY_DELETE 0x4C
#define HID_KEY_END 0x4D
#define HID_KEY_PAGE_DOWN 0x4E
#define HID_KEY_ARROW_RIGHT 0x4F
#define HID_KEY_ARROW_LEFT 0x50
#define HID_KEY_ARROW_DOWN 0x51
#define HID_KEY_ARROW_UP 0x52
#define HID_KEY_NUM_LOCK 0x53
#define HID_KEY_KEYPAD_DIVIDE 0x54
#define HID_KEY_KEYPAD_MULTIPLY 0x55
#define HID_KEY_KEYPAD_SUBTRACT 0x56
#define HID_KEY_KEYPAD_ADD 0x57
#define HID_KEY_KEYPAD_ENTER 0x58
#define HID_KEY_KEYPAD_1 0x59
#define HID_KEY_KEYPAD_2 0x5A
#define HID_KEY_KEYPAD_3 0x5B
#define HID_KEY_KEYPAD_4 0x5C
#define HID_KEY_KEYPAD_5 0x5D
#define HID_KEY_KEYPAD_6 0x5E
#define HID_KEY_KEYPAD_7 0x5F
#define HID_KEY_KEYPAD_8 0x60
#define HID_KEY_KEYPAD_9 0x61
#define HID_KEY_KEYPAD_0 0x62
#define HID_KEY_KEYPAD_DECIMAL 0x63
#define HID_KEY_NON_US_BACKSLASH 0x64
#define HID_KEY_APPLICATION 0x65 // Menu key
#define HID_KEY_POWER 0x66
#define HID_KEY_KEYPAD_EQUAL 0x67
#define HID_KEY_F13 0x68
#define HID_KEY_F14 0x69
#define HID_KEY_F15 0x6A
#define HID_KEY_F16 0x6B
#define HID_KEY_F17 0x6C
#define HID_KEY_F18 0x6D
#define HID_KEY_F19 0x6E
#define HID_KEY_F20 0x6F
#define HID_KEY_F21 0x70
#define HID_KEY_F22 0x71
#define HID_KEY_F23 0x72
#define HID_KEY_F24 0x73
#define HID_KEY_EXECUTE 0x74
#define HID_KEY_HELP 0x75
#define HID_KEY_MENU 0x76
#define HID_KEY_SELECT 0x77
#define HID_KEY_STOP 0x78
#define HID_KEY_AGAIN 0x79
#define HID_KEY_UNDO 0x7A
#define HID_KEY_CUT 0x7B
#define HID_KEY_COPY 0x7C
#define HID_KEY_PASTE 0x7D
#define HID_KEY_FIND 0x7E
#define HID_KEY_MUTE 0x7F        // Keyboard page mute (KC__MUTE)
#define HID_KEY_VOLUME_UP 0x80   // Keyboard page volume up (KC__VOLUP)
#define HID_KEY_VOLUME_DOWN 0x81 // Keyboard page volume down (KC__VOLDOWN)
#define HID_KEY_LOCKING_CAPS 0x82
#define HID_KEY_LOCKING_NUM 0x83
#define HID_KEY_LOCKING_SCROLL 0x84
#define HID_KEY_KEYPAD_COMMA 0x85
#define HID_KEY_KEYPAD_EQUAL_AS400 0x86
#define HID_KEY_INTERNATIONAL_1 0x87
#define HID_KEY_INTERNATIONAL_2 0x88
#define HID_KEY_INTERNATIONAL_3 0x89
#define HID_KEY_INTERNATIONAL_4 0x8A
#define HID_KEY_INTERNATIONAL_5 0x8B
#define HID_KEY_INTERNATIONAL_6 0x8C
#define HID_KEY_INTERNATIONAL_7 0x8D
#define HID_KEY_INTERNATIONAL_8 0x8E
#define HID_KEY_INTERNATIONAL_9 0x8F
#define HID_KEY_LANG_1 0x90
#define HID_KEY_LANG_2 0x91
#define HID_KEY_LANG_3 0x92
#define HID_KEY_LANG_4 0x93
#define HID_KEY_LANG_5 0x94
#define HID_KEY_LANG_6 0x95
#define HID_KEY_LANG_7 0x96
#define HID_KEY_LANG_8 0x97
#define HID_KEY_LANG_9 0x98
#define HID_KEY_ALTERNATE_ERASE 0x99
#define HID_KEY_SYSREQ 0x9A
#define HID_KEY_CANCEL 0x9B
#define HID_KEY_CLEAR 0x9C
#define HID_KEY_PRIOR 0x9D
#define HID_KEY_RETURN 0x9E
#define HID_KEY_SEPARATOR 0x9F
#define HID_KEY_OUT 0xA0
#define HID_KEY_OPER 0xA1
#define HID_KEY_CLEAR_AGAIN 0xA2
#define HID_KEY_CRSEL 0xA3
#define HID_KEY_EXSEL 0xA4

// Hardware Modifiers (Usage Page 0x07)
#define HID_KEY_CONTROL_LEFT 0xE0
#define HID_KEY_SHIFT_LEFT 0xE1
#define HID_KEY_ALT_LEFT 0xE2
#define HID_KEY_GUI_LEFT 0xE3 // Windows/Command key
#define HID_KEY_CONTROL_RIGHT 0xE4
#define HID_KEY_SHIFT_RIGHT 0xE5
#define HID_KEY_ALT_RIGHT 0xE6
#define HID_KEY_GUI_RIGHT 0xE7

#define DMK_CONSUMER 0x20000000

// ==========================================
// Consumer / Media Usages (Usage Page 0x0C)
// Requires a separate HID Report ID!
// ==========================================
#define DMK_CONSUMER_PLAY_PAUSE (DMK_CONSUMER | 0xCD)
#define DMK_CONSUMER_SCAN_NEXT (DMK_CONSUMER | 0xB5)
#define DMK_CONSUMER_SCAN_PREVIOUS (DMK_CONSUMER | 0xB6)
#define DMK_CONSUMER_STOP (DMK_CONSUMER | 0xB7)
#define DMK_CONSUMER_MUTE (DMK_CONSUMER | 0xE2)
#define DMK_CONSUMER_VOLUME_UP (DMK_CONSUMER | 0xE9)
#define DMK_CONSUMER_VOLUME_DOWN (DMK_CONSUMER | 0xEA)
#define DMK_CONSUMER_FAST_FORWARD (DMK_CONSUMER | 0xB3)
#define DMK_CONSUMER_REWIND (DMK_CONSUMER | 0xB4)
#define DMK_CONSUMER_EJECT (DMK_CONSUMER | 0xB8)
#define DMK_CONSUMER_AL_CC_CONFIG (DMK_CONSUMER | 0x183)
#define DMK_CONSUMER_AL_EMAIL (DMK_CONSUMER | 0x18A)
#define DMK_CONSUMER_AL_CALCULATOR (DMK_CONSUMER | 0x192)
#define DMK_CONSUMER_AL_LOCAL_BROWSER (DMK_CONSUMER | 0x194)
#define DMK_CONSUMER_AC_SEARCH (DMK_CONSUMER | 0x221)
#define DMK_CONSUMER_AC_HOME (DMK_CONSUMER | 0x223)
#define DMK_CONSUMER_AC_BACK (DMK_CONSUMER | 0x224)
#define DMK_CONSUMER_AC_FORWARD (DMK_CONSUMER | 0x225)
#define DMK_CONSUMER_AC_STOP (DMK_CONSUMER | 0x226)
#define DMK_CONSUMER_AC_REFRESH (DMK_CONSUMER | 0x227)
#define DMK_CONSUMER_AC_BOOKMARKS (DMK_CONSUMER | 0x22A)

// Generic Desktop Page(0x01) - System Control
#define HID_USAGE_SYSTEM_POWER_DOWN 0x81
#define HID_USAGE_SYSTEM_SLEEP 0x82
#define HID_USAGE_SYSTEM_WAKE 0x83

// ==========================================
// Aliases (Shortcuts)
// ==========================================

// Letters
#define K_A HID_KEY_A
#define K_B HID_KEY_B
#define K_C HID_KEY_C
#define K_D HID_KEY_D
#define K_E HID_KEY_E
#define K_F HID_KEY_F
#define K_G HID_KEY_G
#define K_H HID_KEY_H
#define K_I HID_KEY_I
#define K_J HID_KEY_J
#define K_K HID_KEY_K
#define K_L HID_KEY_L
#define K_M HID_KEY_M
#define K_N HID_KEY_N
#define K_O HID_KEY_O
#define K_P HID_KEY_P
#define K_Q HID_KEY_Q
#define K_R HID_KEY_R
#define K_S HID_KEY_S
#define K_T HID_KEY_T
#define K_U HID_KEY_U
#define K_V HID_KEY_V
#define K_W HID_KEY_W
#define K_X HID_KEY_X
#define K_Y HID_KEY_Y
#define K_Z HID_KEY_Z

#ifndef K_NO
#define K_NO K_NULL
#endif

// Numbers
#define K_0 HID_KEY_0
#define K_1 HID_KEY_1
#define K_2 HID_KEY_2
#define K_3 HID_KEY_3
#define K_4 HID_KEY_4
#define K_5 HID_KEY_5
#define K_6 HID_KEY_6
#define K_7 HID_KEY_7
#define K_8 HID_KEY_8
#define K_9 HID_KEY_9

// Arithmetic
#define K_MIN HID_KEY_MINUS
#define K_MINS HID_KEY_MINUS
#define K_EQU HID_KEY_EQUAL
#define K_EQL HID_KEY_EQUAL

// Punctuation & Function
#define K_ESC HID_KEY_ESCAPE
#define K_TAB HID_KEY_TAB
#define K_SPC HID_KEY_SPACE
#define K_ENT HID_KEY_ENTER
#define K_BSPC HID_KEY_BACKSPACE
#define K_COMM HID_KEY_COMMA     // , <
#define K_DOT HID_KEY_PERIOD     // . >
#define K_SLSH HID_KEY_SLASH     // / ?
#define K_SCLN HID_KEY_SEMICOLON // ; :
#define K_SCOLON HID_KEY_SEMICOLON
#define K_BRAL HID_KEY_BRACKET_LEFT // [ {
#define K_LBRACKET HID_KEY_BRACKET_LEFT
#define K_LBRC HID_KEY_BRACKET_LEFT
#define K_BRAR HID_KEY_BRACKET_RIGHT // ] }
#define K_RBRACKET HID_KEY_BRACKET_RIGHT
#define K_RBRC HID_KEY_BRACKET_RIGHT
#define K_BKSL HID_KEY_BACKSLASH // \ |
#define K_BSLASH HID_KEY_BACKSLASH
#define K_BSLS HID_KEY_BACKSLASH
#define K_HASH HID_KEY_NON_US_HASH // # ~ (on ISO keyboards)
#define K_NONUS_HASH HID_KEY_NON_US_HASH
#define K_NUHS HID_KEY_NON_US_HASH
#define K_APOS HID_KEY_APOSTROPHE // ' "
#define K_QUOTE HID_KEY_APOSTROPHE
#define K_QUOT HID_KEY_APOSTROPHE
#define K_GRAV HID_KEY_GRAVE // ` ~
#define K_GRV HID_KEY_GRAVE
#define K_CAPS HID_KEY_CAPS_LOCK
#define K_CAPSLOCK HID_KEY_CAPS_LOCK

// Function Keys
#define K_F1 HID_KEY_F1
#define K_F2 HID_KEY_F2
#define K_F3 HID_KEY_F3
#define K_F4 HID_KEY_F4
#define K_F5 HID_KEY_F5
#define K_F6 HID_KEY_F6
#define K_F7 HID_KEY_F7
#define K_F8 HID_KEY_F8
#define K_F9 HID_KEY_F9
#define K_F10 HID_KEY_F10
#define K_F11 HID_KEY_F11
#define K_F12 HID_KEY_F12
#define K_F13 HID_KEY_F13
#define K_F14 HID_KEY_F14
#define K_F15 HID_KEY_F15
#define K_F16 HID_KEY_F16
#define K_F17 HID_KEY_F17
#define K_F18 HID_KEY_F18
#define K_F19 HID_KEY_F19
#define K_F20 HID_KEY_F20
#define K_F21 HID_KEY_F21
#define K_F22 HID_KEY_F22
#define K_F23 HID_KEY_F23
#define K_F24 HID_KEY_F24

// Navigation / Editing
#define K_INS HID_KEY_INSERT
#define K_INSERT HID_KEY_INSERT
#define K_DEL HID_KEY_DELETE
#define K_DELETE HID_KEY_DELETE
#define K_HOME HID_KEY_HOME
#define K_END HID_KEY_END
#define K_PGUP HID_KEY_PAGE_UP
#define K_PGDN HID_KEY_PAGE_DOWN
#define K_PAGE_UP HID_KEY_PAGE_UP
#define K_PAGE_DOWN HID_KEY_PAGE_DOWN
#define K_PSCR HID_KEY_PRINT_SCREEN
#define K_PSCREEN HID_KEY_PRINT_SCREEN
#define K_SLCK HID_KEY_SCROLL_LOCK
#define K_SCKLOCK HID_KEY_SCROLL_LOCK
#define K_PAUS HID_KEY_PAUSE
#define K_PAUSE HID_KEY_PAUSE
#define K_MENU HID_KEY_APPLICATION

// Arrows
#define K_A_L HID_KEY_ARROW_LEFT
#define K_LEFT HID_KEY_ARROW_LEFT
#define K_A_R HID_KEY_ARROW_RIGHT
#define K_RIGHT HID_KEY_ARROW_RIGHT
#define K_RGHT HID_KEY_ARROW_RIGHT
#define K_A_U HID_KEY_ARROW_UP
#define K_UP HID_KEY_ARROW_UP
#define K_A_D HID_KEY_ARROW_DOWN
#define K_DOWN HID_KEY_ARROW_DOWN

// Numpad
#define K_NLCK HID_KEY_NUM_LOCK
#define K_NUMLOCK HID_KEY_NUM_LOCK
#define K_KPSL HID_KEY_KEYPAD_DIVIDE
#define K_KP_SLASH HID_KEY_KEYPAD_DIVIDE
#define K_PSLS HID_KEY_KEYPAD_DIVIDE
#define K_KPAX HID_KEY_KEYPAD_MULTIPLY
#define K_KP_ASTERISK HID_KEY_KEYPAD_MULTIPLY
#define K_PAST HID_KEY_KEYPAD_MULTIPLY
#define K_KPMI HID_KEY_KEYPAD_SUBTRACT
#define K_KP_MINUS HID_KEY_KEYPAD_SUBTRACT
#define K_PMNS HID_KEY_KEYPAD_SUBTRACT
#define K_KPPL HID_KEY_KEYPAD_ADD
#define K_KP_PLUS HID_KEY_KEYPAD_ADD
#define K_PPLS HID_KEY_KEYPAD_ADD
#define K_KPEN HID_KEY_KEYPAD_ENTER
#define K_KP_ENTER HID_KEY_KEYPAD_ENTER
#define K_PENT HID_KEY_KEYPAD_ENTER
#define K_KPDT HID_KEY_KEYPAD_DECIMAL
#define K_KP_DOT HID_KEY_KEYPAD_DECIMAL
#define K_PDOT HID_KEY_KEYPAD_DECIMAL
#define K_KP0 HID_KEY_KEYPAD_0
#define K_KP1 HID_KEY_KEYPAD_1
#define K_KP2 HID_KEY_KEYPAD_2
#define K_KP3 HID_KEY_KEYPAD_3
#define K_KP4 HID_KEY_KEYPAD_4
#define K_KP5 HID_KEY_KEYPAD_5
#define K_KP6 HID_KEY_KEYPAD_6
#define K_KP7 HID_KEY_KEYPAD_7
#define K_KP8 HID_KEY_KEYPAD_8
#define K_KP9 HID_KEY_KEYPAD_9
#define K_P0 HID_KEY_KEYPAD_0
#define K_P1 HID_KEY_KEYPAD_1
#define K_P2 HID_KEY_KEYPAD_2
#define K_P3 HID_KEY_KEYPAD_3
#define K_P4 HID_KEY_KEYPAD_4
#define K_P5 HID_KEY_KEYPAD_5
#define K_P6 HID_KEY_KEYPAD_6
#define K_P7 HID_KEY_KEYPAD_7
#define K_P8 HID_KEY_KEYPAD_8
#define K_P9 HID_KEY_KEYPAD_9

// System Control / Power / Application / Extra
#define K_NONUS_BSLASH HID_KEY_NON_US_BACKSLASH
#define K_NUBS HID_KEY_NON_US_BACKSLASH
#define K_APPLICATION HID_KEY_APPLICATION
#define K_APP HID_KEY_APPLICATION
#define K_POWER HID_KEY_POWER
#define K_KP_EQUAL HID_KEY_KEYPAD_EQUAL
#define K_PEQL HID_KEY_KEYPAD_EQUAL

// Keyboard 0x74 to 0xA4 Keycodes Aliases
#define K_EXECUTE HID_KEY_EXECUTE
#define K_HELP HID_KEY_HELP
#define K_MENU_KB HID_KEY_MENU
#define K_SELECT HID_KEY_SELECT
#define K_STOP_KB HID_KEY_STOP
#define K_AGAIN HID_KEY_AGAIN
#define K_UNDO HID_KEY_UNDO
#define K_CUT HID_KEY_CUT
#define K_COPY HID_KEY_COPY
#define K_PASTE HID_KEY_PASTE
#define K_FIND HID_KEY_FIND
#define K__MUTE HID_KEY_MUTE
#define K__VOLUP HID_KEY_VOLUME_UP
#define K__VOLDOWN HID_KEY_VOLUME_DOWN
#define K_LOCKING_CAPS HID_KEY_LOCKING_CAPS
#define K_LOCKING_NUM HID_KEY_LOCKING_NUM
#define K_LOCKING_SCROLL HID_KEY_LOCKING_SCROLL
#define K_KP_COMMA HID_KEY_KEYPAD_COMMA
#define K_PCMM HID_KEY_KEYPAD_COMMA
#define K_KP_EQUAL_AS400 HID_KEY_KEYPAD_EQUAL_AS400
#define K_INT1 HID_KEY_INTERNATIONAL_1
#define K_RO HID_KEY_INTERNATIONAL_1
#define K_INT2 HID_KEY_INTERNATIONAL_2
#define K_KANA HID_KEY_INTERNATIONAL_2
#define K_INT3 HID_KEY_INTERNATIONAL_3
#define K_JYEN HID_KEY_INTERNATIONAL_3
#define K_INT4 HID_KEY_INTERNATIONAL_4
#define K_HENK HID_KEY_INTERNATIONAL_4
#define K_INT5 HID_KEY_INTERNATIONAL_5
#define K_MHEN HID_KEY_INTERNATIONAL_5
#define K_INT6 HID_KEY_INTERNATIONAL_6
#define K_INT7 HID_KEY_INTERNATIONAL_7
#define K_INT8 HID_KEY_INTERNATIONAL_8
#define K_INT9 HID_KEY_INTERNATIONAL_9
#define K_LANG1 HID_KEY_LANG_1
#define K_LANG2 HID_KEY_LANG_2
#define K_LANG3 HID_KEY_LANG_3
#define K_LANG4 HID_KEY_LANG_4
#define K_LANG5 HID_KEY_LANG_5
#define K_LANG6 HID_KEY_LANG_6
#define K_LANG7 HID_KEY_LANG_7
#define K_LANG8 HID_KEY_LANG_8
#define K_LANG9 HID_KEY_LANG_9
#define K_ALT_ERASE HID_KEY_ALTERNATE_ERASE
#define K_SYSREQ HID_KEY_SYSREQ
#define K_CANCEL HID_KEY_CANCEL
#define K_CLEAR HID_KEY_CLEAR
#define K_PRIOR HID_KEY_PRIOR
#define K_RETURN HID_KEY_RETURN
#define K_SEPARATOR HID_KEY_SEPARATOR
#define K_OUT HID_KEY_OUT
#define K_OPER HID_KEY_OPER
#define K_CLEAR_AGAIN HID_KEY_CLEAR_AGAIN
#define K_CRSEL HID_KEY_CRSEL
#define K_EXSEL HID_KEY_EXSEL

// System Control Keys Aliases
#define K_SYSTEM_POWER HID_USAGE_SYSTEM_POWER_DOWN
#define K_PWR HID_USAGE_SYSTEM_POWER_DOWN
#define K_SYSTEM_SLEEP HID_USAGE_SYSTEM_SLEEP
#define K_SLEP HID_USAGE_SYSTEM_SLEEP
#define K_SYSTEM_WAKE HID_USAGE_SYSTEM_WAKE
#define K_WAKE HID_USAGE_SYSTEM_WAKE

// Modifiers Keycodes
#define K_LCTL HID_KEY_CONTROL_LEFT
#define K_LSFT HID_KEY_SHIFT_LEFT
#define K_LALT HID_KEY_ALT_LEFT
#define K_LGUI HID_KEY_GUI_LEFT
#define K_RCTL HID_KEY_CONTROL_RIGHT
#define K_RSFT HID_KEY_SHIFT_RIGHT
#define K_RALT HID_KEY_ALT_RIGHT
#define K_RGUI HID_KEY_GUI_RIGHT

// Legacy Modifiers from original file (kept for compatibility)
#define K_SHFT HID_KEY_SHIFT_RIGHT
#define K_CTRL HID_KEY_CONTROL_RIGHT
#define K_ALT HID_KEY_ALT_LEFT

// Modifier Bitmasks (for HID modifier byte)
#define MOD_CTRL 0x01 // Usually 0x01 is Left Ctrl, 0x10 is Right Ctrl
#define MOD_SHFT 0x02 // Usually 0x02 is Left Shift, 0x20 is Right Shift
#define MOD_ALT 0x04  // Usually 0x04 is Left Alt, 0x40 is Right Alt
#define MOD_GUI 0x08  // Left GUI/Win

// Custom / Layer codes
#define K_LYRUP 0xA5
#define K_LYRDWN 0xA6
#define K_RGB_TOGG 0xA7
#define K_RGB_NEXT 0xA8
#define K_RGB_PREV 0xA9
#define K_RGB_HUI 0xAA
#define K_RGB_HUD 0xAB
#define K_RGB_SAI 0xAC
#define K_RGB_SAD 0xAD
#define K_RGB_VAI 0xAE
#define K_RGB_VAD 0xAF
#define K_RGB_SPI 0xB0
#define K_RGB_SPD 0xB1

#define RGB_TOGG K_RGB_TOGG
#define RGB_NEXT K_RGB_NEXT
#define RGB_PREV K_RGB_PREV
#define RGB_HUI K_RGB_HUI
#define RGB_HUD K_RGB_HUD
#define RGB_SAI K_RGB_SAI
#define RGB_SAD K_RGB_SAD
#define RGB_VAI K_RGB_VAI
#define RGB_VAD K_RGB_VAD
#define RGB_SPI K_RGB_SPI
#define RGB_SPD K_RGB_SPD

// Media Keys Aliases (Requires Consumer Report)
#define K_AUDIO_MUTE DMK_CONSUMER_MUTE
#define K_MUTE DMK_CONSUMER_MUTE
#define K_AUDIO_VOL_UP DMK_CONSUMER_VOLUME_UP
#define K_VOLU DMK_CONSUMER_VOLUME_UP
#define K_AUDIO_VOL_DOWN DMK_CONSUMER_VOLUME_DOWN
#define K_VOLD DMK_CONSUMER_VOLUME_DOWN
#define K_MEDIA_NEXT_TRACK DMK_CONSUMER_SCAN_NEXT
#define K_MNXT DMK_CONSUMER_SCAN_NEXT
#define K_MEDIA_PREV_TRACK DMK_CONSUMER_SCAN_PREVIOUS
#define K_MPRV DMK_CONSUMER_SCAN_PREVIOUS
#define K_MEDIA_STOP DMK_CONSUMER_STOP
#define K_MSTP DMK_CONSUMER_STOP
#define K_MEDIA_PLAY_PAUSE DMK_CONSUMER_PLAY_PAUSE
#define K_MPLY DMK_CONSUMER_PLAY_PAUSE
#define K_MEDIA_FAST_FORWARD DMK_CONSUMER_FAST_FORWARD
#define K_MFFD DMK_CONSUMER_FAST_FORWARD
#define K_MEDIA_REWIND DMK_CONSUMER_REWIND
#define K_MRWD DMK_CONSUMER_REWIND
#define K_MEDIA_EJECT DMK_CONSUMER_EJECT
#define K_EJCT DMK_CONSUMER_EJECT
#define K_MEDIA_SELECT DMK_CONSUMER_AL_CC_CONFIG
#define K_MSEL DMK_CONSUMER_AL_CC_CONFIG
#define K_MAIL DMK_CONSUMER_AL_EMAIL
#define K_CALCULATOR DMK_CONSUMER_AL_CALCULATOR
#define K_CALC DMK_CONSUMER_AL_CALCULATOR
#define K_MY_COMPUTER DMK_CONSUMER_AL_LOCAL_BROWSER
#define K_MYCM DMK_CONSUMER_AL_LOCAL_BROWSER
#define K_WWW_SEARCH DMK_CONSUMER_AC_SEARCH
#define K_WSCH DMK_CONSUMER_AC_SEARCH
#define K_WWW_HOME DMK_CONSUMER_AC_HOME
#define K_WHOM DMK_CONSUMER_AC_HOME
#define K_WWW_BACK DMK_CONSUMER_AC_BACK
#define K_WBAK DMK_CONSUMER_AC_BACK
#define K_WWW_FORWARD DMK_CONSUMER_AC_FORWARD
#define K_WFWD DMK_CONSUMER_AC_FORWARD
#define K_WWW_STOP DMK_CONSUMER_AC_STOP
#define K_WSTP DMK_CONSUMER_AC_STOP
#define K_WWW_REFRESH DMK_CONSUMER_AC_REFRESH
#define K_WREF DMK_CONSUMER_AC_REFRESH
#define K_WWW_FAVORITES DMK_CONSUMER_AC_BOOKMARKS
#define K_WFAV DMK_CONSUMER_AC_BOOKMARKS

/* Mousekeys (Virtual) - Stub for future implementation
#define HID_KEY_MOUSE_UP           0xF0
#define HID_KEY_MOUSE_DOWN         0xF1
#define HID_KEY_MOUSE_LEFT         0xF2
#define HID_KEY_MOUSE_RIGHT        0xF3
#define HID_KEY_MOUSE_BTN1         0xF4
#define HID_KEY_MOUSE_BTN2         0xF5
#define HID_KEY_MOUSE_BTN3         0xF6
#define HID_KEY_MOUSE_BTN4         0xF7
#define HID_KEY_MOUSE_BTN5         0xF8
#define HID_KEY_MOUSE_WH_UP        0xF9
#define HID_KEY_MOUSE_WH_DOWN      0xFA
#define HID_KEY_MOUSE_WH_LEFT      0xFB
#define HID_KEY_MOUSE_WH_RIGHT     0xFC
#define HID_KEY_MOUSE_ACCEL0       0xFD
#define HID_KEY_MOUSE_ACCEL1       0xFE
#define HID_KEY_MOUSE_ACCEL2       0xFF

#define K_MS_UP                    HID_KEY_MOUSE_UP
#define K_MS_U                     HID_KEY_MOUSE_UP
#define K_MS_DOWN                  HID_KEY_MOUSE_DOWN
#define K_MS_D                     HID_KEY_MOUSE_DOWN
#define K_MS_LEFT                  HID_KEY_MOUSE_LEFT
#define K_MS_L                     HID_KEY_MOUSE_LEFT
#define K_MS_RIGHT                 HID_KEY_MOUSE_RIGHT
#define K_MS_R                     HID_KEY_MOUSE_RIGHT
#define K_MS_BTN1                  HID_KEY_MOUSE_BTN1
#define K_BTN1                     HID_KEY_MOUSE_BTN1
#define K_MS_BTN2                  HID_KEY_MOUSE_BTN2
#define K_BTN2                     HID_KEY_MOUSE_BTN2
#define K_MS_BTN3                  HID_KEY_MOUSE_BTN3
#define K_BTN3                     HID_KEY_MOUSE_BTN3
#define K_MS_BTN4                  HID_KEY_MOUSE_BTN4
#define K_BTN4                     HID_KEY_MOUSE_BTN4
#define K_MS_BTN5                  HID_KEY_MOUSE_BTN5
#define K_BTN5                     HID_KEY_MOUSE_BTN5
#define K_MS_WH_UP                 HID_KEY_MOUSE_WH_UP
#define K_WH_U                     HID_KEY_MOUSE_WH_UP
#define K_MS_WH_DOWN               HID_KEY_MOUSE_WH_DOWN
#define K_WH_D                     HID_KEY_MOUSE_WH_DOWN
#define K_MS_WH_LEFT               HID_KEY_MOUSE_WH_LEFT
#define K_WH_L                     HID_KEY_MOUSE_WH_LEFT
#define K_MS_WH_RIGHT              HID_KEY_MOUSE_WH_RIGHT
#define K_WH_R                     HID_KEY_MOUSE_WH_RIGHT
#define K_MS_ACCEL0                HID_KEY_MOUSE_ACCEL0
#define K_ACL0                     HID_KEY_MOUSE_ACCEL0
#define K_MS_ACCEL1                HID_KEY_MOUSE_ACCEL1
#define K_ACL1                     HID_KEY_MOUSE_ACCEL1
#define K_MS_ACCEL2                HID_KEY_MOUSE_ACCEL2
#define K_ACL2                     HID_KEY_MOUSE_ACCEL2
*/

// Fn Keys Aliases (Momentary Layer Switches)
#define K_FN0 MO(0)
#define K_FN1 MO(1)
#define K_FN2 MO(2)
#define K_FN3 MO(3)
#define K_FN4 MO(4)
#define K_FN5 MO(5)
#define K_FN6 MO(6)
#define K_FN7 MO(7)
#define K_FN8 MO(8)
#define K_FN9 MO(9)
#define K_FN10 MO(10)
#define K_FN11 MO(11)
#define K_FN12 MO(12)
#define K_FN13 MO(13)
#define K_FN14 MO(14)
#define K_FN15 MO(15)

// Momentary Layer Switches (L_0 .. L_15)
#define L_0 0x0100
#define L_1 0x0101
#define L_2 0x0102
#define L_3 0x0103
#define L_4 0x0104
#define L_5 0x0105
#define L_6 0x0106
#define L_7 0x0107
#define L_8 0x0108
#define L_9 0x0109
#define L_10 0x010A
#define L_11 0x010B
#define L_12 0x010C
#define L_13 0x010D
#define L_14 0x010E
#define L_15 0x010F

// NULL
#define K_NULL HID_KEY_NONE

// Macro structures and helper macros
typedef enum { KEY_DOWN = 1, KEY_UP, DELAY } MacroAction;

typedef struct {
    uint8_t action;
    uint16_t value;
} MacroStep;

// Sentinel value meaning the macro has no layer trigger (regular macro only).
#define MACRO_NO_LAYER 0xFF

typedef struct {
    const MacroStep *steps;
    uint8_t count;
    // Layer index that automatically fires this macro when activated.
    // Set to MACRO_NO_LAYER (0xFF) for a regular key-triggered macro.
    uint8_t layer_trigger;
} Macro;

#define M_DN(key) {KEY_DOWN, key}
#define M_UP(key) {KEY_UP, key}
#define M_D(ms) {DELAY, ms}

// Macro keycode mapping (0xC0 to 0xDF)
#define M(index) (0xC0 + (index))

// Aliases for macro documentation compatibility
#define KEY_ALT K_ALT
#define KEY_TAB K_TAB

// Modifier bitmasks (for MK, HT, OS)
#define MOD_LCTRL (1 << 0)
#define MOD_LSHIFT (1 << 1)
#define MOD_LALT (1 << 2)
#define MOD_LGUI (1 << 3)
#define MOD_RCTRL (1 << 4)
#define MOD_RSHIFT (1 << 5)
#define MOD_RALT (1 << 6)
#define MOD_RGUI (1 << 7)
#define MOD_HYPER (MOD_LCTRL | MOD_LSHIFT | MOD_LALT | MOD_LGUI)

// DMK encoding prefixes (32-bit keycodes)
#define DMK_MK 0x01000000
#define DMK_OS 0x02000000
#define DMK_HT 0x04000000
#define DMK_TG 0x08000000
#define DMK_MO 0x10000000

// Helper Macros for Keymaps
#define MK(mod_mask, kc) (DMK_MK | ((mod_mask) << 8) | (kc))
#define HT(layer_or_mod, kc) (DMK_HT | ((layer_or_mod) << 8) | (kc))
#define HT_T(layer_or_mod, kc, ms) (DMK_HT | ((layer_or_mod) << 8) | (kc) | ((uint32_t)(ms) << 16))
#define TG(layer) (DMK_TG | (layer))
#define MO(layer) (DMK_MO | (layer))

#define OS(val)                                                                                                        \
    ((val) < 32 ? (DMK_OS | 0x20000 | (val))                                                                           \
                : (DMK_OS | 0x10000 | (((val) >= 0xE0 && (val) <= 0xE7 ? (1 << ((val) - 0xE0)) : (val)) << 8)))
#define OS_LAYER(layer) OS(layer)
#define OS_MOD(mod_mask) OS(mod_mask)

// OSM (One Shot Modifiers) short aliases
#define OS_SHIFT OS(MOD_LSHIFT)
#define OS_CTRL OS(MOD_LCTRL)
#define OS_ALT OS(MOD_LALT)
#define OS_GUI OS(MOD_LGUI)
#define OS_HYPER OS(MOD_HYPER)

// Transparent key alias
#define K_TRNS 0x00000000

// Structure for a physical key coordinate on a specific layer
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t layer;
} KeyCoord;

// Maximum number of simultaneous keys in a single chord
#define MAX_CHORD_KEYS 4

// Chord configuration structure
typedef struct {
    KeyCoord keys[MAX_CHORD_KEYS]; // Array of keys that make up the chord
    uint8_t key_count;             // Number of active keys in this chord
    void (*action)(void);          // Action function pointer triggered on chord press
} Chord;

// Helper macro to define chord coordinates: K(row, col, layer)
#define K(r, c, l) {r, c, l}

// ==========================================
// MIDI Keycodes (Standard QMK Range 0x7100 - 0x71FF)
// ==========================================
#define MIDI_ON 0x7100
#define MIDI_OFF 0x7101
#define MIDI_TOGGLE 0x7102
#define MIDI_NOTE_C_0 0x7103
#define MIDI_NOTE_C_SHARP_0 0x7104
#define MIDI_NOTE_D_0 0x7105
#define MIDI_NOTE_D_SHARP_0 0x7106
#define MIDI_NOTE_E_0 0x7107
#define MIDI_NOTE_F_0 0x7108
#define MIDI_NOTE_F_SHARP_0 0x7109
#define MIDI_NOTE_G_0 0x710A
#define MIDI_NOTE_G_SHARP_0 0x710B
#define MIDI_NOTE_A_0 0x710C
#define MIDI_NOTE_A_SHARP_0 0x710D
#define MIDI_NOTE_B_0 0x710E
#define MIDI_NOTE_C_1 0x710F
#define MIDI_NOTE_C_SHARP_1 0x7110
#define MIDI_NOTE_D_1 0x7111
#define MIDI_NOTE_D_SHARP_1 0x7112
#define MIDI_NOTE_E_1 0x7113
#define MIDI_NOTE_F_1 0x7114
#define MIDI_NOTE_F_SHARP_1 0x7115
#define MIDI_NOTE_G_1 0x7116
#define MIDI_NOTE_G_SHARP_1 0x7117
#define MIDI_NOTE_A_1 0x7118
#define MIDI_NOTE_A_SHARP_1 0x7119
#define MIDI_NOTE_B_1 0x711A
#define MIDI_NOTE_C_2 0x711B
#define MIDI_NOTE_C_SHARP_2 0x711C
#define MIDI_NOTE_D_2 0x711D
#define MIDI_NOTE_D_SHARP_2 0x711E
#define MIDI_NOTE_E_2 0x711F
#define MIDI_NOTE_F_2 0x7120
#define MIDI_NOTE_F_SHARP_2 0x7121
#define MIDI_NOTE_G_2 0x7122
#define MIDI_NOTE_G_SHARP_2 0x7123
#define MIDI_NOTE_A_2 0x7124
#define MIDI_NOTE_A_SHARP_2 0x7125
#define MIDI_NOTE_B_2 0x7126
#define MIDI_NOTE_C_3 0x7127
#define MIDI_NOTE_C_SHARP_3 0x7128
#define MIDI_NOTE_D_3 0x7129
#define MIDI_NOTE_D_SHARP_3 0x712A
#define MIDI_NOTE_E_3 0x712B
#define MIDI_NOTE_F_3 0x712C
#define MIDI_NOTE_F_SHARP_3 0x712D
#define MIDI_NOTE_G_3 0x712E
#define MIDI_NOTE_G_SHARP_3 0x712F
#define MIDI_NOTE_A_3 0x7130
#define MIDI_NOTE_A_SHARP_3 0x7131
#define MIDI_NOTE_B_3 0x7132
#define MIDI_NOTE_C_4 0x7133
#define MIDI_NOTE_C_SHARP_4 0x7134
#define MIDI_NOTE_D_4 0x7135
#define MIDI_NOTE_D_SHARP_4 0x7136
#define MIDI_NOTE_E_4 0x7137
#define MIDI_NOTE_F_4 0x7138
#define MIDI_NOTE_F_SHARP_4 0x7139
#define MIDI_NOTE_G_4 0x713A
#define MIDI_NOTE_G_SHARP_4 0x713B
#define MIDI_NOTE_A_4 0x713C
#define MIDI_NOTE_A_SHARP_4 0x713D
#define MIDI_NOTE_B_4 0x713E
#define MIDI_NOTE_C_5 0x713F
#define MIDI_NOTE_C_SHARP_5 0x7140
#define MIDI_NOTE_D_5 0x7141
#define MIDI_NOTE_D_SHARP_5 0x7142
#define MIDI_NOTE_E_5 0x7143
#define MIDI_NOTE_F_5 0x7144
#define MIDI_NOTE_F_SHARP_5 0x7145
#define MIDI_NOTE_G_5 0x7146
#define MIDI_NOTE_G_SHARP_5 0x7147
#define MIDI_NOTE_A_5 0x7148
#define MIDI_NOTE_A_SHARP_5 0x7149
#define MIDI_NOTE_B_5 0x714A
#define MIDI_OCTAVE_N2 0x714B
#define MIDI_OCTAVE_N1 0x714C
#define MIDI_OCTAVE_0 0x714D
#define MIDI_OCTAVE_1 0x714E
#define MIDI_OCTAVE_2 0x714F
#define MIDI_OCTAVE_3 0x7150
#define MIDI_OCTAVE_4 0x7151
#define MIDI_OCTAVE_5 0x7152
#define MIDI_OCTAVE_6 0x7153
#define MIDI_OCTAVE_7 0x7154
#define MIDI_OCTAVE_DOWN 0x7155
#define MIDI_OCTAVE_UP 0x7156
#define MIDI_TRANSPOSE_N6 0x7157
#define MIDI_TRANSPOSE_6 0x7163
#define MIDI_TRANSPOSE_DOWN 0x7164
#define MIDI_TRANSPOSE_UP 0x7165
#define MIDI_VELOCITY_0 0x7166
#define MIDI_VELOCITY_10 0x7170
#define MIDI_VELOCITY_DOWN 0x7171
#define MIDI_VELOCITY_UP 0x7172
#define MIDI_CHANNEL_1 0x7173
#define MIDI_CHANNEL_16 0x7182
#define MIDI_CHANNEL_DOWN 0x7183
#define MIDI_CHANNEL_UP 0x7184
#define MIDI_ALL_NOTES_OFF 0x7185
#define MIDI_SUSTAIN 0x7186
#define MIDI_PORTAMENTO 0x7187
#define MIDI_SOSTENUTO 0x7188
#define MIDI_SOFT 0x7189
#define MIDI_LEGATO 0x718A
#define MIDI_MODULATION 0x718B
#define MIDI_MODULATION_SPEED_DOWN 0x718C
#define MIDI_MODULATION_SPEED_UP 0x718D
#define MIDI_PITCH_BEND_DOWN 0x718E
#define MIDI_PITCH_BEND_UP 0x718F

// Custom MIDI CC range (mapped to QK_KB range for Vial compatibility)
#define MIDI_CC_INC_BASE 0x7E00
#define MIDI_CC_DEC_BASE 0x7E10
#define MIDI_CC_VAL_127_BASE 0x7E20
#define MIDI_CC_TOGGLE_BASE 0x7E30

// Helper Macros for static configuration
#define MIDI_CC(cc, val) (0x21000000 | (((cc) & 0x7F) << 8) | ((val) & 0x7F))
#define MIDI_CC_VAL_127(cc) (MIDI_CC_VAL_127_BASE + ((cc) & 0x0F))
#define MIDI_CC_TOGGLE(cc) (MIDI_CC_TOGGLE_BASE + ((cc) & 0x0F))
#define MIDI_CC_INC(cc) (MIDI_CC_INC_BASE + ((cc) & 0x0F))
#define MIDI_CC_DEC(cc) (MIDI_CC_DEC_BASE + ((cc) & 0x0F))

#define MIDI_CC_X_VAL_127(cc) MIDI_CC_VAL_127(cc)
#define MIDI_CC_X_TOGGLE(cc) MIDI_CC_TOGGLE(cc)
#define MIDI_CC_X_INC(cc) MIDI_CC_INC(cc)
#define MIDI_CC_X_DEC(cc) MIDI_CC_DEC(cc)

#endif