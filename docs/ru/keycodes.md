🌐 **Язык / Language:** [Русский](keycodes.md) | [English](../en/keycodes.md)

📖 **Навигация / Navigation:** [Сборка](build.md) • [Конфигурация](config.md) • [Коды клавиш](keycodes.md) • [Раскладка](keymap.md) • [Пины](pins.md) • [Vial](vial.md) • [Модули](modules.md) • [MIDI](midi.md)

---

# Коды клавиш DMK Firmware

В этом документе перечислены все стандартные скан-коды HID, клавиши мультимедиа (Consumer Page), модификаторы, системные команды, коды виртуальной мыши и коды управления слоями, определенные в проекте ([keys.h](../../dmk_core/include/keys.h)).

## Буквенно-цифровые (A-Z, 0-9)

| Символ кода клавиши         | Короткое имя / Алиас | Описание                   | Код HID (Hex)     |
| --------------------------- | -------------------- | -------------------------- | ----------------- |
| `HID_KEY_A` ... `HID_KEY_Z` | `K_A` ... `K_Z`      | Буквы от A до Z            | `0x04` ... `0x1D` |
| `HID_KEY_1` ... `HID_KEY_9` | `K_1` ... `K_9`      | Цифровые клавиши от 1 до 9 | `0x1E` ... `0x26` |
| `HID_KEY_0`                 | `K_0`                | Цифровая клавиша 0         | `0x27`            |

## Знаки препинания и навигация

