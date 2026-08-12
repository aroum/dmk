🌐 **Язык / Language:** [Русский](build.md) | [English](../en/build.md)

📖 **Навигация / Navigation:** [Сборка](build.md) • [Конфигурация](config.md) • [Коды клавиш](keycodes.md) • [Раскладка](keymap.md) • [Пины](pins.md) • [Vial](vial.md)

---

## Установка зависимостей

Для сборки прошивки понадобятся компилятор `ARM GCC` и инструмент `uv`.

### Linux (Ubuntu/Debian)

1. Установите системные утилиты и инструменты сборки:

   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake ninja-build git python3 python3-pip
   ```

2. Установите компилятор ARM GCC:

   ```bash
   sudo apt install -y gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi
   ```

3. Установите `uv` (рекомендуемый менеджер окружения Python):

   ```bash
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

### macOS

1. Установите Homebrew (если он еще не установлен):

   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

2. Установите инструменты и компилятор:

   ```bash
   brew install cmake ninja git python
   brew tap osx-cross/arm
   brew install arm-gcc-bin
   ```

3. (Опционально) Установите утилиты для прошивки по USB:

   ```bash
   brew install dfu-util
   brew install picotool
   brew install --cask nrfutil
   ```

4. Установите `uv`:

   ```bash
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

### Windows

1. **Используя Scoop (Рекомендуется)**:
   Откройте PowerShell и выполните:

   ```powershell
   # Установка Scoop (если не установлен)
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   Invoke-RestMethod -Uri https://get.scoop.sh | Invoke-Expression

   # Установка необходимых утилит
   scoop install git cmake ninja python uv

   # Установка компилятора ARM GCC
   scoop bucket add extras
   scoop install gcc-arm-none-eabi
   ```

2. **Используя winget**:
   Откройте Командную строку / PowerShell и выполните:

   ```cmd
   winget install Kitware.CMake
   winget install Python.Python.3
   winget install Git.Git
   winget install Ninja-build.Ninja
   ```

   Для установки компилятора ARM GCC скачайте установщик напрямую с [сайта разработчика ARM](https://developer.arm.com/downloads/-/gnu-rm) и добавьте путь к папке `bin` в переменную окружения System PATH.
3. Установите `uv`:

   ```powershell
   powershell -c "irm https://astral.sh/uv/install.ps1 | iex"
   ```

---

## Использование

Для запуска сборки используйте скрипт `build_all.sh`:

```
./build_all.sh [ФЛАГИ]
```

## Описание аргументов и флагов

### Основные параметры

- `--keyboard [NAME]`/ `-b [NAME]` — Выбор клавиатуры. `NAME` соответствует названию подпапки в директории `keyboards`.
- `--probe [PROBE]` / `-p [PROBE]` — Выбор отладчика для прошивки `milandr`. По умолчанию используется `j-link`.
- `--mcu [MCU]` — Выбор целевого микроконтроллера (`milandr`, `rp2040`, `rp2350`, `nrf52840`, `baikal`). По умолчанию используется `milandr`.
- `--memory [SIZE]` — Выбор размера памяти для прошивки (например, `256KB`, `512KB`, `2MB`, `4MB`, `16MB`). Используется для плат с внешней флеш-памятью.
- `--define [DEFS]`/`-d [DEFS]` Например `-d  LEFT, 6_COL_LAYOUT, KEEPER_KEYMAP`
- `--uf2` — Сгенерировать UF2-файл (для RP2040/RP2350/nRF52840).
- `--lang [LANG]` — Установить язык скрипта (`ru` или `en`).

### Комбинированные флаги

Можно использовать объединение флагов (например, `-cf`):

- `-c`, `--clean` — Выполнить «чистую» сборку (очистка временных файлов).
- `-f`, `--flash` — Прошивка через внешник отладчик.
- `-u`, `--usb-flash` — Прошивка через USB (для RP2040/RP2350, nRF52 и Milandr).
- `-h`, `--help` — Показать справку.

## Способы и инструменты прошивки

Прошить микроконтроллер можно несколькими способами в зависимости от платформы и наличия отладчика:

### 1. Прошивка UF2 (как на обычную флешку)

Для плат с UF2 bootloader (RP2040, RP2350, nRF52840):

1. Зажмите кнопку `BOOTSEL` / `RESET` при подключении USB и вставьте плату в ПК — она появится в системе как обычный съемный диск.
2. Просто скопируйте собранный файл `.uf2` из папки `build/` на этот диск. Плата прошьется и перезагрузится автоматически.

### 2. Инструменты для автоматической прошивки по USB (`-u`, `--usb-flash`)

- **RP2040 / RP2350**: [`picotool`](https://github.com/raspberrypi/picotool) — автоматическая прошивка через USB.
  - macOS: `brew install picotool`
- **nRF52840**: [`nrfutil`](https://pypi.org/project/nrfutil/) / `adafruit-nrfutil` — прошивка через DFU-бутлоадер Adafruit.
  - macOS: `brew install --cask nrfutil`
  - Linux / Windows: `pip install nrfutil` / `pip install adafruit-nrfutil`
- **Миландр (K1986BE92FI)**: [`dfu-util`](http://dfu-util.sourceforge.net/) — прошивка с бутлоадером [K1986BE92FI-dfu-bootloader](https://github.com/aroum/K1986BE92FI-dfu-bootloader).
  - macOS: `brew install dfu-util`
  - Linux: `sudo apt install dfu-util`

### 3. Прошивка через внешний отладчик (`-f`, `--flash`)

- **[OpenOCD](https://openocd.org/)**: Используется для прямой прошивки микроконтроллеров по SWD/JTAG (J-Link, ST-Link, CMSIS-DAP). Поддерживает Миландр, RP2040, nRF52840 и Байкал.

Установка OpenOCD через пакетный менеджер вашей системы:

- **macOS (Homebrew)**: `brew install open-ocd`
- **Debian / Ubuntu**: `sudo apt install openocd`
- **Fedora**: `sudo dnf install openocd`
- **Windows (MSYS2)**: `pacman -S mingw-w64-x86_64-openocd`

Полезные ссылки на утилиты:

- **picotool (RP2040/RP2350)**: <https://github.com/raspberrypi/picotool>
- **nrfutil (nRF52840)**: <https://pypi.org/project/nrfutil/>
- **dfu-util (USB DFU)**: <http://dfu-util.sourceforge.net/>
- **OpenOCD (SWD/JTAG)**: <https://openocd.org/>
- **Milandr DFU Bootloader**: <https://github.com/aroum/K1986BE92FI-dfu-bootloader>

## Примеры использования

### Milandr

**Полная сборка с прошивкой через отладчик и очисткой для сплит-клавиатуры:**

```bash
./build_all.sh -b corne -d LEFT -cf
./build_all.sh -b corne -d RIGHT -cf
```

**Чистая сборка и прошивка по USB DFU:**

```bash
./build_all.sh -b corne -d LEFT -cu
```

### Raspberry Pi (RP2040/RP2350)

**Сборка с генерацией UF2:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040
```

