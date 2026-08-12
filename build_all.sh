#!/bin/bash
set -euo pipefail

# --- LANGUAGE CONFIG ---
CURRENT_LANG="en"
# -----------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

CLEAN=false
FLASH=false
UF2=false
USB_FLASH=false
KEYBOARD="corne"
MCU="milandr"
PROBE="j-link"
MEMORY=""
NRF_PORT=""
DEFINE=""

# --- LOCALIZATION STRINGS ---
if [[ "$CURRENT_LANG" == "en" ]]; then
    MSG_ERR_UNKNOWN_OPT="Unknown option:"
    MSG_ERR_UNEXPECTED="Unexpected argument:"
    MSG_ERR_KBD_NOT_FOUND="Keyboard not found in"
    MSG_AVAIL_KBD="Available keyboards:"
    MSG_CLEAN_DIR="=== Cleaning build directory ==="
    MSG_INIT_CMAKE="=== Initializing CMake ==="
    MSG_BUILD="=== Building"
    MSG_BUILD_SUCCESS="=== Build successful:"
    MSG_BIN="Binary:"
    MSG_UF2_READY_CMAKE="UF2 is ready (generated via CMake):"
    MSG_ERR_ELF2UF2="elf2uf2 not found! Please install pico-sdk."
    MSG_ERR_ELF_NOT_FOUND="ELF not found:"
    MSG_GEN_UF2="=== Generating UF2 ==="
    MSG_UF2_READY="UF2 is ready:"
    MSG_ERR_UF2_NOT_FOUND="UF2 not found:"
    MSG_UF2_UNSUPPORTED="UF2 is not supported for"
    MSG_ERR_PROBE_CONF="Probe config not found:"
    MSG_FLASH_DEBUG="=== Flashing via debugger ==="
    MSG_FLASH_SUCCESS="=== Flash successful ==="
    MSG_ERR_HEX_NOT_FOUND="HEX not found:"
    MSG_ERR_NRF_SCRIPT="Flash script not found:"
    MSG_FLASH_USB_NRF="=== Flashing via USB (adafruit-nrfutil) ==="
    MSG_NRF_DFU_HINT="Make sure the board is in DFU mode (double click Reset button)."
    MSG_FLASH_FAILED="=== Flash failed ==="
    MSG_ERR_PICOTOOL="picotool not found! Please install it."
    MSG_ERR_DFU_UTIL="dfu-util not found! Please install it."
    MSG_FLASH_USB="=== Flashing via USB ==="
    MSG_FLASH_USB_DFU="=== Flashing via USB (dfu-util) ==="
    MSG_MILANDR_DFU_HINT="Make sure the board is in DFU mode (hold PB6/button, press Reset, then release PB6)."
    MSG_PICO_BOOTSEL_ERR="Device not found in BOOTSEL mode or USB-ready mode."
    MSG_PICO_BOOTSEL_ACT="Action Required: Hold BOOTSEL, press Reset, then release BOOTSEL."
    MSG_PICO_PRESS_KEY="Press any key when device is ready..."
    MSG_PICO_CONT="\nContinuing..."
    MSG_PICO_UPLOAD="Uploading"
    MSG_PICO_SUCCESS="Flash Successful! Program started."
    MSG_PICO_FAIL="Flash failed! Check connection."
    MSG_USB_UNSUPPORTED="USB-flash is not supported for"