| Символ кода клавиши        | Короткое имя / Алиас                 | Описание                                                                         | Код HID (Hex) |
| -------------------------- | ------------------------------------ | -------------------------------------------------------------------------------- | ------------- |
| `HID_KEY_ESCAPE`           | `K_ESC`                              | Escape                                                                           | `0x29`        |
| `HID_KEY_TAB`              | `K_TAB`                              | Табуляция                                                                        | `0x2B`        |
| `HID_KEY_SPACE`            | `K_SPC`                              | Пробел                                                                           | `0x2C`        |
| `HID_KEY_ENTER`            | `K_ENT`                              | Ввод / Enter                                                                     | `0x28`        |
| `HID_KEY_BACKSPACE`        | `K_BSPC`                             | Backspace / Возврат                                                              | `0x2A`        |
| `HID_KEY_MINUS`            | `K_MIN` / `K_MINS`                   | Минус `-` / `_`                                                                  | `0x2D`        |
| `HID_KEY_EQUAL`            | `K_EQU` / `K_EQL`                    | Равно `=` / `+`                                                                  | `0x2E`        |
| `HID_KEY_BRACKET_LEFT`     | `K_BRAL` / `K_LBRACKET` / `K_LBRC`   | Левая квадратная скобка `[` / `{`                                                | `0x2F`        |
| `HID_KEY_BRACKET_RIGHT`    | `K_BRAR` / `K_RBRACKET` / `K_RBRC`   | Правая квадратная скобка `]` / `}`                                               | `0x30`        |
| `HID_KEY_BACKSLASH`        | `K_BKSL` / `K_BSLASH` / `K_BSLS`     | Обратный слэш `\` / `\|`                                                         | `0x31`        |
| `HID_KEY_SEMICOLON`        | `K_SCLN` / `K_SCOLON`                | Точка с запятой `;` / `:`                                                        | `0x33`        |
| `HID_KEY_APOSTROPHE`       | `K_APOS` / `K_QUOTE` / `K_QUOT`      | Апостроф `'` / `"`                                                               | `0x34`        |
| `HID_KEY_GRAVE`            | `K_GRAV` / `K_GRV`                   | Грав / Обратный апостроф `` ` `` / `~`                                           | `0x35`        |
| `HID_KEY_COMMA`            | `K_COMM`                             | Запятая `,` / `<`                                                                | `0x36`        |
| `HID_KEY_PERIOD`           | `K_DOT`                              | Точка `.` / `>`                                                                  | `0x37`        |
| `HID_KEY_SLASH`            | `K_SLSH`                             | Косая черта `/` / `?`                                                            | `0x38`        |
| `HID_KEY_NON_US_HASH`      | `K_HASH` / `K_NONUS_HASH` / `K_NUHS` | Символ `#` / `~` для клавиатур отличных от США (ISO раскладки)                   | `0x32`        |
| `HID_KEY_NON_US_BACKSLASH` | `K_NONUS_BSLASH` / `K_NUBS`          | Символ `\` / `\|` для клавиатур отличных от США (ISO раскладки)                  | `0x64`        |
| `HID_KEY_CAPS_LOCK`        | `K_CAPS` / `K_CAPSLOCK`              | Фиксация регистра Caps Lock                                                      | `0x39`        |
| -                          | `K_NO`                               | Нет операции (устаревший алиас для выравнивания макросов, ссылается на `K_SCLN`) | `0x33`        |

## Модификаторы

| Символ кода клавиши      | Короткое имя / Алиас | Описание                                 | Код HID (Hex) |
| ------------------------ | -------------------- | ---------------------------------------- | ------------- |
| `HID_KEY_CONTROL_LEFT`   | `K_LCTL`             | Левый Control                            | `0xE0`        |
| `HID_KEY_SHIFT_LEFT`     | `K_LSFT`             | Левый Shift                              | `0xE1`        |
| `HID_KEY_ALT_LEFT`       | `K_LALT`             | Левый Alt / Option                       | `0xE2`        |
| `HID_KEY_GUI_LEFT`       | `K_LGUI`             | Левый GUI (Windows / Command)            | `0xE3`        |
| `HID_KEY_CONTROL_RIGHT`  | `K_RCTL`             | Правый Control                           | `0xE4`        |
| `HID_KEY_SHIFT_RIGHT`    | `K_RSFT`             | Правый Shift                             | `0xE5`        |
| `HID_KEY_ALT_RIGHT`      | `K_RALT`             | Правый Alt / Option                      | `0xE6`        |
| `HID_KEY_GUI_RIGHT`      | `K_RGUI`             | Правый GUI (Windows / Command)           | `0xE7`        |
| `HID_KEY_SHIFT_RIGHT`    | `K_SHFT`             | Устаревший Shift (алиас совместимости)   | `0xE5`        |
| `HID_KEY_CONTROL_RIGHT`  | `K_CTRL`             | Устаревший Control (алиас совместимости) | `0xE4`        |
| `HID_KEY_ALT_LEFT`       | `K_ALT`              | Устаревший Alt (алиас совместимости)     | `0xE2`        |
| `HID_KEY_LOCKING_CAPS`   | `K_LOCKING_CAPS`     | Фиксируемый Caps Lock                    | `0x82`        |
| `HID_KEY_LOCKING_NUM`    | `K_LOCKING_NUM`      | Фиксируемый Num Lock                     | `0x83`        |
| `HID_KEY_LOCKING_SCROLL` | `K_LOCKING_SCROLL`   | Фиксируемый Scroll Lock                  | `0x84`        |

## Навигация, редактирование и клавиши F-серии

| Символ кода клавиши            | Короткое имя / Алиас                 | Описание                        | Код HID (Hex)     |
| ------------------------------ | ------------------------------------ | ------------------------------- | ----------------- |
| `HID_KEY_ARROW_LEFT`           | `K_A_L` / `K_LEFT`                   | Стрелка влево                   | `0x50`            |
| `HID_KEY_ARROW_RIGHT`          | `K_A_R` / `K_RIGHT` / `K_RGHT`       | Стрелка вправо                  | `0x4F`            |
| `HID_KEY_ARROW_UP`             | `K_A_U` / `K_UP`                     | Стрелка вверх                   | `0x52`            |
| `HID_KEY_ARROW_DOWN`           | `K_A_D` / `K_DOWN`                   | Стрелка вниз                    | `0x51`            |
| `HID_KEY_INSERT`               | `K_INS` / `K_INSERT`                 | Вставка / Insert                | `0x49`            |
| `HID_KEY_DELETE`               | `K_DEL` / `K_DELETE`                 | Удаление / Delete               | `0x4C`            |
| `HID_KEY_HOME`                 | `K_HOME`                             | В начало / Home                 | `0x4A`            |
| `HID_KEY_END`                  | `K_END`                              | В конец / End                   | `0x4D`            |
| `HID_KEY_PAGE_UP`              | `K_PGUP` / `K_PAGE_UP`               | Страница вверх / Page Up        | `0x4B`            |
| `HID_KEY_PAGE_DOWN`            | `K_PGDN` / `K_PAGE_DOWN`             | Страница вниз / Page Down       | `0x4E`            |
| `HID_KEY_PRINT_SCREEN`         | `K_PSCR` / `K_PSCREEN`               | Печать экрана / Print Screen    | `0x46`            |
| `HID_KEY_SCROLL_LOCK`          | `K_SLCK` / `K_SCKLOCK`               | Scroll Lock                     | `0x47`            |
| `HID_KEY_PAUSE`                | `K_PAUS` / `K_PAUSE`                 | Пауза / Break                   | `0x48`            |
| `HID_KEY_APPLICATION`          | `K_MENU` / `K_APPLICATION` / `K_APP` | Клавиша меню / Приложения       | `0x65`            |
| `HID_KEY_F1` ... `HID_KEY_F24` | `K_F1` ... `K_F24`                   | Функциональные клавиши F1 - F24 | `0x3A` ... `0x73` |
| `HID_KEY_EXECUTE`              | `K_EXECUTE`                          | Выполнить / Execute             | `0x74`            |
| `HID_KEY_HELP`                 | `K_HELP`                             | Справка / Help                  | `0x75`            |
| `HID_KEY_MENU`                 | `K_MENU_KB`                          | Клавиатурное меню               | `0x76`            |
| `HID_KEY_SELECT`               | `K_SELECT`                           | Выбрать / Select                | `0x77`            |
| `HID_KEY_STOP`                 | `K_STOP_KB`                          | Клавиатурный стоп               | `0x78`            |
| `HID_KEY_AGAIN`                | `K_AGAIN`                            | Повторить / Again               | `0x79`            |
| `HID_KEY_UNDO`                 | `K_UNDO`                             | Отменить / Undo                 | `0x7A`            |
| `HID_KEY_CUT`                  | `K_CUT`                              | Вырезать / Cut                  | `0x7B`            |
| `HID_KEY_COPY`                 | `K_COPY`                             | Копировать / Copy               | `0x7C`            |
| `HID_KEY_PASTE`                | `K_PASTE`                            | Вставить / Paste                | `0x7D`            |
| `HID_KEY_FIND`                 | `K_FIND`                             | Найти / Find                    | `0x7E`            |

## Цифровая клавиатура (Numpad)

| Символ кода клавиши          | Короткое имя / Алиас                    | Описание                                      | Код HID (Hex)     |
| ---------------------------- | --------------------------------------- | --------------------------------------------- | ----------------- |
| `HID_KEY_NUM_LOCK`           | `K_NLCK` / `K_NUMLOCK`                  | Num Lock                                      | `0x53`            |
| `HID_KEY_KEYPAD_DIVIDE`      | `K_KPSL` / `K_KP_SLASH` / `K_PSLS`      | Слэш `/` на цифровой клавиатуре               | `0x54`            |
| `HID_KEY_KEYPAD_MULTIPLY`    | `K_KPAX` / `K_KP_ASTERISK` / `K_PAST`   | Звездочка `*` на цифровой клавиатуре          | `0x55`            |
| `HID_KEY_KEYPAD_SUBTRACT`    | `K_KPMI` / `K_KP_MINUS` / `K_PMNS`      | Минус `-` на цифровой клавиатуре              | `0x56`            |
| `HID_KEY_KEYPAD_ADD`         | `K_KPPL` / `K_KP_PLUS` / `K_PPLS`       | Плюс `+` на цифровой клавиатуре               | `0x57`            |
| `HID_KEY_KEYPAD_ENTER`       | `K_KPEN` / `K_KP_ENTER` / `K_PENT`      | Enter на цифровой клавиатуре                  | `0x58`            |
| `HID_KEY_KEYPAD_DECIMAL`     | `K_KPDT` / `K_KP_DOT` / `K_PDOT`        | Точка `.` на цифровой клавиатуре              | `0x63`            |
| `HID_KEY_KEYPAD_COMMA`       | `K_KP_COMMA` / `K_PCMM`                 | Запятая на цифровой клавиатуре                | `0x85`            |
| `HID_KEY_KEYPAD_EQUAL_AS400` | `K_KP_EQUAL_AS400`                      | Знак равенства на цифровой клавиатуре (AS400) | `0x86`            |
| `HID_KEY_KEYPAD_EQUAL`       | `K_KP_EQUAL` / `K_PEQL`                 | Знак равенства на цифровой клавиатуре         | `0x67`            |
| `HID_KEY_KEYPAD_0` ... `9`   | `K_KP0` ... `K_KP9` / `K_P0` ... `K_P9` | Цифры от 0 до 9 на цифровой клавиатуре        | `0x62` ... `0x61` |

## Международные и языковые клавиши

| Символ кода клавиши       | Короткое имя / Алиас | Описание                                  | Код HID (Hex) |
| ------------------------- | -------------------- | ----------------------------------------- | ------------- |
| `HID_KEY_INTERNATIONAL_1` | `K_INT1` / `K_RO`    | Международная 1 (JIS `\`)                 | `0x87`        |
| `HID_KEY_INTERNATIONAL_2` | `K_INT2` / `K_KANA`  | Международная 2 (JIS Катакана/Хирагана)   | `0x88`        |
| `HID_KEY_INTERNATIONAL_3` | `K_INT3` / `K_JYEN`  | Международная 3 (JIS `¥`)                 | `0x89`        |
| `HID_KEY_INTERNATIONAL_4` | `K_INT4` / `K_HENK`  | Международная 4 (JIS Хенкан)              | `0x8A`        |
| `HID_KEY_INTERNATIONAL_5` | `K_INT5` / `K_MHEN`  | Международная 5 (JIS Мухенкан)            | `0x8B`        |
| `HID_KEY_INTERNATIONAL_6` | `K_INT6`             | Международная 6                           | `0x8C`        |
| `HID_KEY_INTERNATIONAL_7` | `K_INT7`             | Международная 7                           | `0x8D`        |
| `HID_KEY_INTERNATIONAL_8` | `K_INT8`             | Международная 8                           | `0x8E`        |
| `HID_KEY_INTERNATIONAL_9` | `K_INT9`             | Международная 9                           | `0x8F`        |
| `HID_KEY_LANG_1`          | `K_LANG1`            | Языковая клавиша 1 (Хангыль/Ханча)        | `0x90`        |
| `HID_KEY_LANG_2`          | `K_LANG2`            | Языковая клавиша 2 (Преобразование Ханча) | `0x91`        |
| `HID_KEY_LANG_3`          | `K_LANG3`            | Языковая клавиша 3 (JIS Катакана)         | `0x92`        |
| `HID_KEY_LANG_4`          | `K_LANG4`            | Языковая клавиша 4 (JIS Хирагана)         | `0x93`        |
| `HID_KEY_LANG_5`          | `K_LANG5`            | Языковая клавиша 5 (JIS Дзенкаку/Ханкаку) | `0x94`        |
| `HID_KEY_LANG_6`          | `K_LANG6`            | Языковая клавиша 6                        | `0x95`        |
| `HID_KEY_LANG_7`          | `K_LANG7`            | Языковая клавиша 7                        | `0x96`        |
| `HID_KEY_LANG_8`          | `K_LANG8`            | Языковая клавиша 8                        | `0x97`        |
| `HID_KEY_LANG_9`          | `K_LANG9`            | Языковая клавиша 9                        | `0x98`        |

## Управление системой и питанием

> [!WARNING]
> **Заглушка-placeholder**: Коды системных клавиш (за исключением стандартной `HID_KEY_POWER`) зарезервированы для будущей реализации. На данный момент они не активны, так как прошивка передает только стандартные отчеты Keyboard Page (0x07).

| Символ кода клавиши           | Короткое имя / Алиас        | Описание                         | Код HID/Usage (Hex)       |
| ----------------------------- | --------------------------- | -------------------------------- | ------------------------- |
| `HID_KEY_POWER`               | `K_POWER`                   | Питание клавиатуры               | `0x66`                    |
| `HID_USAGE_SYSTEM_POWER_DOWN` | `K_SYSTEM_POWER` / `K_PWR`  | Выключение системы               | `0x81` (System Page 0x01) |
| `HID_USAGE_SYSTEM_SLEEP`      | `K_SYSTEM_SLEEP` / `K_SLEP` | Перевод системы в спящий режим   | `0x82` (System Page 0x01) |
| `HID_USAGE_SYSTEM_WAKE`       | `K_SYSTEM_WAKE` / `K_WAKE`  | Пробуждение системы              | `0x83` (System Page 0x01) |
| `HID_KEY_ROLL_OVER`           | `K_ROLL_OVER`               | Ошибка клавиатуры RollOver       | `0x01`                    |
| `HID_KEY_POST_FAIL`           | `K_POST_FAIL`               | Ошибка POST клавиатуры           | `0x02`                    |
| `HID_KEY_UNDEFINED`           | `K_UNDEFINED`               | Неопределенная ошибка клавиатуры | `0x03`                    |

## Мультимедийные клавиши (Consumer Media)

> [!NOTE]
> Мультимедийные клавиши (Consumer page) полностью поддерживаются. Их можно включить, добавив `#define EXTRAKEY_ENABLE` в конфигурационный файл вашей клавиатуры (или передав флаг `-DEXTRAKEY=ON` в CMake). Это активирует комбинированный HID-интерфейс для отправки медиа-отчетов.

