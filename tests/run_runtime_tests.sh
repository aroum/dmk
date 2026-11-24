#!/usr/bin/env bash
set -euo pipefail
root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
out="$(mktemp -d "${TMPDIR:-/tmp}/dmk-runtime.XXXXXX")"
trap 'rm -rf "$out"' EXIT
flags=(-std=c11 -Wall -Wextra -Werror -O1 -g)
if [[ "${DMK_TEST_SANITIZERS:-0}" == 1 ]]; then
    flags+=(-fsanitize=address,undefined -fno-omit-frame-pointer)
fi
"${CC:-clang}" "${flags[@]}" -I "$root/tests/mocks" -I "$root/dmk_core/include" \
    -I "$root/dmk_core/features" -I "$root/dmk_core/drivers" -I "$root/dmk_core/config" \
    "$root/tests/test_runtime_ownership.c" \
    "$root/dmk_core/features/oneshot.c" -o "$out/runtime"
"$out/runtime"
# Run the actual USB/HT/layer core harness in both transport modes.
for mode in immediate batch; do
    defines=()
    if [[ "$mode" == batch ]]; then defines+=(-DMATRIX_REPORT_BATCH); fi
    "${CC:-clang}" "${flags[@]}" ${defines[@]+"${defines[@]}"} \
        -I "$root/tests" -I "$root/tests/mocks" -I "$root/dmk_core/include" \
        -I "$root/dmk_core/config" -I "$root/dmk_core/features" \
        -I "$root/dmk_core/core" -I "$root/dmk_core/drivers" \
        "$root/tests/test_core.c" -o "$out/core-$mode"
    "$out/core-$mode"
done
