🌐 **Язык / Language:** [Русский](config.md) | [English](../en/config.md)

📖 **Навигация / Navigation:** [Сборка](build.md) • [Конфигурация](config.md) • [Коды клавиш](keycodes.md) • [Раскладка](keymap.md) • [Пины](pins.md) • [Vial](vial.md)

---

# Настройка клавиатуры (`config.h`)

Все аппаратные параметры, слои и макросы клавиатуры описываются в заголовочном файле `config.h` (например, `keyboards/corne/config.h`).

---

## Настройка USB

Добавление имени MCU к имени артефакта:

```c
#define VID      PA0
#define PID      P0_00
#define USB_NAME P0_00
```

## Кроссплатформенная поддержка

Конфигурация может динамически адаптироваться под разные микроконтроллеры с помощью проверок препроцессора (`#if defined(MCU_...)`). Поддерживаются следующие MCU: `MCU_milandr`, `MCU_rp2040`, `MCU_rp2350`, `MCU_nrf52840` и `MCU_baikal`.

```c
#if defined(MCU_milandr)
    #define SERIAL_PIN      PA0
#elif defined(MCU_rp2040) || defined(MCU_rp2350)
    #define SERIAL_PIN      GPIO0
#elif defined(MCU_nrf52840)
    #define SERIAL_PIN      P0_00
#elif defined(MCU_baikal)
    #define SERIAL_PIN      GPIO0
#endif
```

---

## Конфигурация матрицы

Клавиатуры определяют способ сканирования переключателей с помощью макроса `MATRIX_TYPE` и назначения пинов.

### Типы сканирования матрицы

- `MATRIX_TYPE_ROW2COL`: Строки — входы, колонки — выходы.
- `MATRIX_TYPE_COL2ROW`: Колонки — входы, строки — выходы.
- `MATRIX_TYPE_DIRECT`: Каждая клавиша подключена напрямую к своему пину GPIO.

### Матрицы ROW2COL и COL2ROW

Укажите пины строк и колонок с помощью `ROW_PINS` и `COL_PINS`.

```c
#include "pin_defs.h"

#define MATRIX_TYPE_ROW2COL
#define ROW_PINS  { GPIO0, GPIO1, GPIO2, GPIO3 }
#define COL_PINS  { GPIO4, GPIO5, GPIO6, GPIO7, GPIO8 }
```

### Матрица с прямым подключением (Direct Pins)

Если клавиши подключены напрямую к GPIO, определите `DIRECT_PINS`:

```c
#include "pin_defs.h"

#define MATRIX_TYPE_DIRECT
#define DIRECT_PINS  { GPIO0, GPIO1, GPIO2, GPIO3, GPIO4 }
```

### Антидребезг (Debounce)

Задайте задержку антидребезга (в миллисекундах) с помощью макроса `DEBOUNCE` (по умолчанию: `10`):

```c
#define DEBOUNCE 10
```

---

## Физическая раскладка и координаты

Физические раскладки сопоставляют электрические координаты матрицы с логическим положением клавиши.
Это сопоставление задается через координатную карту `LAYOUT` парами `{СТРОКА, КОЛОНКА}`:

```c
#define NUM_ROWS 4
#define NUM_COLS 10

// Сопоставление координат {ROW, COL}
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}, {0, 9}, \
    {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}, \
    {2, 0}, {2, 1}, {2, 2}, {2, 3}, {2, 4}, {2, 5}, {2, 6}, {2, 7}, {2, 8}, {2, 9}, \
                            {3, 3}, {3, 4}, {3, 5}, {3, 6}                  \
}
#define LAYOUT_DEFAULT LAYOUT
```

### Правило координат для прямого подключения

Для прямого подключения (`MATRIX_TYPE_DIRECT`) сетка матрицы отсутствует. Поэтому координата `ROW` всегда должна быть **равна 0**, а `COL` соответствует индексу пина:

```c
// Пример сопоставления для прямого подключения
#define LAYOUT { \
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4} \
}
```

    ### Визуальное позиционирование клавиш в редакторе (`LAYOUT_EDITOR`)

    Если электрическая схема матрицы (`LAYOUT`) отличается от физического расположения кнопок на клавиатуре (например, матрица одномерная, а кнопки расположены в два ряда или со сдвигом), вы можете определить опциональный макрос `LAYOUT_EDITOR`.

    Он задает виртуальные координаты `{ROW, COL}` на экране редактора для каждой клавиши в том же порядке, в котором они перечислены в `LAYOUT`:

    ```c
    // Пример для nizkoteno (10 кнопок): в матрице они в один ряд,
    // но на экране редактора мы хотим расположить их в два ряда по 5 кнопок
    #define LAYOUT_EDITOR { \
        {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, \
        {1, 0}, {1, 1}, {1, 2}, {1, 3}, {1, 4}  \
    }
    ```

Если макрос `LAYOUT_EDITOR` не задан, редактор автоматически расположит кнопки на экране построчно, основываясь на строках из макроса `LAYOUT`.

---

---

## Настройка макросов

Макросы — это структурированные действия, выполняемые при нажатии клавиши (коды клавиш от `0xC0` до `0xDF`). Макросы объявляются внутри блока `DEFINE_KEYMAP`:

```c
#ifdef DEFINE_KEYMAP
// Определение шагов макроса (действия: KEY_DOWN, KEY_UP, DELAY)
const Macro keyboard_macros[] = {
    [0] = { // Индекс макроса 0
        .steps = (MacroStep[]) {
            { KEY_DOWN, K_H },
            { KEY_UP,   K_H },
            { KEY_DOWN, K_E },
            { KEY_UP,   K_E },
            { KEY_DOWN, K_L },
            { KEY_UP,   K_L },
            { KEY_DOWN, K_L },
            { KEY_UP,   K_L },
            { KEY_DOWN, K_O },
            { KEY_UP,   K_O },
            { DELAY,    100 }, // Ожидание 100мс
        },
        .count = 11,
        .layer_trigger = MACRO_NO_LAYER, // Обычный макрос, вызывается клавишей M(0)
    }
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif
```

### Именование макросов через `#define`

Числовые коды `M(0)`, `M(1)` и т.д. в раскладке плохо читаются. Хорошей практикой является объявление именованных псевдонимов через `#define` — это делает раскладку самодокументированной:

```c
#ifdef DEFINE_KEYMAP

// Именованные псевдонимы для макросов
#define MC_NUMDOT M(0) // Alt+46 — знак «.» через нумпад
#define MC_GRTSGN M(1) // Alt+62 — знак «>»
#define MC_SNIP   M(2) // Win+Shift+S — скриншот области

const MacroStep MACRO_NUMDOT[] = {M_DN(K_LALT), M_D(40), M_DN(K_KP4), M_UP(K_KP4),
                                  M_D(40),       M_DN(K_KP6), M_UP(K_KP6), M_UP(K_LALT)};

const MacroStep MACRO_GRTSGN[] = {M_DN(K_LALT), M_D(40), M_DN(K_KP6), M_UP(K_KP6),
                                  M_D(40),       M_DN(K_KP2), M_UP(K_KP2), M_UP(K_LALT)};

const MacroStep MACRO_SNIP[] = {M_DN(K_LGUI), M_DN(K_LSFT), M_DN(K_S),
                                M_UP(K_S),    M_UP(K_LSFT), M_UP(K_LGUI)};

const Macro keyboard_macros[] = {
    [0] = {MACRO_NUMDOT, sizeof(MACRO_NUMDOT) / sizeof(MACRO_NUMDOT[0]), MACRO_NO_LAYER},
    [1] = {MACRO_GRTSGN, sizeof(MACRO_GRTSGN) / sizeof(MACRO_GRTSGN[0]), MACRO_NO_LAYER},
    [2] = {MACRO_SNIP,   sizeof(MACRO_SNIP)   / sizeof(MACRO_SNIP[0]),   MACRO_NO_LAYER},
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);

// Теперь в раскладке используются понятные имена вместо M(0), M(1):
const uint32_t keymap[][NUM_KEYS] = {
    [DEF] = { ..., MC_NUMDOT, MC_GRTSGN, MC_SNIP, ... },
};
#endif
```