| Символ кода клавиши / Ссылка          | Короткое имя / Алиас              | Описание                                 | Код Usage (Hex)              |
| ------------------------------------- | --------------------------------- | ---------------------------------------- | ---------------------------- |
| `HID_USAGE_CONSUMER_MUTE`             | `K_MUTE` / `K_AUDIO_MUTE`         | Выключить звук                           | `0xE2` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_VOLUME_UP`        | `K_VOLU` / `K_AUDIO_VOL_UP`       | Увеличить громкость                      | `0xE9` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_VOLUME_DOWN`      | `K_VOLD` / `K_AUDIO_VOL_DOWN`     | Уменьшить громкость                      | `0xEA` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_PLAY_PAUSE`       | `K_PLAY` / `K_MEDIA_PLAY_PAUSE`   | Воспроизведение / Пауза                  | `0xCD` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_SCAN_NEXT`        | `K_NEXT` / `K_MEDIA_NEXT_TRACK`   | Следующий трек                           | `0xB5` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_SCAN_PREVIOUS`    | `K_PREV` / `K_MEDIA_PREV_TRACK`   | Предыдущий трек                          | `0xB6` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_STOP`             | `K_MSTP` / `K_MEDIA_STOP`         | Стоп                                     | `0xB7` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_FAST_FORWARD`     | `K_MFFD` / `K_MEDIA_FAST_FORWARD` | Перемотка вперед                         | `0xB3` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_REWIND`           | `K_MRWD` / `K_MEDIA_REWIND`       | Перемотка назад                          | `0xB4` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_EJECT`            | `K_EJCT` / `K_MEDIA_EJECT`        | Стоп / Извлечь медиа                     | `0xB8` (Consumer Page 0x0C)  |
| `HID_USAGE_CONSUMER_AL_CC_CONFIG`     | `K_MSEL` / `K_MEDIA_SELECT`       | Настройка панели управления              | `0x183` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_EMAIL`         | `K_MAIL`                          | Клиент электронной почты                 | `0x18A` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_CALCULATOR`    | `K_CALC` / `K_CALCULATOR`         | Calculator                               | `0x192` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AL_LOCAL_BROWSER` | `K_MYCM` / `K_MY_COMPUTER`        | Проводник (Мой компьютер)                | `0x194` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_SEARCH`        | `K_WSCH` / `K_WWW_SEARCH`         | Поиск в Интернете                        | `0x221` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_HOME`          | `K_WHOM` / `K_WWW_HOME`           | Домашняя страница                        | `0x223` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_BACK`          | `K_WBAK` / `K_WWW_BACK`           | Назад в браузере                         | `0x224` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_FORWARD`       | `K_WFWD` / `K_WWW_FORWARD`        | Вперед в браузере                        | `0x225` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_STOP`          | `K_WSTP` / `K_WWW_STOP`           | Остановить загрузку в браузере           | `0x226` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_REFRESH`       | `K_WREF` / `K_WWW_REFRESH`        | Обновить страницу в браузере             | `0x227` (Consumer Page 0x0C) |
| `HID_USAGE_CONSUMER_AC_BOOKMARKS`     | `K_WFAV` / `K_WWW_FAVORITES`      | Закладки браузера                        | `0x22A` (Consumer Page 0x0C) |
| `HID_KEY_MUTE`                        | `K__MUTE`                         | Отключение звука (раздел клавиатуры)     | `0x7F` (Keyboard Page 0x07)  |
| `HID_KEY_VOLUME_UP`                   | `K__VOLUP`                        | Увеличение громкости (раздел клавиатуры) | `0x80` (Keyboard Page 0x07)  |
| `HID_KEY_VOLUME_DOWN`                 | `K__VOLDOWN`                      | Уменьшение громкости (раздел клавиатуры) | `0x81` (Keyboard Page 0x07)  |