else
    MSG_ERR_UNKNOWN_OPT="Неизвестная опция:"
    MSG_ERR_UNEXPECTED="Неожиданный аргумент:"
    MSG_ERR_KBD_NOT_FOUND="Клавиатура не найдена в"
    MSG_AVAIL_KBD="Доступные клавиатуры:"
    MSG_CLEAN_DIR="=== Очистка директории сборки ==="
    MSG_INIT_CMAKE="=== Инициализация CMake ==="
    MSG_BUILD="=== Сборка"
    MSG_BUILD_SUCCESS="=== Сборка успешна:"
    MSG_BIN="Бинарник:"
    MSG_UF2_READY_CMAKE="UF2 готов (сгенерирован через CMake):"
    MSG_ERR_ELF2UF2="elf2uf2 не найден! Установите pico-sdk."
    MSG_ERR_ELF_NOT_FOUND="ELF не найден:"
    MSG_GEN_UF2="=== Генерация UF2 ==="
    MSG_UF2_READY="UF2 готов:"
    MSG_ERR_UF2_NOT_FOUND="UF2 не найден:"
    MSG_UF2_UNSUPPORTED="UF2 не поддерживается для"
    MSG_ERR_PROBE_CONF="Конфиг отладчика не найден:"
    MSG_FLASH_DEBUG="=== Прошивка через отладчик ==="
    MSG_FLASH_SUCCESS="=== Прошивка успешна ==="
    MSG_ERR_HEX_NOT_FOUND="HEX не найден:"
    MSG_ERR_NRF_SCRIPT="Скрипт прошивки не найден:"
    MSG_FLASH_USB_NRF="=== Прошивка через USB (adafruit-nrfutil) ==="
    MSG_NRF_DFU_HINT="Убедитесь, что плата переведена в режим DFU (двойное нажатие кнопки Reset)."
    MSG_FLASH_FAILED="=== Прошивка не удалась ==="
    MSG_ERR_PICOTOOL="picotool не найден! Установите его."
    MSG_ERR_DFU_UTIL="dfu-util не найден! Установите его."
    MSG_FLASH_USB="=== Прошивка через USB ==="
    MSG_FLASH_USB_DFU="=== Прошивка через USB (dfu-util) ==="
    MSG_MILANDR_DFU_HINT="Убедитесь, что плата переведена в режим DFU (зажав кнопку PB6 при сбросе)."
    MSG_PICO_BOOTSEL_ERR="Устройство не найдено в режиме BOOTSEL или режиме USB."
    MSG_PICO_BOOTSEL_ACT="Необходимое действие: Удерживайте BOOTSEL, нажмите Reset, затем отпустите BOOTSEL."
    MSG_PICO_PRESS_KEY="Нажмите любую клавишу, когда устройство будет готово..."
    MSG_PICO_CONT="\nПродолжаем..."
    MSG_PICO_UPLOAD="Загрузка"
    MSG_PICO_SUCCESS="Прошивка успешна! Программа запущена."
    MSG_PICO_FAIL="Прошивка не удалась! Проверьте подключение."
    MSG_USB_UNSUPPORTED="USB-flash не поддерживается для"
fi
# ----------------------------

# Print RAM/FLASH usage table (same format as Milandr CMake output)
show_memory_usage() {
    local elf="$1" mcu="$2" memory_override="$3"

    command -v arm-none-eabi-size &>/dev/null || return
    [[ -f "$elf" ]] || return

    local size_line
    size_line=$(arm-none-eabi-size "$elf" 2>/dev/null | tail -1)
    local text data bss
    text=$(echo "$size_line" | awk '{print $1}')
    data=$(echo "$size_line" | awk '{print $2}')
    bss=$(echo  "$size_line" | awk '{print $3}')
    [[ -z "$text" || "$text" == "text" ]] && return

    local flash_used=$(( text + data ))
    local ram_used=$(( data + bss ))

    # Default sizes per MCU (flash can be overridden with --memory)
    local flash_size ram_size
    case "$mcu" in
        rp2040)   flash_size=$(( 2 * 1024 * 1024 )); ram_size=$(( 264 * 1024 )) ;;
        rp2350)   flash_size=$(( 4 * 1024 * 1024 )); ram_size=$(( 520 * 1024 )) ;;
        nrf52840) flash_size=$(( 1024 * 1024 ));     ram_size=$(( 256 * 1024 )) ;;
        baikal)   flash_size=$(( 1024 * 1024 ));     ram_size=$(( 256 * 1024 )) ;;
        *)        return ;; # milandr already printed by CMake
    esac

    # Override flash size if --memory was given (e.g. "2MB", "512KB")
    if [[ -n "$memory_override" ]]; then
        local num unit
        num=$(echo "$memory_override" | grep -oE '[0-9]+')
        unit=$(echo "$memory_override" | grep -oiE '[a-z]+')
        case "${unit^^}" in
            KB) flash_size=$(( num * 1024 )) ;;
            MB) flash_size=$(( num * 1024 * 1024 )) ;;
        esac
    fi

    # Format helper: bytes -> human readable
    fmt_size() {
        local b="$1"
        if (( b >= 1048576 )); then
            echo "$(( b / 1048576 )) MB"
        elif (( b >= 1024 )); then
            echo "$(( b / 1024 )) KB"
        else
            echo "${b} B"
        fi
    }

    local flash_pct=$(( flash_used * 10000 / flash_size ))
    local ram_pct=$(( ram_used * 10000 / ram_size ))
    local flash_pct_fmt="$(( flash_pct / 100 )).$(printf '%02d' $(( flash_pct % 100 )) )%"
    local ram_pct_fmt="$(( ram_pct / 100 )).$(printf '%02d' $(( ram_pct % 100 )) )%"

    printf "Memory region         Used Size  Region Size  %%age Used\n"
    printf "           FLASH: %11s  %11s  %9s\n" "${flash_used} B" "$(fmt_size $flash_size)" "$flash_pct_fmt"
    printf "             RAM: %11s  %11s  %9s\n" "${ram_used} B"   "$(fmt_size $ram_size)"   "$ram_pct_fmt"
}

