**Язык:** [Русский](build.md) | [English](../en/build.md)

**Документация:** [Установка](install.md) • [Бэкап и восстановление](backup-restore.md) • [Сборка](build.md) • [Конфигурация](config.md) • [Модели](models.md) • [VIA & Debug](via-debug.md)

# Сборка

Запускайте сборку из корня репозитория. DMK поддерживает сборку для всех линеек HHKB: Classic, Classic Type-S, Hybrid и Hybrid Type-S (как ANSI 60 клавиш, так и японских JIS 69 клавиш).

## Команды сборки

### 1. HHKB Classic и Classic Type-S (Wired)

```bash
# Classic (ANSI 60 кл.):
./build_all.sh -b hhkb -d HHKB_CLASSIC

# Classic Type-S (ANSI 60 кл.):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S

# Classic Type-S JIS (69 кл.):
./build_all.sh -b hhkb -d HHKB_CLASSIC_TYPE_S,HHKB_JIS
```

PowerShell (Windows):
```powershell
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S
.\build_all.ps1 -Keyboard hhkb -Define HHKB_CLASSIC_TYPE_S,HHKB_JIS
```

### 2. HHKB Hybrid и Hybrid Type-S (Bluetooth + USB)

Для Hybrid собирается сырой бинарный образ STM32 с поддержкой SPI-протокола обмена с чипом Nordic nRF52832:

```bash
# Hybrid / Hybrid Type-S (ANSI 60 кл.):
./build_all.sh -b hhkb -d HHKB_HYBRID -c

# Hybrid / Hybrid Type-S JIS (69 кл.):
./build_all.sh -b hhkb -d HHKB_HYBRID,HHKB_JIS -c
```

