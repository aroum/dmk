/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Baikal Electronics JSC JSC
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "sync.h"

__attribute__ ((section (".spinlock")))
spin_lock_t _sw_spin_locks[NUM_SPIN_LOCKS];

void spin_locks_reset(void) {
    for (uint32_t i = 0; i < NUM_SPIN_LOCKS; i++) {
        spin_unlock_unsafe(spin_lock_instance(i));
    }
}

spin_lock_t *spin_lock_init(uint32_t lock_num) {
    assert(lock_num < NUM_SPIN_LOCKS);
    spin_lock_t *lock = spin_lock_instance(lock_num);
    spin_unlock_unsafe(lock);
    return lock;
}