show_help() {
    if [[ "$CURRENT_LANG" == "en" ]]; then
        echo "Usage: ./build_all.sh [OPTIONS]"
        echo ""
        echo "Main parameters:"
        echo "  -b, --keyboard [NAME]    Keyboard selection (default: corne)"
        echo "  --mcu [MCU]              MCU selection (milandr/rp2040/rp2350/nrf52840/baikal, default: milandr)"
        echo "  -p, --probe [PROBE]      Debugger selection (j-link, default: j-link)"
        echo "  --memory [SIZE]          Memory size selection (e.g., 256KB, 512KB, 2MB, 4MB, 16MB)"
        echo "  --nrf-port [PORT]        Port for flashing nRF52840 (e.g., /dev/tty.usbmodemXXX)"
        echo "  --lang [LANG]            Set script language (ru or en) and save it"
        echo ""
        echo "Combined flags:"
        echo "  -c, --clean              Perform a clean build"
        echo "  -f, --flash              Flash via debugger"
        echo "  -u, --usb-flash          Flash via USB (for RP2040/RP2350/nRF52840/milandr)"
        echo "  --uf2                    Generate UF2 file (for RP2040/RP2350/nRF52840)"
        echo "  -h, --help               Show this help"
        echo ""
        echo "Examples:"
        echo "  ./build_all.sh -b corne -cf"
        echo "  ./build_all.sh -b corne -cu --uf2 --mcu rp2040"
        echo "  ./build_all.sh -b corne -cf --memory 512KB"
        echo "  ./build_all.sh --lang en"
    else
        echo "Usage: ./build_all.sh [OPTIONS]"
        echo ""
        echo "Основные параметры:"
        echo "  -b, --keyboard [NAME]    Выбор клавиатуры (по умолчанию: corne)"
        echo "  --mcu [MCU]              Выбор микроконтроллера (milandr/rp2040/rp2350/nrf52840/baikal, по умолчанию: milandr)"
        echo "  -p, --probe [PROBE]      Выбор отладчика (j-link, по умолчанию: j-link)"
        echo "  --memory [SIZE]          Выбор размера памяти (e.g., 256KB, 512KB, 2MB, 4MB, 16MB)"
        echo "  --nrf-port [PORT]        Порт для прошивки nRF52840 (например /dev/tty.usbmodemXXX)"
        echo "  --lang [LANG]            Установить язык скрипта (ru или en) и сохранить"
        echo ""
        echo "Комбинированные флаги:"
        echo "  -c, --clean              Выполнить чистую сборку"
        echo "  -f, --flash              Прошить через отладчик"
        echo "  -u, --usb-flash          Прошить через USB (для RP2040/RP2350/nRF52840/milandr)"
        echo "  --uf2                    Сгенерировать UF2-файл (для RP2040/RP2350/nRF52840)"
        echo "  -h, --help               Показать справку"
        echo ""
        echo "Примеры:"
        echo "  ./build_all.sh -b corne -cf"
        echo "  ./build_all.sh -b corne -cu --uf2 --mcu rp2040"
        echo "  ./build_all.sh -b corne -cf --memory 512KB"
        echo "  ./build_all.sh --lang ru"
    fi
}

