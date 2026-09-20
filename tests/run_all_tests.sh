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