## Управление подсветкой RGB

| Символ кода клавиши | Короткое имя / Алиас | Описание                                           | Кастомный код (Hex) |
| ------------------- | -------------------- | -------------------------------------------------- | ------------------- |
| `K_RGB_TOGG`        | `RGB_TOGG`           | Включить / выключить RGB подсветку                 | `0xA7`              |
| `K_RGB_NEXT`        | `RGB_NEXT`           | Следующий режим анимации / кастомная тема          | `0xA8`              |
| `K_RGB_PREV`        | `RGB_PREV`           | Предыдущий режим анимации / кастомная тема         | `0xA9`              |
| `K_RGB_HUI`         | `RGB_HUI`            | Увеличить оттенок (Hue +8)                         | `0xAA`              |
| `K_RGB_HUD`         | `RGB_HUD`            | Уменьшить оттенок (Hue -8)                         | `0xAB`              |
| `K_RGB_SAI`         | `RGB_SAI`            | Увеличить насыщенность (Saturation +17)            | `0xAC`              |
| `K_RGB_SAD`         | `RGB_SAD`            | Уменьшить насыщенность (Saturation -17)            | `0xAD`              |
| `K_RGB_VAI`         | `RGB_VAI`            | Увеличить яркость подсветки (Value/Brightness +17) | `0xAE`              |
| `K_RGB_VAD`         | `RGB_VAD`            | Уменьшить яркость подсветки (Value/Brightness -17) | `0xAF`              |
| `K_RGB_SPI`         | `RGB_SPI`            | Увеличить скорость анимации (Speed +16)            | `0xB0`              |
| `K_RGB_SPD`         | `RGB_SPD`            | Уменьшить скорость анимации (Speed -16)            | `0xB1`              |