---

## Макросы, привязанные к слоям (Layer Trigger Macros)

Поле `layer_trigger` в структуре `Macro` позволяет назначить макрос на активацию конкретного слоя. Когда слой становится активным, все макросы с соответствующим `layer_trigger` выполняются автоматически — **без нажатия клавиши**.

- `MACRO_NO_LAYER` (значение `0xFF`) — обычный макрос, срабатывает только по клавише `M(n)`.
- Любое другое значение — индекс слоя, при активации которого срабатывает макрос.

```c
#ifdef DEFINE_KEYMAP
// Шаги: нажать Win+1
const MacroStep STEPS_SWITCH_LAYER1[] = {
    { KEY_DOWN, K_LGUI },
    { KEY_DOWN, K_1    },
    { KEY_UP,   K_1    },
    { KEY_UP,   K_LGUI },
};

// Шаги: нажать Win+2
const MacroStep STEPS_SWITCH_LAYER2[] = {
    { KEY_DOWN, K_LGUI },
    { KEY_DOWN, K_2    },
    { KEY_UP,   K_2    },
    { KEY_UP,   K_LGUI },
};

const Macro keyboard_macros[] = {
    // Макрос 0: обычный, вызывается клавишей M(0)
    [0] = {
        .steps        = STEPS_SWITCH_LAYER1,
        .count        = 4,
        .layer_trigger = MACRO_NO_LAYER,
    },
    // Макрос 1: срабатывает автоматически при переключении на слой 1 (LWR)
    [1] = {
        .steps        = STEPS_SWITCH_LAYER1,
        .count        = 4,
        .layer_trigger = 1, // LWR
    },
    // Макрос 2: срабатывает автоматически при переключении на слой 2 (RSE)
    [2] = {
        .steps        = STEPS_SWITCH_LAYER2,
        .count        = 4,
        .layer_trigger = 2, // RSE
    },
};

const uint8_t keyboard_macros_count = sizeof(keyboard_macros) / sizeof(keyboard_macros[0]);
#endif
```

> [!NOTE]
> Один и тот же слой может запускать **несколько** макросов одновременно — просто присвойте нескольким записям одинаковый `layer_trigger`. Макросы выполняются в порядке их объявления в массиве.

> [!NOTE]
> Layer trigger макросы **не поддерживаются** в режиме Vial (динамическое редактирование раскладки). Они работают только в статических конфигурациях `config.h`.

---

## Настройки сплит-клавиатур

Для раздельных клавиатур (таких как Corne, Jianovka или Nizkoteno Split) связь между половинками осуществляется через унифицированную архитектуру драйверов последовательного протокола.

### Единый формат назначения режимов и пинов

```c
#define SPLIT_CONNECTION_TYPE SOFT         // SOFT | HW_HALF_DUPLEX | HW_FULL_DUPLEX
#define SPLIT_TX_PIN          GPIO0        // Один провод данных (SOFT/HW_HALF_DUPLEX) или пин TX (HW_FULL_DUPLEX)
#define SPLIT_RX_PIN          GPIO1        // Пин RX (только для HW_FULL_DUPLEX)
```