parse_short_opts() {
    local flags="${1#-}"
    local i c
    for ((i = 0; i < ${#flags}; i++)); do
        c="${flags:$i:1}"
        case "$c" in
        c) CLEAN=true ;;
            f) FLASH=true ;;
            u) USB_FLASH=true ;;
            s) ;; # silent / skip flash
            h) show_help; exit 0 ;;
            *)
                echo -e "${RED}${MSG_ERR_UNKNOWN_OPT} -${c}${NC}"
                show_help
                exit 1
                ;;
        esac
    done
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --clean) CLEAN=true; shift ;;
        --flash) FLASH=true; shift ;;
        --usb-flash) USB_FLASH=true; shift ;;
        --uf2) UF2=true; shift ;;
        -s|--silent) shift ;;
        --help) show_help; exit 0 ;;
        --lang)
            NEW_LANG="$2"
            if [[ "$NEW_LANG" != "ru" && "$NEW_LANG" != "en" ]]; then
                echo -e "${RED}Invalid language / Неверный язык: $NEW_LANG. Use 'ru' or 'en'.${NC}"
                exit 1
            fi
            # Использование sed с проверкой ОС (macOS требует '' после -i)
            if [[ "$OSTYPE" == darwin* ]]; then
                sed -i '' "s/^CURRENT_LANG=\".*\"/CURRENT_LANG=\"$NEW_LANG\"/" "$0"
            else
                sed -i "s/^CURRENT_LANG=\".*\"/CURRENT_LANG=\"$NEW_LANG\"/" "$0"
            fi
            
            if [[ "$NEW_LANG" == "en" ]]; then
                echo -e "${GREEN}Language successfully changed to English.${NC}"
            else
                echo -e "${GREEN}Язык успешно изменен на русский.${NC}"
            fi
            exit 0
            ;;
        -b|--keyboard)
            KEYBOARD="$2"
            shift 2
            ;;
        --mcu)
            MCU="$2"
            shift 2
            ;;
        -p|--probe)
            PROBE="$2"
            shift 2
            ;;
        --memory)
            MEMORY="$2"
            shift 2
            ;;
        --nrf-port)
            NRF_PORT="$2"
            shift 2
            ;;
        -d|--define)
            DEFINE="$2"
            shift 2
            ;;
        -?*)
            parse_short_opts "$1"
            shift
            ;;
        *)
            echo -e "${RED}${MSG_ERR_UNEXPECTED} $1${NC}"
            show_help
            exit 1
            ;;
    esac
done

# Проверка существования клавиатуры
KEYBOARD_DIR="${SCRIPT_DIR}/keyboards/${KEYBOARD}"
if [[ ! -d "$KEYBOARD_DIR" ]]; then
    echo -e "${RED}${MSG_ERR_KBD_NOT_FOUND} ${KEYBOARD_DIR}${NC}"
    echo "${MSG_AVAIL_KBD}"
    ls -1 "${SCRIPT_DIR}/keyboards/"
    exit 1
fi

# Настройка целевого имени
SUFFIX=""
if [[ -n "$DEFINE" ]]; then
    DEFINE_LOWER=$(echo "$DEFINE" | tr '[:upper:]' '[:lower:]')
    if [[ "$DEFINE_LOWER" == *left* ]]; then
        SUFFIX="_left"
    elif [[ "$DEFINE_LOWER" == *right* ]]; then
        SUFFIX="_right"
    else
        CLEAN_DEF=$(echo "$DEFINE_LOWER" | tr -cd 'a-z0-9_')
        [[ -n "$CLEAN_DEF" ]] && SUFFIX="_${CLEAN_DEF}"
    fi
fi
TARGET_NAME="dmk_${KEYBOARD}_${MCU}${SUFFIX}"
BUILD_DIR="${SCRIPT_DIR}/build"
HRD_PROBE="${SCRIPT_DIR}/platforms/milandr/dep/probe/jlink4swd.cfg"

if [[ "$CLEAN" == true ]]; then
    echo -e "${YELLOW}${MSG_CLEAN_DIR}${NC}"
    rm -rf "$BUILD_DIR"
fi

if [[ ! -d "$BUILD_DIR" ]]; then
    mkdir -p "$BUILD_DIR"
fi

