# Система внешних модулей и расширений в DMK

В DMK реализована модульная архитектура, позволяющая подключать кастомные драйверы оборудования, индикаторы, экраны и алгоритмы без модификации исходного кода ядра прошивки.

---

## 1. Структура модуля

Внешний модуль представляет собой отдельную директорию с файлом `module.cmake`, исходными кодами и заголовочными файлами.

```
my_custom_module/
├── module.cmake           # Инструкции сборки CMake для модуля
├── include/               # Публичные заголовки модуля
│   └── my_module.h
├── my_module.c            # Реализация модуля
└── custom_driver.pio      # (Опционально) Ассемблерная программа PIO для RP2040 / RP2350
```

### Пример `module.cmake`:
```cmake
target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/my_module.c"
)

target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)
```

---

## 2. Подключение модулей при сборке

Список директорий модулей передается через переменную CMake `-DDMK_MODULES` (разделяются точкой с запятой):

```bash
# Подключение одного модуля
cmake -B build -DKEYBOARD=corne -DDMK_MODULES=/path/to/my_custom_module -DMCU=rp2040

# Подключение нескольких модулей
cmake -B build -DKEYBOARD=magneteno -DDMK_MODULES="tests/modules/hall_calibration;tests/modules/sharp_memory_lcd" -DMCU=rp2350
```

Поддерживаются как относительные пути (от корня репозитория), так и абсолютные пути к внешним папкам на диске.

---

## 3. Хуки событий и жизненного цикла ядра (Hooks API)

Связь модулей с ядром DMK построена на слабых (weak) хуках, объявленных в `dmk_core/include/hooks.h`. Любой модуль может переопределить нужные функции:

| Функция хука | Момент вызова | Типичное применение |
| :--- | :--- | :--- |
| `void hook_early_init(void)` | В `main.c` перед `vTaskStartScheduler()` | Создание фоновых FreeRTOS задач (отрисовка экрана, опрос датчиков) |
| `void hook_layer_change(uint8_t active_layer)` | При смене активного слоя в `layers.c` | Светодиодная / RGB индикация слоев, отображение на OLED/LCD |
| `void hook_matrix_change(uint8_t row, uint8_t col, bool pressed)` | При каждом изменении состояния кнопки матрицы | Отладочные светодиоды нажатий, звуковой отклик |
| `void hook_key_sent(uint16_t keycode, bool pressed)` | При отправке HID кейкода хосту через USB | Расчет скорости печати WPM в реальном времени |
| `void hook_hid_led_change(uint8_t led_mask)` | При обновлении состояния Lock-клавиш хостом | Индикаторы CapsLock (`0x02`), NumLock (`0x01`), ScrollLock (`0x04`) |

### Пример реализации хука в модуле:
```c
#include "hooks.h"
#include "rgb.h"
#include <stdint.h>
#include <stdbool.h>

#define LAYER_LED_INDEX 0 // Первый светодиод — индикатор слоя
#define CAPS_LED_INDEX  1 // Второй светодиод — индикатор CapsLock

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

## 4. Прямое управление RGB подсветкой

В `dmk_core/drivers/rgb.h` доступны низкоуровневые функции для управления отдельными светодиодами из модулей:

* `void rgb_set_pixel_raw(uint32_t index, uint32_t color_hex)` — Устанавливает цвет светодиода по индексу в формате `0xRRGGBB`.
* `void rgb_show(void)` — Мгновенно передает кадровый буфер RGB на светодиоды.

---

## 5. Кастомные матрицы (Датчики Холла, MUX, Трекболы)

Если клавиатура использует нестандартный механизм сканирования:
1. Задайте `#define CUSTOM_MATRIX 1` в `config.h` клавиатуры или передайте `-DCUSTOM_MATRIX=ON` в CMake. При этом стандартный драйвер `matrix.c` исключается из сборки.
2. Реализуйте функции `matrix_task(void *pvParameters)` и `matrix_is_pressed(row, col)`.
3. Отправляйте события нажатия в очередь `matrix_queue`:
   ```c
   matrix_event_t event = { .split = 0, .row = r, .col = c, .pressed = 1 };
   xQueueSend(matrix_queue, &event, 0);
   ```

### Модуль калибровки датчиков Холла (`hall_calibration`)
Модуль в `tests/modules/hall_calibration` предоставляет:
* **Калибровку конечных точек:** `rest_adc` (нулевая точка покоя) и `bottom_adc` (максимальный ход).
* **Настройку точки срабатывания:** Динамический порог 5%..95% хода клавиши.
* **Continuous Rapid Trigger:** Мгновенный сброс клавиши при обратном движении вверх (например, на 0.1 мм) и повторное срабатывание на лету.
* **Запись во Flash/EEPROM:** Сохранение профилей в выделенный сектор Flash на RP2040/RP2350.

---

## 6. Дисплеи (U8g2 и Sharp Memory LCD)

Библиотека `lib/u8g2` подключена сабмодулем и доступна для вывода графики и текста на экраны.

* **Sharp Memory LCD (`LS011B7DH03` 160x68):**
  * Использует нативный драйвер `u8g2_Setup_ls011b7dh03_160x68_f`.
  * Автоматическое стробирование полярности `EXTCOMIN` 1 Гц для защиты кристаллов.
  * Расчет скорости набора текста WPM (Words Per Minute) по 5-секундному скользящему окну со спидометром.
  * Отображение имени активного слоя, бейджей `[CAPS]`, `[NUM]` и статуса USB подключения.

---

## 7. Программируемый ввод-вывод (PIO) на RP2040 и RP2350

Любые файлы ассемблера `*.pio`, помещенные в директорию модуля или клавиатуры, **автоматически компилируются** при помощи утилиты `pioasm` через вызов `pico_generate_pio_header()`.

* Сгенерированные заголовки `*.pio.h` сразу доступны для включения: `#include "my_driver.pio.h"`.
* Библиотека `hardware_pio` уже слинкована с таргетом прошивки.

---

## 8. Примеры готовых модулей

В директории `tests/modules/` находятся эталонные реализации:

| Путь к модулю | Описание |
| :--- | :--- |
| `tests/modules/led_layer_indicator` | Переключение дискретных GPIO светодиодов в зависимости от активного слоя. |
| `tests/modules/rgb_layer_indicator` | Изменение цвета RGB подсветки/индикатора по номеру слоя. |
| `tests/modules/debug_indicator` | Произвольная маршрутизация событий USB и нажатий клавиш на GPIO или RGB диоды. |
| `tests/modules/hall_calibration` | Калибровка датчиков Холла, Continuous Rapid Trigger и сохранение во Flash. |
| `tests/modules/sharp_memory_lcd` | Информационный дисплей Sharp MIP LCD с WPM спидометром, слоями и CapsLock. |
| `tests/modules/u8g2_display` | Базовый движок вывода на экраны на базе библиотеки U8g2. |
