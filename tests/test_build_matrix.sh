#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BOLD='\033[1m'
NC='\033[0m'

echo -e "${BOLD}=== DMK Multi-Platform CI Build Matrix Verification ===${NC}\n"

# Array of targets: "keyboard:mcu"
TARGETS=(
    "nizkoteno:rp2040"
    "nizkoteno:rp2350"
    "corne:milandr"
    "corne:nrf52840"
    "corne:baikal"
)

PASSED=0
FAILED=0
FAILED_TARGETS=()

for item in "${TARGETS[@]}"; do
    IFS=":" read -r KBD MCU <<< "$item"
    echo -e "${YELLOW}Testing build for ${KBD} on ${MCU}...${NC}"

    if "${SCRIPT_DIR}/build_all.sh" -c -b "$KBD" --mcu "$MCU" > /dev/null 2>&1; then
        echo -e "  ${GREEN}✔ ${KBD} (${MCU}) build PASSED${NC}"
        ((PASSED++))
    else
        echo -e "  ${RED}✖ ${KBD} (${MCU}) build FAILED${NC}"
        ((FAILED++))
        FAILED_TARGETS+=("$item")
    fi
done

echo -e "\n${BOLD}=== Build Matrix Summary ===${NC}"
echo -e "  Total:  ${#TARGETS[@]}"
echo -e "  Passed: ${GREEN}${PASSED}${NC}"
echo -e "  Failed: ${RED}${FAILED}${NC}"

if [[ $FAILED -gt 0 ]]; then
    echo -e "\n${RED}Failed targets: ${FAILED_TARGETS[*]}${NC}"
    exit 1
else
    echo -e "\n${GREEN}🎉 All platform builds passed!${NC}"
    exit 0
fi