## Виртуальные мышиные клавиши (Mousekeys)

> [!NOTE]
> В `dmk` полностью реализована нативная эмуляция USB HID Mouse (курсор, колесо скролла, 5 кнопок мыши и физика плавного разгона с тремя режимами ускорения). Данные клавиши можно использовать в раскладках и переопределять через хуки/модули.

| Виртуальный код клавиши  | Короткое имя / Алиас       | Описание                                           | Виртуальный код (Hex) |
| ------------------------ | -------------------------- | -------------------------------------------------- | --------------------- |
| `HID_KEY_MOUSE_UP`       | `K_MS_UP` / `K_MS_U`       | Курсор мыши вверх                                  | `0xF0`                |
| `HID_KEY_MOUSE_DOWN`     | `K_MS_DOWN` / `K_MS_D`     | Курсор мыши вниз                                   | `0xF1`                |
| `HID_KEY_MOUSE_LEFT`     | `K_MS_LEFT` / `K_MS_L`     | Курсор мыши влево                                  | `0xF2`                |
| `HID_KEY_MOUSE_RIGHT`    | `K_MS_RIGHT` / `K_MS_R`    | Курсор мыши вправо                                 | `0xF3`                |
| `HID_KEY_MOUSE_BTN1`     | `K_MS_BTN1` / `K_BTN1`     | Кнопка мыши 1 (Левый клик)                         | `0xF4`                |
| `HID_KEY_MOUSE_BTN2`     | `K_MS_BTN2` / `K_BTN2`     | Кнопка мыши 2 (Правый клик)                        | `0xF5`                |
| `HID_KEY_MOUSE_BTN3`     | `K_MS_BTN3` / `K_BTN3`     | Кнопка мыши 3 (Средний клик / нажатие на колесико) | `0xF6`                |
| `HID_KEY_MOUSE_BTN4`     | `K_MS_BTN4` / `K_BTN4`     | Кнопка мыши 4                                      | `0xF7`                |
| `HID_KEY_MOUSE_BTN5`     | `K_MS_BTN5` / `K_BTN5`     | Кнопка мыши 5                                      | `0xF8`                |
| `HID_KEY_MOUSE_WH_UP`    | `K_MS_WH_UP` / `K_WH_U`    | Колесико мыши вверх                                | `0xF9`                |
| `HID_KEY_MOUSE_WH_DOWN`  | `K_MS_WH_DOWN` / `K_WH_D`  | Колесико мыши вниз                                 | `0xFA`                |
| `HID_KEY_MOUSE_WH_LEFT`  | `K_MS_WH_LEFT` / `K_WH_L`  | Колесико мыши влево                                | `0xFB`                |
| `HID_KEY_MOUSE_WH_RIGHT` | `K_MS_WH_RIGHT` / `K_WH_R` | Колесико мыши вправо                               | `0xFC`                |
| `HID_KEY_MOUSE_ACCEL0`   | `K_MS_ACCEL0` / `K_ACL0`   | Ускорение мыши 0                                   | `0xFD`                |
| `HID_KEY_MOUSE_ACCEL1`   | `K_MS_ACCEL1` / `K_ACL1`   | Ускорение мыши 1                                   | `0xFE`                |
| `HID_KEY_MOUSE_ACCEL2`   | `K_MS_ACCEL2` / `K_ACL2`   | Ускорение мыши 2                                   | `0xFF`                |

