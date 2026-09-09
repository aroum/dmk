🌐 **Язык / Language:** [Русский](modules.md) | [English](../en/modules.md)

📖 **Навигация / Navigation:** [Сборка](build.md) • [Конфигурация](config.md) • [Коды клавиш](keycodes.md) • [Раскладка](keymap.md) • [Пины](pins.md) • [Vial](vial.md) • [Модули](modules.md) • [MIDI](midi.md)

---

# Система внешних модулей и расширений в DMK

В DMK реализована гибкая и модульная архитектура расширений, позволяющая подключать кастомные аппаратные драйверы, светодиодные индикаторы, экраны, аналоговые датчики и фоновые задачи без модификации и засорения ядра прошивки.

### Ключевые преимущества модульной системы:
* **Чистота кодовой базы ядра**: Любой экспериментальный или специфичный для конкретного проекта код живет в изолированной директории.
* **Бесшовное обновление**: Вы можете обновлять DMK из апстрим-репозитория без конфликтов слияния.
* **Поддержка отдельных репозиториев пользователя**: Возможность вести свои клавиатуры, раскладки и модули в отдельном GitHub-репозитории и собирать прошивку через GitHub Actions без создания форка ядра DMK.

---

## 1. Структура и анатомия модуля

Внешний модуль представляет собой изолированную папку, содержащую манифест сборки `module.cmake`, исходный код на C и заголовочные файлы.

```text
my_custom_module/
├── module.cmake           # Инструкции сборки CMake для модуля
├── include/               # Публичные заголовки (автоматически добавляются в include path)
│   └── my_module.h
├── my_module.c            # Реализация модуля
├── custom_driver.pio      # (Опционально) Ассемблерная программа PIO для RP2040 / RP2350
└── README.md              # (Опционально) Описание модуля
```

### Синтаксис `module.cmake`
Внутри `module.cmake` используются стандартные директивы CMake для подключения исходных файлов и зависимостей к таргету прошивки `${TARGET_NAME}`:

```cmake
# Добавление исходных файлов модуля
target_sources(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/my_module.c"
)

# Добавление путей к заголовкам (корень модуля и include/ ядро DMK подключает автоматически)
target_include_directories(${TARGET_NAME} PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/include"
)

# Опционально: флаги компиляции или системные библиотеки
# target_compile_definitions(${TARGET_NAME} PRIVATE HAS_MY_MODULE=1)
# target_link_libraries(${TARGET_NAME} PRIVATE hardware_adc)
```

---

## 2. Подключение модулей при сборке

Список модулей передается через аргумент CMake `-DDMK_MODULES` (директории разделяются точкой с запятой):

```bash
# Подключение одного модуля (относительный или абсолютный путь)
cmake -B build -DKEYBOARD=corne -DDMK_MODULES=tests/modules/led_layer_indicator -DMCU=rp2040

# Подключение нескольких модулей (берутся в кавычки, разделяются ;)
cmake -B build -DKEYBOARD=magneteno -DDMK_MODULES="tests/modules/hall_calibration;tests/modules/sharp_memory_lcd" -DMCU=rp2350
```

Поддерживаются как относительные пути (от корня репозитория DMK), так и абсолютные пути на диске (например, `/home/user/my_modules/display_driver`).

### Модули внутри папки клавиатуры (`keyboards/<клавиатура>/modules/`)
Если модуль относится к конкретной клавиатуре (например, `keyboards/omsk/modules/midi_jack` или кастомная матрица `keyboards/magneteno/modules/matrix_magneteno`):
* Поместите модуль прямо в подпапку `keyboards/<клавиатура>/modules/<имя_модуля>/`.
* Система сборки DMK **автоматически находит и подключает все модули из папки `modules/` клавиатуры** при её компиляции!
* Передавать `-DDMK_MODULES` или `-m` в командной строке для них **не требуется**:
  ```bash
  # Сборка omsk автоматически подключит модуль keyboards/omsk/modules/midi_jack:
  ./build_all.sh -b omsk -c

  # Сборка magneteno автоматически подключит модули matrix_magneteno и hall_calibration:
  ./build_all.sh -b magneteno -c
  ```

