#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

RUN_MATRIX=false
for arg in "$@"; do
    if [[ "$arg" == "--all" || "$arg" == "--matrix" ]]; then
        RUN_MATRIX=true
    fi
done

echo -e "${BOLD}====================================================${NC}"
echo -e "${BOLD}              DMK Comprehensive Test Suite          ${NC}"
echo -e "${BOLD}====================================================${NC}\n"

# 1. C Core Unit Tests
echo -e "${BOLD}[1/3] Running Host-Based C Core Unit Tests...${NC}"
clang -Wall -Wextra \
    -I "${SCRIPT_DIR}/tests" \
    -I "${SCRIPT_DIR}/tests/mocks" \
    -I "${SCRIPT_DIR}/dmk_core/include" \
    -I "${SCRIPT_DIR}/dmk_core/config" \
    -I "${SCRIPT_DIR}/dmk_core/features" \
    -I "${SCRIPT_DIR}/dmk_core/core" \
    -I "${SCRIPT_DIR}/dmk_core/drivers" \
    "${SCRIPT_DIR}/tests/test_core.c" -o "${SCRIPT_DIR}/tests/test_core"
"${SCRIPT_DIR}/tests/test_core"
rm -f "${SCRIPT_DIR}/tests/test_core"
clang -Wall -Wextra -I "${SCRIPT_DIR}/tests/mocks" \
    "${SCRIPT_DIR}/tests/test_memory_hooks.c" "${SCRIPT_DIR}/dmk_core/core/memory.c" \
    -o "${SCRIPT_DIR}/tests/test_memory_hooks"
"${SCRIPT_DIR}/tests/test_memory_hooks"
rm -f "${SCRIPT_DIR}/tests/test_memory_hooks"
clang -Wall -Wextra -I "${SCRIPT_DIR}/dmk_core/include" \
    "${SCRIPT_DIR}/tests/test_bounded_heap.c" -o "${SCRIPT_DIR}/tests/test_bounded_heap"
"${SCRIPT_DIR}/tests/test_bounded_heap"
rm -f "${SCRIPT_DIR}/tests/test_bounded_heap"
for heap_size in 3072 8192; do
    clang -Wall -Wextra -DconfigTOTAL_HEAP_SIZE="$heap_size" -I "${SCRIPT_DIR}/tests/heap_mocks" \
        "${SCRIPT_DIR}/tests/test_heap4_failure.c" "${SCRIPT_DIR}/dmk_core/core/memory.c" \
        "${SCRIPT_DIR}/lib/freertos/heap_4.c" -o "${SCRIPT_DIR}/tests/test_heap4_failure"
    "${SCRIPT_DIR}/tests/test_heap4_failure"
done
rm -f "${SCRIPT_DIR}/tests/test_heap4_failure"
clang -Wall -Wextra -I "${SCRIPT_DIR}/tests/mocks" -I "${SCRIPT_DIR}/dmk_core/include" \
    -I "${SCRIPT_DIR}/dmk_core/config" -I "${SCRIPT_DIR}/dmk_core/drivers" \
    "${SCRIPT_DIR}/tests/test_split_packet.c" -o "${SCRIPT_DIR}/tests/test_split_packet"
"${SCRIPT_DIR}/tests/test_split_packet"
rm -f "${SCRIPT_DIR}/tests/test_split_packet"
clang -Wall -Wextra -DDMK_SPLIT_TEST -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 -DRGB_NUM=3 -DLED_PIN=1 \
    -I "${SCRIPT_DIR}/tests/split_mocks" -I "${SCRIPT_DIR}/tests/mocks" \
    -I "${SCRIPT_DIR}/dmk_core/include" -I "${SCRIPT_DIR}/dmk_core/config" \
    -I "${SCRIPT_DIR}/dmk_core/features" -I "${SCRIPT_DIR}/dmk_core/drivers" \
    "${SCRIPT_DIR}/tests/test_split_protocol.c" "${SCRIPT_DIR}/dmk_core/drivers/split.c" \
    -o "${SCRIPT_DIR}/tests/test_split_protocol"
"${SCRIPT_DIR}/tests/test_split_protocol"
rm -f "${SCRIPT_DIR}/tests/test_split_protocol"
clang -Wall -Wextra -DDMK_SPLIT_TEST -DSPLIT_RIGHT -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 -DRGB_NUM=3 -DLED_PIN=1 \
    -I "${SCRIPT_DIR}/tests/split_mocks" -I "${SCRIPT_DIR}/tests/mocks" \
    -I "${SCRIPT_DIR}/dmk_core/include" -I "${SCRIPT_DIR}/dmk_core/config" \
    -I "${SCRIPT_DIR}/dmk_core/features" -I "${SCRIPT_DIR}/dmk_core/drivers" \
    "${SCRIPT_DIR}/tests/test_split_protocol.c" "${SCRIPT_DIR}/dmk_core/drivers/split.c" \
    -o "${SCRIPT_DIR}/tests/test_split_protocol"
"${SCRIPT_DIR}/tests/test_split_protocol"
rm -f "${SCRIPT_DIR}/tests/test_split_protocol"
clang -Wall -Wextra -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 '-DLED_PINS={1,2,3}' \
    -DLED_DEBUG=0 -DLED_HID_CAPS_LOCK=1 -DLED_ACTIVITY_PIN=0 -I "${SCRIPT_DIR}/tests/split_mocks" \
    -I "${SCRIPT_DIR}/tests/mocks" -I "${SCRIPT_DIR}/dmk_core/include" \
    -I "${SCRIPT_DIR}/dmk_core/config" -I "${SCRIPT_DIR}/dmk_core/features" \
    -I "${SCRIPT_DIR}/dmk_core/drivers" "${SCRIPT_DIR}/tests/test_split_led.c" \
    "${SCRIPT_DIR}/dmk_core/drivers/led.c" -o "${SCRIPT_DIR}/tests/test_split_led"