## Кастомные слои и системные клавиши

| Символ кода клавиши  | Короткое имя / Алиас | Описание                                                                     | Кастомный код (Hex)   |
| -------------------- | -------------------- | ---------------------------------------------------------------------------- | --------------------- |
| -                    | `K_LYRUP`            | Переключиться на следующий слой (Слой вверх)                                 | `0xA5`                |
| -                    | `K_LYRDWN`           | Переключиться на предыдущий слой (Слой вниз)                                 | `0xA6`                |
| `K_BOOTLOADER`       | `K_BOOT` / `QK_BOOTLOADER` / `QK_BOOT` / `BOOTLOADER_KEY` | Перезагрузка в режим загрузчика (DFU / UF2 / ROM Bootloader)   | `0xB2`                |
| `HID_KEY_NONE`       | `K_NULL`             | Пустое действие / Ничего не делать                                           | `0x00`                |
| `K_TRNS`             | `K_TRNS`             | Прозрачная клавиша (пропускает нажатие на слой ниже)                         | `0x00000000`          |
| `L_0` ... `L_15`     | -                    | Прямой переключатель слоев (Слой 0 ... 15)                                   | `0x0100` ... `0x010F` |
| `K_FN0` ... `K_FN15` | -                    | Fn клавиши (привязаны к временному переключателю слоев `MO(0)` ... `MO(15)`) | Закодированный слой   |