| Платформа                | Режим (`SPLIT_CONNECTION_TYPE`) | Линия связи                   | Настройки пинов                                              | Примечания                                                   |
| :----------------------- | :------------------------------ | :---------------------------- | :----------------------------------------------------------- | :----------------------------------------------------------- |
| **RP2040 / RP2350**      | `HW_HALF_DUPLEX`                | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN GPIO0`                                 | Аппаратный PIO (`split.pio`). Не блокирует CPU, любой GPIO.  |
| **RP2040 / RP2350**      | `SOFT`                          | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN GPIO0`                                 | Программный Bit-Bang на любом GPIO.                          |
| **Milandr (MDR32F9Q2I)** | `HW_FULL_DUPLEX`                | Двухпроводная (2 Pins: TX+RX) | `#define SPLIT_TX_PIN PF1`<br>`#define SPLIT_RX_PIN PF0`     | Аппаратный UART (`MDR_UART2` на `MDR_PORTF`).                |
| **Milandr (MDR32F9Q2I)** | `SOFT`                          | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN PF0`                                   | Программный Bit-Bang на любом GPIO.                          |
| **nRF52840**             | `HW_HALF_DUPLEX`                | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN P0_00`                                 | Аппаратный UARTE в Open-Drain (`S0D1`) режиме на 1 пине.     |
| **nRF52840**             | `HW_FULL_DUPLEX`                | Двухпроводная (2 Pins: TX+RX) | `#define SPLIT_TX_PIN P0_00`<br>`#define SPLIT_RX_PIN P0_01` | Аппаратный UARTE на отдельных TX и RX.                       |
| **nRF52840**             | `SOFT`                          | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN P0_00`                                 | Программный Bit-Bang на любом GPIO.                          |
| **Baikal (BE-T1000)**    | `HW_HALF_DUPLEX`                | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN PC6`                                   | Аппаратный RS-485 полудуплекс (UART6 `PC6` или UART7 `PC8`). |
| **Baikal (BE-T1000)**    | `HW_FULL_DUPLEX`                | Двухпроводная (2 Pins: TX+RX) | `#define SPLIT_TX_PIN PA2`<br>`#define SPLIT_RX_PIN PA3`     | Аппаратный UART (UART0..UART7).                              |
| **Baikal (BE-T1000)**    | `SOFT`                          | Однопроводная (1 Pin)         | `#define SPLIT_TX_PIN PA2`                                   | Программный Bit-Bang на любом GPIO.                          |

> [!NOTE]
> **Обратная совместимость**: Использование старого макроса `#define SERIAL_PIN GPIO0` автоматически транслируется в `SPLIT_TX_PIN` и `SPLIT_RX_PIN`. Если `SPLIT_CONNECTION_TYPE` не указан, прошивка автоматически выбирает наиболее подходящий режим для текущей платформы.

---

### Конфигурация геометрии и определения стороны

```c
#define MASTER_SIDE LEFT      // Варианты: LEFT, RIGHT, или AUTO
#define NUM_ROWS_SPLIT 4      // Строк на каждой половине
#define NUM_COLS_SPLIT 5      // Колонок на каждой половине
#define SPLIT_COL_OFFSET 5    // Логическое смещение колонок для правой половины
```

- **Динамическое определение роли**: При использовании `AUTO` (или при сборке без указания стороны `-d`), половинка, к которой физически подключен USB кабель к ПК, автоматически становится главной (`MASTER_SIDE`) на основе обнаружения USB-подключения (`tud_mounted()`).

---

### Примеры конфигурации пинов по платформам

#### 1. RP2040 / RP2350 (PIO Hardware Single-Wire)

```c
#define SPLIT_CONNECTION_TYPE HW_HALF_DUPLEX
#define SPLIT_TX_PIN          GPIO0        // Любой свободный GPIO пин (GPIO0..GPIO29)
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 бод, 2 = 19200 бод (по умолчанию)
```

#### 2. Milandr Hardware UART (Full-Duplex / 2 провода TX+RX)

```c
#define SPLIT_CONNECTION_TYPE HW_FULL_DUPLEX
#define SPLIT_TX_PIN          PF1          // Пин TX (автоматически выбирает MDR_PORTF и MDR_UART2)
#define SPLIT_RX_PIN          PF0          // Пин RX
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 бод, 2 = 19200 бод
```