**Чистая сборка и прошивка через USB:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cu
```

**Чистая сборка, UF2 и USB-прошивка одновременно:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cu --uf2
```

**Чистая сборка с указанием размера памяти:**

```bash
./build_all.sh -b pncateho --uf2 --mcu rp2040 -cf --memory 512KB
```

## Автоматическая сборка в своем репозитории (GitHub Actions)

Вы можете вынести конфигурацию своей клавиатуры в отдельный репозиторий и собирать прошивку автоматически при каждом `git push` через GitHub Actions.

### Структура пользовательского репозитория

Ваш репозиторий конфигурации должен иметь следующую структуру файлов:

```text
my-dmk-config/
├── .github/
│   └── workflows/
│       └── build.yml               # Workflow файл GitHub Actions
└── keyboards/
    └── my_keyboard/                # Имя вашей клавиатуры (передается в -b)
        ├── config.h                # Конфигурация пинов и матрицы
        ├── keymap.c                # Раскладка клавиш
        └── vial.json               # (Опционально) Конфигурация для Vial
```

### Шаблон файла `.github/workflows/build.yml`

Шаблон готового `workflow`:

```yaml
name: Build DMK Firmware

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]
  workflow_dispatch:

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
      - name: Checkout User Configuration
        uses: actions/checkout@v4
        with:
          path: user_config

      - name: Checkout DMK Firmware Core
        uses: actions/checkout@v4
        with:
          repository: aroum/dmk
          path: dmk
          submodules: recursive

      - name: Install Toolchain and Dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cmake ninja-build gcc-arm-none-eabi libnewlib-arm-none-eabi

      - name: Install uv
        uses: astral-sh/setup-uv@v5

      - name: Build Firmware
        run: |
          # Скопировать конфигурацию клавиатуры пользователя в дерево DMK
          mkdir -p dmk/keyboards
          cp -r user_config/keyboards/* dmk/keyboards/

          cd dmk
          # Замените 'my_keyboard' и 'rp2040' на имя вашей клавиатуры и целевой MCU
          ./build_all.sh -b my_keyboard --mcu rp2040 -c --uf2

      - name: Upload Artifacts
        uses: actions/upload-artifact@v4
        with:
          name: dmk-binaries
          path: |
            dmk/build/dmk_*
          if-no-files-found: error
```

Пример шаблона пользовательского репозитория: `https://github.com/aroum/dmk-config-template`.

## Единое окружение uv

Все инструменты (`tools/nrfutil`, `tools/uf2`) делят одну общую среду, настроенную в корневом файле `pyproject.toml`.

Для синхронизации зависимостей выполните:

```bash
uv sync
```

Будет создана одна общая директория `.venv` в корне проекта. Скрипты сборки (`build_all.sh` и `build_all.ps1`) автоматически запускают скрипты через `uv run --project . ...` в этом корневом окружении.

## Автоматическое обнаружение порта для nice!nano

При прошивке nRF52840 вы можете передать аргумент `--nrf-port auto`, чтобы скрипт автоматически нашёл и выбрал последовательный порт, содержащий `nice!nano` в имени или описании. Если такое устройство не будет найдено, скрипт выведет пронумерованный список всех доступных портов для интерактивного выбора.

```bash
./build_all.sh -b kabarga --mcu nrf52840 -cu --nrf-port auto
```