## MIDI клавиши и макросы

> [!NOTE]
> Подробное руководство по настройке MIDI, интеграции с Vial и физическому порту MIDI Jack приведено в [документации по MIDI](midi.md).

| Символ кода клавиши                                   | Короткое имя / Алиас                           | Описание                                                      | Код / Формат          |
| ----------------------------------------------------- | ---------------------------------------------- | ------------------------------------------------------------- | --------------------- |
| `MIDI_NOTE_C_0` ... `MIDI_NOTE_B_5`                   | -                                              | Воспроизведение ноты (6 октав, 72 ноты)                       | `0x7100` ... `0x7147` |
| `MIDI_OCTAVE_N2` ... `MIDI_OCTAVE_7`                  | -                                              | Прямой выбор базовой октавы (от -2 до 7)                      | `0x7148` ... `0x7151` |
| `MIDI_OCTAVE_DOWN` / `MIDI_OCTAVE_UP`                 | -                                              | Сдвиг октавы вниз / вверх                                     | `0x7155` / `0x7156`   |
| `MIDI_TRANSPOSE_N6` ... `MIDI_TRANSPOSE_6`            | -                                              | Прямой выбор транспозиции (от -6 до +6 полутонов)             | `0x7157` ... `0x7163` |
| `MIDI_TRANSPOSE_DOWN` / `MIDI_TRANSPOSE_UP`           | -                                              | Сдвиг транспозиции на полутон вниз / вверх                    | `0x7164` / `0x7165`   |
| `MIDI_VELOCITY_0` ... `MIDI_VELOCITY_10`              | -                                              | Прямой выбор громкости (Velocity)                             | `0x7166` ... `0x7170` |
| `MIDI_VELOCITY_DOWN` / `MIDI_VELOCITY_UP`             | -                                              | Уменьшить / увеличить громкость (Velocity)                    | `0x7171` / `0x7172`   |
| `MIDI_CHANNEL_1` ... `MIDI_CHANNEL_16`                | -                                              | Прямой выбор MIDI-канала (1–16)                               | `0x7173` ... `0x7182` |
| `MIDI_CHANNEL_DOWN` / `MIDI_CHANNEL_UP`               | -                                              | Сдвиг MIDI-канала вниз / вверх                                | `0x7183` / `0x7184`   |
| `MIDI_ALL_NOTES_OFF`                                  | -                                              | Выключить все звучащие ноты (MIDI Panic)                      | `0x7185`              |
| `MIDI_SUSTAIN` / `MIDI_PORTAMENTO` / `MIDI_SOSTENUTO` | -                                              | MIDI-эффекты: сустейн, портаменто, состенуто                  | `0x7186` ... `0x7188` |
| `MIDI_SOFT` / `MIDI_LEGATO` / `MIDI_MODULATION`       | -                                              | MIDI-эффекты: софт-педаль, легато, модуляция                  | `0x7189` ... `0x718B` |
| `MIDI_PITCH_BEND_DOWN` / `MIDI_PITCH_BEND_UP`         | -                                              | Колесо Pitch Bend вниз / вверх                                | `0x718E` / `0x718F`   |
| `MIDI_CC_INC(cc)`                                     | `MIDI_CC_X_INC(cc)`                            | Увеличить значение CC `cc` (0–15) на 1 (Vial / энкодеры)      | `0x7E00 + cc`         |
| `MIDI_CC_DEC(cc)`                                     | `MIDI_CC_X_DEC(cc)`                            | Уменьшить значение CC `cc` (0–15) на 1 (Vial / энкодеры)      | `0x7E10 + cc`         |
| `MIDI_CC_VAL_127(cc)`                                 | `MIDI_CC_X_VAL_127(cc)`                        | Отправляет CC `cc` 127 при нажатии, 0 при отпускании          | `0x7E20 + cc`         |
| `MIDI_CC_TOGGLE(cc)`                                  | `MIDI_CC_X_TOGGLE(cc)`                         | Переключает CC `cc` между 127 и 0 при нажатии                 | `0x7E30 + cc`         |
| `MIDI_CC(cc, val)`                                    | -                                              | Статическая привязка: `val` при нажатии, 0 при отпускании     | Закодированный CC     |