#### 3. Универсальный Software Bit-Bang (Для всех МК: Миландр, nRF52840, Baikal, RP2040)

```c
#define SPLIT_CONNECTION_TYPE SOFT
#define SPLIT_TX_PIN          PF0          // Один пин для 1-проводного кабеля TRRS
#define SPLIT_HARDWARE_OPTION 2            // 1 = 115200 бод, 2 = 19200 бод
```

---

### Справочник пинов Hardware UART по платформам

#### 1. nRF52840 (Nordic)

- **Гибкая матрица пинов (Pin Crossbar):** Аппаратный модуль UARTE (`NRF_UARTE0`) на nRF52840 может быть назначен на **ЛЮБОЙ** свободный GPIO пин (`P0_00`..`P0_31`, `P1_00`..`P1_15`).
- **Однопроводной полудуплекс (`HW_HALF_DUPLEX`):** `SPLIT_TX_PIN` назначает оба сигнала (TXD и RXD) на один пин в режиме Open-Drain (`S0D1`) с внутренним pull-up.
  - _Рекомендуемые пины:_ `P0_06`, `P0_08`, `P0_17`, `P0_20`, `P1_09`.
- **Двухпроводной полный дуплекс (`HW_FULL_DUPLEX`):**
  - _Пример:_ `#define SPLIT_TX_PIN P0_06`, `#define SPLIT_RX_PIN P0_08`.
- _Ограничения:_ Не рекомендуется использовать пины, занятые под NFC (`P0.09`, `P0.10`) или часовой кварц 32кГц (`P0.00`, `P0.01`), если данные модули активны.

#### 2. RP2040 / RP2350 (Raspberry Pi)

- **Режим PIO Half-Duplex (`HW_HALF_DUPLEX`):** Поддерживается на **ЛЮБОМ** GPIO пине (`GPIO0`..`GPIO29`).
- **Аппаратная периферия UART (`HW_FULL_DUPLEX`):**
  - **RP2040 (Все пары пинов по функции F2):**
    - **UART0 TX:** `GPIO0`, `GPIO12`, `GPIO16`, `GPIO28`
    - **UART0 RX:** `GPIO1`, `GPIO13`, `GPIO17`, `GPIO29`
    - **UART1 TX:** `GPIO4`, `GPIO8`, `GPIO20`, `GPIO24`
    - **UART1 RX:** `GPIO5`, `GPIO9`, `GPIO21`, `GPIO25`
  - **RP2350 (Дополнительные пары по функции F11):**
    - **UART0 TX (F11):** `GPIO2`, `GPIO14`, `GPIO18` (а также `GPIO30`, `GPIO34`, `GPIO46` для QFN-80)
    - **UART0 RX (F11):** `GPIO3`, `GPIO15`, `GPIO19` (а также `GPIO31`, `GPIO35`, `GPIO47` для QFN-80)
    - **UART1 TX (F11):** `GPIO6`, `GPIO10`, `GPIO22`, `GPIO26` (а также `GPIO38`, `GPIO42` для QFN-80)
    - **UART1 RX (F11):** `GPIO7`, `GPIO11`, `GPIO23`, `GPIO27` (а также `GPIO39`, `GPIO43` для QFN-80)

#### 3. Milandr MDR32F9Q2I (К1986ВЕ92FI)

- **Формат пинов:** Пины передаются в едином стандартном формате матрицы — `PF0`, `PF1`, `PA6`, `PA7` и т.д.
- **Автоматическое определение:** Прошивка сама определяет соответствующий GPIO порт (`MDR_PORTF`, `MDR_PORTA`) и модуль периферии (`MDR_UART2` для `PF0`/`PF1`, `MDR_UART1` для `PA6`/`PA7`).
  - **`MDR_UART2` (Порт F):** `SPLIT_TX_PIN PF1`, `SPLIT_RX_PIN PF0`
  - **`MDR_UART1` (Порт A):** `SPLIT_TX_PIN PA7`, `SPLIT_RX_PIN PA6`