echo -e "${YELLOW}${MSG_INIT_CMAKE}${NC}"
CMAKE_ARGS=(-G "Unix Makefiles" -S "$SCRIPT_DIR" -B "$BUILD_DIR")
CMAKE_ARGS+=(-DKEYBOARD="${KEYBOARD}")
CMAKE_ARGS+=(-DMCU="${MCU}")
if [[ "$MCU" == "rp2350" ]]; then
    CMAKE_ARGS+=(-DPICO_PLATFORM=rp2350)
fi
[[ -n "$MEMORY" ]] && CMAKE_ARGS+=(-DMEMORY="${MEMORY}")
[[ -n "$DEFINE" ]] && CMAKE_ARGS+=(-DDEFINE="${DEFINE}")
if [[ "$MCU" == "milandr" && "$USB_FLASH" == true ]]; then
    CMAKE_ARGS+=(-DBOOTLOADER=ON)
fi
cmake "${CMAKE_ARGS[@]}"

echo -e "${YELLOW}${MSG_BUILD} ${TARGET_NAME} ===${NC}"
if [[ "$OSTYPE" == darwin* ]]; then
    JOBS=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
else
    JOBS=$(nproc 2>/dev/null || echo 4)
fi

cmake --build "$BUILD_DIR" -j"$JOBS"

echo -e "${GREEN}${MSG_BUILD_SUCCESS} ${TARGET_NAME} ===${NC}"
echo -e "${MSG_BIN} ${BUILD_DIR}/${TARGET_NAME}"

# Print memory usage for platforms that don't get it from CMake linker output
if [[ "$MCU" != "milandr" ]]; then
    ELF_EARLY="${BUILD_DIR}/${TARGET_NAME}"
    [[ -f "$ELF_EARLY" ]] || ELF_EARLY="${BUILD_DIR}/${TARGET_NAME}.elf"
    show_memory_usage "$ELF_EARLY" "$MCU" "$MEMORY"
fi

ELF_FILE="${BUILD_DIR}/${TARGET_NAME}"
[[ -f "$ELF_FILE" ]] || ELF_FILE="${BUILD_DIR}/${TARGET_NAME}.elf"
BIN_FILE="${BUILD_DIR}/${TARGET_NAME}.bin"
UF2_FILE="${BUILD_DIR}/${TARGET_NAME}.uf2"

# Генерация UF2-файла, если требуется
if [[ "$UF2" == true ]]; then
    case "$MCU" in
        rp2040|rp2350)
            if [[ -f "$UF2_FILE" ]]; then
                echo -e "${GREEN}${MSG_UF2_READY_CMAKE} ${UF2_FILE}${NC}"
            elif ! command -v elf2uf2 &> /dev/null; then
                echo -e "${RED}${MSG_ERR_ELF2UF2}${NC}"
            else
                if [[ ! -f "$ELF_FILE" ]]; then
                    echo -e "${RED}${MSG_ERR_ELF_NOT_FOUND} ${ELF_FILE}${NC}"
                else
                    echo -e "${YELLOW}${MSG_GEN_UF2}${NC}"
                    elf2uf2 "$ELF_FILE" "$UF2_FILE"
                    echo -e "${GREEN}${MSG_UF2_READY} ${UF2_FILE}${NC}"
                fi
            fi
            ;;
        nrf52840)
            if [[ -f "$UF2_FILE" ]]; then
                echo -e "${GREEN}${MSG_UF2_READY_CMAKE} ${UF2_FILE}${NC}"
            else
                echo -e "${RED}${MSG_ERR_UF2_NOT_FOUND} ${UF2_FILE}${NC}"
            fi
            ;;
        *)
            echo -e "${YELLOW}${MSG_UF2_UNSUPPORTED} ${MCU}.${NC}"
            ;;
    esac
fi

# Прошивка через отладчик (OpenOCD)
if [[ "$FLASH" == true ]]; then
    if [[ ! -f "$ELF_FILE" ]]; then
        echo -e "${RED}${MSG_ERR_ELF_NOT_FOUND} ${ELF_FILE}${NC}"
        exit 1
    fi

    [[ -f "$BIN_FILE" ]] || arm-none-eabi-objcopy -O binary "$ELF_FILE" "$BIN_FILE"

    if [[ ! -f "$HRD_PROBE" ]]; then
        echo -e "${RED}${MSG_ERR_PROBE_CONF} ${HRD_PROBE}${NC}"
        exit 1
    fi

    echo -e "${YELLOW}${MSG_FLASH_DEBUG} (${TARGET_NAME})${NC}"
    openocd -f "$HRD_PROBE" -c "init" -c "halt" \
        -c "program ${BIN_FILE} verify reset 0x08000000" -c "exit"
    echo -e "${GREEN}${MSG_FLASH_SUCCESS}${NC}"