## Gamepad / DirectInput клавиши

| Символ кода клавиши                       | Короткое имя / Алиас     | Описание                                                      | Код / Формат          |
| ----------------------------------------- | ------------------------ | ------------------------------------------------------------- | --------------------- |
| `GP_BTN1` ... `GP_BTN32`                  | -                        | Кнопки геймпада 1 ... 32                                      | `0x7200` ... `0x721F` |
| `GP_BTN_A` / `GP_BTN_B`                   | `GP_A` / `GP_B`          | Кнопка A (Кнопка 1) / Кнопка B (Кнопка 2)                     | `0x7200` / `0x7201`   |
| `GP_BTN_X` / `GP_BTN_Y`                   | `GP_X` / `GP_Y`          | Кнопка X (Кнопка 3) / Кнопка Y (Кнопка 4)                     | `0x7202` / `0x7203`   |
| `GP_BTN_LB` / `GP_BTN_RB`                 | `GP_LB` / `GP_RB`        | Бамперы Left Bumper (5) / Right Bumper (6)                    | `0x7204` / `0x7205`   |
| `GP_BTN_SELECT` / `GP_BTN_START`          | `GP_SELECT` / `GP_START` | Кнопки Select/Back (7) / Start (8)                            | `0x7206` / `0x7207`   |
| `GP_BTN_L3` / `GP_BTN_R3`                 | `GP_L3` / `GP_R3`        | Нажатие на стик Left Thumb (9) / Right Thumb (10)             | `0x7208` / `0x7209`   |
| `GP_DPAD_UP` / `GP_DPAD_DOWN`             | -                        | D-Pad вверх / вниз                                            | `0x7220` / `0x7221`   |
| `GP_DPAD_LEFT` / `GP_DPAD_RIGHT`           | -                        | D-Pad влево / вправо                                          | `0x7222` / `0x7223`   |
| `GP_LX_LEFT` / `GP_LX_RIGHT`              | -                        | Левый аналоговый стик X: влево (-127) / вправо (+127)         | `0x7224` / `0x7225`   |
| `GP_LY_UP` / `GP_LY_DOWN`                 | -                        | Левый аналоговый стик Y: вверх (-127) / вниз (+127)           | `0x7226` / `0x7227`   |
| `GP_RX_LEFT` / `GP_RX_RIGHT`              | -                        | Правый аналоговый стик X: влево (-127) / вправо (+127)        | `0x7228` / `0x7229`   |
| `GP_RY_UP` / `GP_RY_DOWN`                 | -                        | Правый аналоговый стик Y: вверх (-127) / вниз (+127)          | `0x722A` / `0x722B`   |
| `GP_TRIGGER_L` / `GP_TRIGGER_R`           | `GP_LT` / `GP_RT`        | Аналоговые курки Left Trigger (Rx) / Right Trigger (Ry)       | `0x722C` / `0x722D`   |