#### 4. Байкал (BE-T1000)

- **Режим Software Bit-Bang (`SOFT`):** Любой свободный GPIO пин (`PA2`, `PA3`, `PB2`, `PC6` и т.д.).
- **Однопроводной полудуплекс (`HW_HALF_DUPLEX`):** Аппаратный RS-485 режим с сигналами управления `DE`/`RE`. Поддерживается **только на `UART6` (`PC6`) и `UART7` (`PC8`)**.
- **Аппаратный полный дуплекс (`HW_FULL_DUPLEX`):**
  - **`UART0` (Порт A):**
    - **TX:** `PA2`, `PA6`, `PA8`
    - **RX:** `PA3`, `PA7`, `PA9`
  - **`UART1` (Порт A):**
    - **TX:** `PA4`, `PA10`, `PA14`
    - **RX:** `PA5`, `PA11`, `PA15`
  - **`UART2` (Порт A):**
    - **TX:** `PA12` | **RX:** `PA13`
  - **`UART3` (Порт B):**
    - **TX:** `PB2`, `PB6`, `PB8`
    - **RX:** `PB3`, `PB7`, `PB9`
  - **`UART4` (Порт B):**
    - **TX:** `PB4`, `PB10`, `PB14`
    - **RX:** `PB5`, `PB11`, `PB15`
  - **`UART5` (Порт B):**
    - **TX:** `PB12` | **RX:** `PB13`
  - **`UART6` (Порт C):**
    - **TX:** `PC6` | **RX:** `PC7`
  - **`UART7` (Порт C):**
    - **TX:** `PC8` | **RX:** `PC9`

---

## Конфигурация светодиодов (LEDs)

`dmk` поддерживает сопоставление светодиодов состояния и диагностики.

```c
#define LED_PINS           { PA4, PA4, PA5, PA6, PA7, PA8 }
#define LED_DEBUG           0 // Индекс в LED_PINS для светодиода диагностики
#define LED_HID_NUM_LOCK    1 // Индекс в LED_PINS для Num Lock LED
#define LED_HID_CAPS_LOCK   2 // Индекс в LED_PINS для Caps Lock LED
#define LED_HID_SCROLL_LOCK 3 // Индекс в LED_PINS for Scroll Lock LED
#define LED_HID_COMPOSE     4 // Индекс в LED_PINS для Compose LED
#define LED_HID_KANA        5 // Индекс в LED_PINS для Kana LED
```

- **Диагностический маяк (`LED_DEBUG`)**:
  - Медленно мигает (0.5 Гц), когда USB подключен, но **не** смонтирован хостом.
  - Быстро мигает (2 Гц), когда USB **успешно смонтирован** ПК.
  - Кратковременно вспыхивает (на 25 мс) при отправке события нажатия.
- **Индикаторы хоста (`LED_HID_*`)**: Сопоставляют состояния блокировок ОС (Caps Lock и др.) напрямую со светодиодами на плате.

---

## Подсветка RGB WS2812

Светодиодные ленты WS2812 настраиваются через указание их количества, пина подключения, карты размещения и кастомных тем:

```c
#define RGB_NUM     10
#define RGB_PIN     GPIO25
#define RGB_MAP     { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 }

#define RGB_THEME_DEFAULT { 0xFF0000, 0x00FF00, 0x0000FF, 0xFFFFFF }
#define RGB_THEME_GAMING  { 0xFF4500, 0xFF8C00, 0xFFD700, 0xADFF2F }
#define RGB_THEMES        { RGB_THEME_DEFAULT, RGB_THEME_GAMING }
#define RGB_LIMIT_VAL     128 // Опционально: ограничение максимальной яркости (0-255) для уменьшения тока потребления
```

- `RGB_LIMIT_VAL`: Опциональный лимит максимальной яркости (значения от `0` до `255`, по умолчанию `255`). Позволяет ограничить максимальный ток потребления светодиодов, аппаратно срезая пиковую яркость в движке подсветки.