---

## 3. Хуки событий и жизненного цикла ядра (Hooks API)

Связь модулей с ядром DMK построена на механизме неблокирующих слабых хуков (weak functions), объявленных в `dmk_core/include/hooks.h`. Для использования хука модуль просто объявляет соответствующую функцию в своем C-коде:

| Функция хука | Момент вызова | Типичное применение |
| :--- | :--- | :--- |
| `void hook_early_init(void)` | В `main.c` перед запуском `vTaskStartScheduler()` | Инициализация GPIO/шин, создание фоновых FreeRTOS задач (`xTaskCreate`) |
| `void hook_layer_change(uint8_t active_layer)` | При смене активного слоя клавиатуры (`layers.c`) | Индикация слоя на светодиодах/RGB, обновление слоя на экране |
| `void hook_matrix_change(uint8_t row, uint8_t col, bool pressed)` | При каждом физическом изменении кнопки (`matrix.c`) | Отладочные светодиоды нажатий, тактильный отклик (haptic), зуммер |
| `void hook_key_sent(uint16_t keycode, bool pressed)` | При отправке HID-кейкода хосту по USB (`keys.c`) | Расчет скорости печати WPM в реальном времени, кейлоггинг |
| `void hook_hid_led_change(uint8_t led_mask)` | При обновлении хостом статуса Lock-клавиш (`led.c`) | Индикация CapsLock (`0x02`), NumLock (`0x01`), ScrollLock (`0x04`) |
| `bool hook_mouse_move(int8_t *dx, int8_t *dy)` | Перед отправкой отчета движения курсора (`mouse.c`) | Перехват движения трекбола/мыши, drag-scroll (скролл колесом при зажатии слоя), масштабирование DPI |
| `bool hook_mouse_scroll(int8_t *wheel, int8_t *pan)` | Перед отправкой отчета прокрутки (`mouse.c`) | Инверсия или программная фильтрация вертикального и горизонтального скролла |
| `void hook_mouse_report(uint8_t buttons, int8_t dx, int8_t dy, int8_t wheel, int8_t pan)` | Перед отправкой составного USB HID Mouse отчета | Аналитика, светодиодная индикация кликов или репликация на вторичные интерфейсы |
| `void hook_midi_send(const uint8_t *msg, uint8_t len)` | При отправке MIDI-сообщения (`midi.c`) | Вывод в аппаратный DIN-5 / TRS MIDI Jack по UART, передача по BLE MIDI или CV/Gate |