"${SCRIPT_DIR}/tests/test_split_led"
rm -f "${SCRIPT_DIR}/tests/test_split_led"
for rx_pin in 1 2; do
    clang -Wall -Wextra -DDMK_SPLIT_TEST -DMCU_nrf52840 -DSPLIT_UART_HARDWARE \
        -DSPLIT_TX_PIN=1 -DSPLIT_RX_PIN="$rx_pin" -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 \
        -I "${SCRIPT_DIR}/tests/split_mocks" -I "${SCRIPT_DIR}/tests/mocks" \
        -I "${SCRIPT_DIR}/dmk_core/include" -I "${SCRIPT_DIR}/dmk_core/config" \
        -I "${SCRIPT_DIR}/dmk_core/features" -I "${SCRIPT_DIR}/dmk_core/drivers" \
        "${SCRIPT_DIR}/tests/test_split_nrf.c" "${SCRIPT_DIR}/platforms/nrf52840/split.c" \
        -o "${SCRIPT_DIR}/tests/test_split_nrf"
    "${SCRIPT_DIR}/tests/test_split_nrf"
done
rm -f "${SCRIPT_DIR}/tests/test_split_nrf"
for rx_pin in 1 2; do
    connection=2
    if [[ "$rx_pin" == 2 ]]; then connection=3; fi
    clang -Wall -Wextra -DDMK_SPLIT_TEST -DMCU_rp2040 -DSPLIT_TX_PIN=1 \
        -DSPLIT_RX_PIN="$rx_pin" -DSPLIT_CONNECTION_TYPE="$connection" \
        -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 -I "${SCRIPT_DIR}/tests/split_mocks" \
        -I "${SCRIPT_DIR}/tests/mocks" -I "${SCRIPT_DIR}/dmk_core/include" \
        -I "${SCRIPT_DIR}/dmk_core/config" -I "${SCRIPT_DIR}/dmk_core/features" \
        -I "${SCRIPT_DIR}/dmk_core/drivers" "${SCRIPT_DIR}/tests/test_split_pio.c" \
        "${SCRIPT_DIR}/platforms/raspberrypi/split.c" -o "${SCRIPT_DIR}/tests/test_split_pio"
    "${SCRIPT_DIR}/tests/test_split_pio"
done
rm -f "${SCRIPT_DIR}/tests/test_split_pio"
for matrix_mode in direct row col; do
    if [[ "$matrix_mode" == direct ]]; then
        matrix_flags=(-DMATRIX_TYPE_DIRECT_PIN '-DDIRECT_PINS={0,1,2}')
    elif [[ "$matrix_mode" == row ]]; then
        matrix_flags=(-DMATRIX_TYPE_ROW2COL -DMATRIX_ACTIVE_LOW '-DROW_PINS={3}' '-DCOL_PINS={0,1,2}')
    else
        matrix_flags=(-DTEST_COL2ROW -DMATRIX_TYPE_COL2ROW -DMATRIX_ACTIVE_LOW '-DROW_PINS={0,1}' '-DCOL_PINS={3}')
    fi
    clang -Wall -Wextra -Wno-unused-function "${matrix_flags[@]}" \
        -DNUM_ROWS_SPLIT=2 -DNUM_COLS_SPLIT=3 -I "${SCRIPT_DIR}/tests/split_mocks" \
        -I "${SCRIPT_DIR}/tests/mocks" -I "${SCRIPT_DIR}/dmk_core/include" \
        -I "${SCRIPT_DIR}/dmk_core/config" -I "${SCRIPT_DIR}/dmk_core/features" \
        -I "${SCRIPT_DIR}/dmk_core/drivers" "${SCRIPT_DIR}/tests/test_split_direct_matrix.c" \
        "${SCRIPT_DIR}/dmk_core/drivers/matrix.c" -o "${SCRIPT_DIR}/tests/test_split_matrix"
    "${SCRIPT_DIR}/tests/test_split_matrix"
done
rm -f "${SCRIPT_DIR}/tests/test_split_matrix"
clang -Wall -Wextra -I "${SCRIPT_DIR}/tests/mocks" -I "${SCRIPT_DIR}/dmk_core/include" \
    -I "${SCRIPT_DIR}/dmk_core/config" -I "${SCRIPT_DIR}/dmk_core/drivers" -I "${SCRIPT_DIR}/dmk_core/features" \
    "${SCRIPT_DIR}/tests/test_features.c" "${SCRIPT_DIR}/dmk_core/features/oneshot.c" \
    "${SCRIPT_DIR}/dmk_core/features/macros.c" -o "${SCRIPT_DIR}/tests/test_features"
"${SCRIPT_DIR}/tests/test_features"
rm -f "${SCRIPT_DIR}/tests/test_features"

# 2. Python Tool Tests
echo -e "${BOLD}[2/3] Running Python Tool Unit Tests...${NC}"
uv run python3 -m unittest discover -s "${SCRIPT_DIR}/tests" -p "test_*.py"

# 3. JavaScript Editor / Web Wizard Tests
echo -e "\n${BOLD}[3/3] Running Web Wizard & Editor Tests (Node.js)...${NC}"
node --test "${SCRIPT_DIR}/tests/test_editor.js"

# 4. Optional: Multi-Platform Build Matrix
if [[ "$RUN_MATRIX" == true ]]; then
    echo -e "\n${BOLD}[Optional] Running Multi-Platform CI Build Matrix...${NC}"
    "${SCRIPT_DIR}/tests/test_build_matrix.sh"
fi

echo -e "\n${GREEN}${BOLD}🎉 All test suites completed successfully!${NC}\n"