После сборки бинарник упаковывается в финальный контейнер поверх оригинальной прошивки PFU скриптом `tools/hhkb/patch_hybrid_hfb.py` (подробнее ниже в разделе [Патчинг заводского образа Hybrid](#только-hybrid-локальная-сборка-и-патчинг-заводского-образа)).

STM32L072 выбирается через `DEFAULT_MCU` в конфигурации клавиатуры, поэтому в команде сборки не нужен аргумент MCU. В каталоге результата создаются контейнер `.hfb` для PFU и сырой `.bin` для Bank 2; сборка не прошивает клавиатуру.

## Идентификаторы моделей и форматы контейнеров HFB

| Линейка клавиатур | Раскладка | Флаги сборки DMK | ID модели PFU | Линейка версий | Результирующий контейнер |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Classic** | ANSI (60 кл.) | `HHKB_CLASSIC` | `AHHX01` | `A4.xx` (A4.50) | Готовый `.hfb` (65 570 байт) |
| **Classic Type-S** | ANSI (60 кл.) | `HHKB_CLASSIC_TYPE_S` | `AHHS01` | `A8.xx` (A8.50) | Готовый `.hfb` (65 570 байт) |
| **Classic Type-S JIS** | JIS (69 кл.) | `HHKB_CLASSIC_TYPE_S,HHKB_JIS` | `AHKS01` | `A9.xx` (A9.50) | Готовый `.hfb` (65 570 байт) |
| **Hybrid / Type-S** | ANSI (60 кл.) | `HHKB_HYBRID` | `AHUX01` | `A0.xx` (A0.48) | Патчинг `HHKB800_FW_A048.hfb` (282 672 байта) |
| **Hybrid / Type-S JIS** | JIS (69 кл.) | `HHKB_HYBRID,HHKB_JIS` | `AHJX01` | `A2.xx` (A2.49) | Патчинг `HHKB820_FW_JP_A249.hfb` (282 672 байта) |

### Структура контейнера HFB для проводных моделей (Classic)

Файл HHKB `.hfb` для проводных моделей имеет фиксированный размер ровно 65 570 байт:

```text
+---------------------------------------------------------------+
| Внешний CRC-16/MCRF4XX, little-endian (2 байта)                |
+---------------------------------------------------------------+
| Внутренний CRC-16/MCRF4XX полезной нагрузки Bank 2 (2 байта)   |
+---------------------------------------------------------------+
| Прошивка Bank 2, дополненная 0xFF (65 536 байт)                |
+---------------------------------------------------------------+
| Постфикс: 14 × 0xFF, ID модели (6), BCD-версия (4), 6 × 0xFF  |
+---------------------------------------------------------------+
```

Обе контрольные суммы используют отражённый полином `0x8408`, начальное значение `0xFFFF` и не используют финальный XOR. Внутренний CRC зависит от полезной нагрузки, это не постоянный тег модели. Образ линкуется для Bank 2 по адресу `0x08010000`.

`tools/hhkb/make_hfb.py` собирает варианты Classic и автоматически считает CRC. При ручном запуске передаются `--model` и `--version`.

## 3. Потребление памяти (Flash и RAM)


Текущий слинкованный образ STM32L072 резервирует 20 480 байт RAM: `.data` — 288 байт, `.bss` — 16 888 байт, `._user_heap_stack` — 2 048 байт; всего 19 224 байта (93,9%), остаётся 1 256 байт в рамках линковочной разметки. Это распределение при сборке, а не измерение максимальной глубины стека во время работы.

## GitHub Actions

CI для Hybrid публикует STM32-бинарники ANSI и JIS. Скачайте подходящий бинарник и [HTML-патчер](../../../../tools/hhkb/hhkb.html). Откройте страницу локально, выберите официальный HFB и бинарник DMK, затем нажмите Patch and download. Заводская прошивка радио сохраняется. Шаг сборки HFB с пустым радио закомментирован.

[Workflow Build Firmware](../../../../.github/workflows/build_test.yml) запускается для `push` и `pull_request` в `main`/`master`; его можно запустить вручную через **Actions → Build Firmware → Run workflow**. После успешного запуска скачайте в разделе **Artifacts** нужный архив:

- `dmk-hhkb-classic-classic-via.hfb` — обычная Classic, модель `AHHX01`, версия HFB A4.50.
- `dmk-hhkb-classic-type-s-via.hfb` — Classic Type-S, модель `AHHS01`, версия HFB A8.50.

В каждом архиве лежат `.hfb` для выбранной модели и [via.json](../../via.json) для загрузки пользовательского меню через вкладку Design [веб-версии VIA](https://www.usevia.app/). Выбирайте архив по модели клавиатуры; сборка Type-S пока проверена компилятором, на плате проверялась обычная Classic.



## Только Hybrid: локальная сборка и патчинг заводского образа

Нужен **именно** заводской файл `HHKB800_FW_A048.hfb`. Получите его отдельно в [официальном центре загрузки HHKB](https://happyhackingkb.com/download/) или используйте сохранённую копию. Ожидаемый SHA-256: `635b995eb5a15aa50c2cedc60da7d8998fcca8285a09d41d984c07fffcaf9d43`. Оригинал и полученные контейнеры храните в каталоге `keyboards/hhkb/local/`, исключённом из Git.

Из корня репозитория:

```bash
mkdir -p keyboards/hhkb/local
# Сначала положите исходный HHKB800_FW_A048.hfb в keyboards/hhkb/local/.
./build_all.sh -b hhkb -d HHKB_HYBRID -c
python3 tools/hhkb/patch_hybrid_hfb.py \
  build/hhkb-script/dmk_hhkb_stm32l072.bin \
  keyboards/hhkb/local/HHKB800_FW_A048.hfb \
  keyboards/hhkb/local/dmk_hybrid.hfb
```


Для **Hybrid JIS** добавьте `HHKB_JIS` к флагам сборки и передайте патчеру `--layout jis` вместе с оригинальным `HHKB820_FW_JP_A249.hfb`. Патчер сохраняет модель `AHJX01` и версию `A2.49` из оригинала. Для ANSI используется `AHUX01 / A0.48`.

[patch_hybrid_hfb.py](../../../../tools/hhkb/patch_hybrid_hfb.py) проверяет хеш оригинала, размер и таблицу векторов Bank 2, заменяет только 64 КиБ приложения STM32, дополняет их `FF` и пересчитывает две CRC. Заводской образ nRF, постфикс и версия сохраняются. Скрипт не перезаписывает существующий выходной файл: для новой сборки укажите новое имя. **Работоспособность полученного таким образом образа проверена и подтверждена на реальном оборудовании HHKB Hybrid.**

Rapid Trigger и АЦП 16 МГц доступны во всех сборках HHKB. Rapid Trigger и оверсемплинг настраиваются в VIA во время работы.

## Выключение Hybrid

Все сборки Hybrid теперь используют проверенный в сборке #204 путь OFF по разбору стока: Nordic idle по SPI, STOP STM32 без удержания PD2 и продолжение работы без сброса STM32 после обычного OFF. Дополнительный флаг сборки не нужен. Сравнительные варианты MIT/stock убраны из CI. Артефакты Hybrid ANSI и JIS, включая Type-S, используют одну реализацию. См. [режимы питания и измерения](config.md#режимы-питания-и-pc5).