### Поддерживаемые эффекты и анимации

`dmk` поддерживает световые эффекты в стиле QMK:

- **Режим 1 (Static Light)**: Статический (солидный) цвет.
- **Режим 2 (Breathing)**: Плавное затухание и разгорание цвета (эффект дыхания).
- **Режим 6 (Rainbow Mood)**: Синхронное переливание всех светодиодов по цветовому кругу.
- **Режим 9 (Rainbow Swirl)**: Эффект бегущей радуги (градиент, перемещающийся по ленте).
- **Режим 15 (Snake)**: Бегущая «змейка» из светящихся светодиодов с затухающим хвостом.
- **Режим 18 (Knight Rider)**: Эффект сканера («глаз Сайлона»), бегающий туда-обратно.
- **Режим 21 (Christmas)**: Новогодний режим с мигающими попеременно красными и зелеными огнями.
- **Режим 22 (Static Gradient)**: Статический радужный градиент, распределенный по всей длине.
- **Режим 100 (Theme Mode)**: Кастомные статические раскладки, заданные в `RGB_THEMES` (доступно только если определен макрос `RGB_THEMES`).

### Управление

Все параметры (режим, оттенок, насыщенность, яркость и скорость анимации) полностью настраиваются в реальном времени через графический интерфейс **Vial GUI** (по стандартному протоколу управления QMK RGBLIGHT).

Физические клавиши на клавиатуре могут быть привязаны к:

- `K_RGB_TOGG`: Включить/выключить подсветку.
- `K_RGB_NEXT`: Переключить на следующий режим анимации (или следующую кастомную тему, если выбран режим тем).
- `K_RGB_PREV`: Переключить на предыдущий режим анимации (или предыдущую кастомную тему).

---

## Мультимедийные клавиши (Consumer Control)

Чтобы включить поддержку мультимедийных клавиш (клавиши Consumer Page, такие как воспроизведение, пауза, регулировка громкости, запуск браузера, почты, калькулятора, поиск в интернете и т. д.), добавьте следующий макрос:

```c
#define EXTRAKEY_ENABLE
```

Это активирует дополнительный составной HID-интерфейс на устройстве, позволяющий передавать 16-битные коды команд Consumer Control.

---

## Энкодеры 

DMK поддерживает аппаратный опрос квадратурных поворотных энкодеров (с 2-битным кодом Грея), поддержку слоёв и настройку в реальном времени через **Vial GUI**.

### 1. Настройка пинов и разрешения

В `config.h` укажите массивы сигнальных выводов каналов A и B:

```c
// Пины каналов A и B для каждого подключенного энкодера
#define ENCODER_PINS_A { GPIO14 }
#define ENCODER_PINS_B { GPIO15 }

// Опционально: количество квадратурных импульсов на один физический щелчок (по умолчанию 4)
#define ENCODER_RESOLUTIONS { 4 }
```

Если энкодеров несколько:
```c
#define ENCODER_PINS_A { GPIO14, GPIO16 }
#define ENCODER_PINS_B { GPIO15, GPIO17 }
#define ENCODER_RESOLUTIONS { 4, 2 }
```

### 2. Назначение действий по слоям (`ENCODER_KEYMAP`)

Действия на вращение по часовой стрелке (CW) и против часовой стрелки (CCW) задаются для каждого слоя в массиве `ENCODER_KEYMAP`:

```c
#define ENCODER_KEYMAP { \
    [0] = { { K_VOLU, K_VOLD } }, \
    [1] = { { K_PGUP, K_PGDN } }  \
}
```

- При вращении энкодера прошивка генерирует кратковременный импульс нажатия (20 мс) и автоматически отпускает клавишу.
- Если включена поддержка `#define VIAL`, энкодеры можно переназначать прямо во вкладке *Encoders* графического интерфейса Vial.