### Пример реализации хуков в модуле:
```c
#include "hooks.h"
#include "rgb.h"
#include "hal_gpio.h"
#include <stdint.h>
#include <stdbool.h>

#define LAYER_LED_INDEX 0 // Первый светодиод — индикатор слоя
#define CAPS_LED_INDEX  1 // Второй светодиод — индикатор CapsLock

void hook_layer_change(uint8_t active_layer) {
    const uint32_t colors[] = { 0x00FF00, 0x0000FF, 0xFF00FF, 0xFFFF00 };
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

## 4. Аппаратная абстракция и сервисы ядра для модулей

Модули имеют прямой доступ ко всем подсистемам ядра DMK и API операционной системы реального времени FreeRTOS:

### 4.1. Кроссплатформенный GPIO HAL (`hal_gpio.h`)
Единый аппаратно-независимый слой управления выводами микроконтроллера (Миландр, RP2040, RP2350, nRF52840, Байкал):
* `void hal_gpio_init(uint32_t pin)` — Инициализирует пин для работы в режиме GPIO.
* `void hal_gpio_set_dir(uint32_t pin, bool out)` — Задает направление (`true` — выход, `false` — вход).
* `void hal_gpio_put(uint32_t pin, bool value)` — Устанавливает высокий (`true`) или низкий (`false`) уровень напряжения.
* `bool hal_gpio_get(uint32_t pin)` — Считывает текущее логическое состояние пина.

### 4.2. Фоновые задачи FreeRTOS (`task.h`, `queue.h`)
Модуль может запускать собственные потоки обработки в `hook_early_init()`:
```c
static void my_background_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        // Неблокирующая периодическая обработка
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void hook_early_init(void) {
    xTaskCreate(my_background_task, "bg_task", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
}
```

### 4.3. Прямое управление адресной RGB-подсветкой (`rgb.h`)
Низкоуровневые функции позволяют модулю управлять отдельными пикселями без сбоя стандартных эффектов подсветки:
* `void rgb_set_pixel_raw(uint32_t index, uint32_t color_hex)` — Устанавливает цвет светодиода по индексу в формате `0xRRGGBB`.
* `void rgb_show(void)` — Передает буфер кадров на физические светодиоды (WS2812 / SK6812).
* `void rgb_set_color(uint8_t hue, uint8_t sat)` — Устанавливает глобальный цвет подсветки в пространстве HSV.
* `void rgb_set_mode(uint8_t mode)` — Переключает режим анимации подсветки.

### 4.4. Эмуляция мыши и прямое управление трекболом (`mouse.h`)
DMK включает полноценную подсистему USB HID мыши с поддержкой одновременного движения, колесика прокрутки (вертикального и горизонтального через AC Pan) и 5 кнопок:
* `void mouse_move(int8_t dx, int8_t dy)` — Относительное перемещение курсора мыши. Вызывает `hook_mouse_move(&dx, &dy)`, позволяя модулям перехватить смещение (например, перенаправить движение трекбола на скролл) или масштабировать чувствительность.
* `void mouse_scroll(int8_t wheel, int8_t pan)` — Прокрутка колесиком (`wheel` — вертикальная, `pan` — горизонтальная). Вызывает `hook_mouse_scroll(&wheel, &pan)`.
* `void mouse_button_set(uint8_t button_mask, bool pressed)` — Установка битовой маски кнопок (`MOUSE_BTN_LEFT`, `MOUSE_BTN_RIGHT`, `MOUSE_BTN_MIDDLE`, `MOUSE_BTN_BACK`, `MOUSE_BTN_FORWARD`).
* `void mouse_button_press(uint8_t button_mask)` / `mouse_button_release(uint8_t button_mask)` — Мгновенное нажатие и отпускание кнопок мыши.

**Клавиши управления мышью в раскладке (Mousekeys):**
* **Кнопки:** `K_MS_BTN1`..`K_MS_BTN5` (псевдонимы `K_BTN1`..`K_BTN5`).
* **Движение курсора:** `K_MS_UP`, `K_MS_DOWN`, `K_MS_LEFT`, `K_MS_RIGHT` (с плавной физикой разгона, настраиваемой параметрами `MOUSEKEY_BASE_SPEED`, `MOUSEKEY_MAX_SPEED`, `MOUSEKEY_TIME_TO_MAX`).
* **Колесико:** `K_MS_WH_UP`, `K_MS_WH_DOWN`, `K_MS_WH_LEFT`, `K_MS_WH_RIGHT`.
* **Режимы скорости:** `K_MS_ACCEL0` (Slow / прецизионный режим для пиксельной точности), `K_MS_ACCEL1` (Normal), `K_MS_ACCEL2` (Fast / турбо).

---

## 5. Кастомные матрицы (Датчики Холла, Rapid Trigger, MUX, Трекболы)

Если ваша клавиатура использует нестандартный механизм опроса клавиш (аналоговые датчики Холла, мультиплексоры, оптические сенсоры):
1. Задайте `#define CUSTOM_MATRIX 1` в `config.h` клавиатуры или передайте `-DCUSTOM_MATRIX=ON` в CMake. При этом стандартный драйвер `dmk_core/drivers/matrix.c` исключается из сборки.
2. Модуль реализует задачи `matrix_task(void *pvParameters)` и проверку `bool matrix_is_pressed(uint8_t row, uint8_t col)`.
3. Модуль отправляет события в системную очередь `matrix_queue` в формате `matrix_event_t`:
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

### Модуль калибровки датчиков Холла (`tests/modules/hall_calibration`)
Встроенный эталонный модуль предоставляет:
* **Автоматическую калибровку конечных точек:** `rest_adc` (нулевая зона покоя) и `bottom_adc` (полное нажатие).
* **Настраиваемую точку срабатывания:** Динамический порог хода от 5% до 95%.
* **Continuous Rapid Trigger:** Мгновенный сброс нажатия при начале обратного движения вверх (например, на 0.1 мм) и мгновенное повторное срабатывание при возобновлении хода вниз.
* **Сохранение во Flash-память:** Профили калибровки сохраняются в энергонезависимой Flash-памяти RP2040/RP2350.

---

## 6. Дисплеи (U8g2 и Sharp Memory LCD)

Библиотека `lib/u8g2` подключена сабмодулем DMK и доступна модулям для вывода графики и шрифтов на любые монохромные и градационные экраны.

* **Sharp Memory LCD (`LS011B7DH03` 160x68):**
  * Использует оптимизированный буфер драйвера `u8g2_Setup_ls011b7dh03_160x68_f`.
  * Фоновая задача FreeRTOS генерирует стробирование полярности `EXTCOMIN` 1 Гц для защиты кристаллов от деградации.
  * Расчет скорости набора текста WPM (Words Per Minute) в реальном времени со стрелочным/полосковым спидометром.
  * Индикаторы активного слоя, статусы CapsLock, NumLock и состояние USB соединения.

---

## 7. Программируемый ввод-вывод (PIO) на RP2040 и RP2350

Любые ассемблерные файлы `*.pio`, помещенные в директорию модуля, **автоматически компилируются** с помощью утилиты `pioasm` через функцию Pico SDK `pico_generate_pio_header()`.

* Сгенерированные заголовочные файлы `*.pio.h` сразу готовы к подключению: `#include "my_driver.pio.h"`.
* Все низкоуровневые аппаратные библиотеки Pico SDK (`hardware_pio`, `hardware_dma`, `hardware_timer`) уже слинкованы с целевым таргетом.

---

## 8. Примеры готовых эталонных модулей

В директории `tests/modules/` находятся полнофункциональные примеры:

| Путь к модулю | Описание |
| :--- | :--- |
| `tests/modules/led_layer_indicator` | Управление отдельными GPIO светодиодами в зависимости от активного слоя. |
| `tests/modules/rgb_layer_indicator` | Динамическая смена цветовой палитры RGB-подсветки/индикатора по номеру слоя. |
| `tests/modules/debug_indicator` | Маршрутизация событий USB-подключения и нажатий клавиш на GPIO или RGB диоды. |
| `tests/modules/hall_calibration` | Калибровка аналоговых датчиков Холла, Continuous Rapid Trigger и сохранение во Flash. |
| `tests/modules/sharp_memory_lcd` | Панель приборов на экране Sharp Memory LCD с расчетом WPM, слоями и бейджами. |
| `tests/modules/u8g2_display` | Универсальный графический движок вывода на экраны на базе библиотеки U8g2. |
| `tests/modules/trackball_example` | Интеграция трекбола/оптического сенсора с хуком `hook_mouse_move` для drag-scroll на слое. |
| `keyboards/omsk/modules/midi_jack` | Физический транспорт DIN-5 / TRS MIDI Jack через аппаратный UART (31250 бод) через хук `hook_midi_send`. |
| `keyboards/magneteno/modules/matrix_magneteno` | Кастомный драйвер сканирования матрицы Hall-Effect через мультиплексор SN74LV4052A. |

## 9. Сборка модулей в изолированном репозитории и CI/CD

Пользовательские модули не обязательно хранить внутри дерева исходных кодов DMK. Вы можете расположить их в своем отдельном репозитории конфигурации (например, в папке `modules/my_module`) и передавать относительный или абсолютный путь в CMake через аргумент `-DDMK_MODULES`.

Полный пример организации репозитория конфигурации и готовый шаблон автоматической матричной сборки через **GitHub Actions CI/CD** описан в [Руководстве по сборке (build.md)](build.md#автоматическая-сборка-в-своем-репозитории-github-actions).