fi

# Прошивка через USB
if [[ "$USB_FLASH" == true ]]; then
    case "$MCU" in
        nrf52840)
            NRF_FLASH_SCRIPT="${SCRIPT_DIR}/tools/nrfutil/flash_nrf.py"
            HEX_FILE="${BUILD_DIR}/${TARGET_NAME}.hex"

            if [[ ! -f "$HEX_FILE" ]]; then
                echo -e "${RED}${MSG_ERR_HEX_NOT_FOUND} ${HEX_FILE}${NC}"
                exit 1
            fi

            if [[ ! -f "$NRF_FLASH_SCRIPT" ]]; then
                echo -e "${RED}${MSG_ERR_NRF_SCRIPT} ${NRF_FLASH_SCRIPT}${NC}"
                exit 1
            fi

            echo -e "${YELLOW}${MSG_FLASH_USB_NRF} (${TARGET_NAME})${NC}"
            echo -e "${YELLOW}${MSG_NRF_DFU_HINT}${NC}"
            if [[ -n "$NRF_PORT" ]]; then
                uv run --project "${SCRIPT_DIR}" "${NRF_FLASH_SCRIPT}" --hex "$HEX_FILE" --port "$NRF_PORT"
            else
                uv run --project "${SCRIPT_DIR}" "${NRF_FLASH_SCRIPT}" --hex "$HEX_FILE"
            fi
            if [[ $? -ne 0 ]]; then
                echo -e "${RED}${MSG_FLASH_FAILED}${NC}"
                exit 1
            fi
            echo -e "${GREEN}${MSG_FLASH_SUCCESS}${NC}"
            ;;
        rp2040|rp2350)
            if ! command -v picotool &> /dev/null; then
                echo -e "${RED}${MSG_ERR_PICOTOOL}${NC}"
                exit 1
            fi

            [[ -f "$BIN_FILE" ]] || arm-none-eabi-objcopy -O binary "$ELF_FILE" "$BIN_FILE"

            echo -e "${YELLOW}${MSG_FLASH_USB} (${TARGET_NAME})${NC}"
            
            # Try magic reboot or check if already in BOOTSEL
            if ! picotool info > /dev/null 2>&1; then
                echo -e "${RED}${MSG_PICO_BOOTSEL_ERR}${NC}"
                echo -e "${YELLOW}${MSG_PICO_BOOTSEL_ACT}${NC}"
                read -n 1 -s -r -p "${MSG_PICO_PRESS_KEY}"
                echo -e "${MSG_PICO_CONT}"
            fi

            echo "${MSG_PICO_UPLOAD} $ELF_FILE..."
            picotool load "$ELF_FILE" -x
            if [ $? -eq 0 ]; then
                echo -e "${GREEN}${MSG_PICO_SUCCESS}${NC}"
            else
                echo -e "${RED}${MSG_PICO_FAIL}${NC}"
                exit 1
            fi
            ;;
        milandr)
            if ! command -v dfu-util &> /dev/null; then
                echo -e "${RED}${MSG_ERR_DFU_UTIL}${NC}"
                exit 1
            fi

            [[ -f "$BIN_FILE" ]] || arm-none-eabi-objcopy -O binary "$ELF_FILE" "$BIN_FILE"

            echo -e "${YELLOW}${MSG_FLASH_USB_DFU} (${TARGET_NAME})${NC}"
            echo -e "${YELLOW}${MSG_MILANDR_DFU_HINT}${NC}"

            dfu-util -a 0 -d 1209:be92 -D "$BIN_FILE"
            if [[ $? -ne 0 ]]; then
                echo -e "${RED}${MSG_FLASH_FAILED}${NC}"
                exit 1
            fi
            echo -e "${GREEN}${MSG_FLASH_SUCCESS}${NC}"
            ;;
        *)
            echo -e "${RED}${MSG_USB_UNSUPPORTED} ${MCU}.${NC}"
            exit 1
            ;;
    esac
fi
