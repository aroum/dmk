/*
 * Copyright (c) 2024 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2025 Baikal Electronics JSC JSC
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _HARDWARE_SYNC_SPIN_LOCK_H
#define _HARDWARE_SYNC_SPIN_LOCK_H

#include <assert.h>
#include <stdbool.h>

#include "bmcu_common.h"

#define NUM_SPIN_LOCKS 32u

#ifndef SW_SPIN_LOCK_TYPE
// Byte flag in memory:
#define SW_SPIN_LOCK_TYPE volatile uint8_t
#endif
typedef SW_SPIN_LOCK_TYPE spin_lock_t;

__STATIC_FORCEINLINE void __compiler_memory_barrier(void) {
    __ASM volatile ("" : : : "memory");
}

__STATIC_FORCEINLINE uint32_t save_and_disable_interrupts(void) {
    uint32_t status;
    __ASM volatile (
        "csrrci %0, mstatus, 0x8\n"
        : "=r" (status) :: "memory"
    );
    return status;
}

__STATIC_FORCEINLINE void restore_interrupts_from_disabled(uint32_t status) {
    // on RISC-V this can enable interrupts, but not disable interrupts... which
    // is the common case and doesn't require a branch
    __compiler_memory_barrier();
    CSR_SET( CSR_MSTATUS, status & CSR_MSTATUS_MIE_Msk );
    __compiler_memory_barrier();
}

__STATIC_FORCEINLINE void __mem_fence_acquire(void) {
    __compiler_memory_barrier();
}

__STATIC_FORCEINLINE void __mem_fence_release(void) {
    __compiler_memory_barrier();
}

#ifndef SW_SPIN_LOCK_INSTANCE
#define SW_SPIN_LOCK_INSTANCE(lock_num) ({             \
    extern spin_lock_t _sw_spin_locks[NUM_SPIN_LOCKS]; \
    &_sw_spin_locks[lock_num];                         \
    })
#endif

#ifndef SW_SPIN_LOCK_NUM
#define SW_SPIN_LOCK_NUM(lock) ({                          \
        extern spin_lock_t _sw_spin_locks[NUM_SPIN_LOCKS]; \
        (lock) - _sw_spin_locks;                           \
        })
#endif

#ifndef SW_SPIN_LOCK_IS_LOCKED
#define SW_SPIN_LOCK_IS_LOCKED(lock) ((bool) *(lock))
#endif

#ifndef SW_SPIN_LOCK_LOCK
#define SW_SPIN_LOCK_LOCK(lock) ({                                              \
    uint32_t _tmp0, _tmp1;                                                      \
    __ASM volatile (                                                            \
        /* Get word address, and bit mask for LSB of the */                     \
        /* correct byte within that word -- note shamt is modulo xlen: */       \
        "slli %1, %0, 3\n"                                                      \
        "bset %1, zero, %1\n"                                                   \
        "andi %0, %0, -4\n"                                                     \
        /* Repeatedly set the bit until we see that it was clear at the */      \
        /* point we set it. A set from 0 -> 1 is a successful lock take. */     \
    "1:"                                                                        \
        "amoor.w.aq %2, %1, (%0)\n"                                             \
        "and %2, %2, %1\n"                                                      \
        "bnez %2, 1b\n"                                                         \
        : "+r" (lock), "=r" (_tmp0), "=r" (_tmp1)                               \
    );                                                                          \
    __mem_fence_acquire();                                                      \
    })
#endif

#ifndef SW_SPIN_TRY_LOCK
#define SW_SPIN_TRY_LOCK(lock) ({                                               \
    uint32_t _tmp0;                                                             \
    __ASM volatile (                                                            \
        /* Get word address, and bit mask for LSB of the */                     \
        /* correct byte within that word -- note shamt is modulo xlen: */       \
        "slli %1, %0, 3\n"                                                      \
        "bset %1, zero, %1\n"                                                   \
        "andi %0, %0, -4\n"                                                     \
        /* Set the bit. If it was clear at the point we set it, then we took */ \
        /* the lock. Otherwise the lock was already held, and we give up. */    \
        "amoor.w.aq %0, %1, (%0)\n"                                             \
        "and %1, %1, %0\n"                                                      \
        : "+r" (lock), "=r" (_tmp0)                                             \
    );                                                                          \
    __mem_fence_acquire();                                                      \
    !_tmp0;                                                                     \
    })
#endif

#ifndef SW_SPIN_LOCK_UNLOCK
#define SW_SPIN_LOCK_UNLOCK(lock) ({                              \
    __mem_fence_release();                                        \
    *(lock) = 0; /* write to spinlock register (release lock) */  \
    })
#endif

__STATIC_FORCEINLINE spin_lock_t *spin_lock_instance(uint32_t lock_num) {
    assert(lock_num < NUM_SPIN_LOCKS);
    return SW_SPIN_LOCK_INSTANCE(lock_num);
}

__STATIC_FORCEINLINE uint32_t spin_lock_get_num(spin_lock_t *lock) {
    uint32_t lock_num = SW_SPIN_LOCK_NUM(lock);
    assert(lock_num < NUM_SPIN_LOCKS);
    return lock_num;
}

/*! \brief Acquire a spin lock without disabling interrupts (hence unsafe)
 *
 * \param lock Spinlock instance
 */
__STATIC_FORCEINLINE void spin_lock_unsafe_blocking(spin_lock_t *lock) {
    // Note we don't do a wfe or anything, because by convention these spin_locks are VERY SHORT LIVED and NEVER BLOCK and run
    // with INTERRUPTS disabled (to ensure that)... therefore nothing on our core could be blocking us, so we just need to wait on another core
    // anyway which should be finished soon
    SW_SPIN_LOCK_LOCK(lock);
}

__STATIC_FORCEINLINE bool spin_try_lock_unsafe(spin_lock_t *lock) {
    return SW_SPIN_TRY_LOCK(lock);
}
/*! \brief Release a spin lock without re-enabling interrupts
 *
 * \param lock Spinlock instance
 */
__STATIC_FORCEINLINE void spin_unlock_unsafe(spin_lock_t *lock) {
    SW_SPIN_LOCK_UNLOCK(lock);
}

/*! \brief Acquire a spin lock safely
 *
 * This function will disable interrupts prior to acquiring the spinlock
 *
 * \param lock Spinlock instance
 * \return interrupt status to be used when unlocking, to restore to original state
 */
__STATIC_FORCEINLINE uint32_t spin_lock_blocking(spin_lock_t *lock) {
    uint32_t save = save_and_disable_interrupts();
    spin_lock_unsafe_blocking(lock);
    return save;
}

/*! \brief Check to see if a spinlock is currently acquired elsewhere.
 *
 * \param lock Spinlock instance
 */
__STATIC_FORCEINLINE bool is_spin_locked(spin_lock_t *lock) {
    return SW_SPIN_LOCK_IS_LOCKED(lock);
}

/*! \brief Release a spin lock safely
 *
 * This function will re-enable interrupts according to the parameters.
 *
 * \param lock Spinlock instance
 * \param saved_irq Return value from the \ref spin_lock_blocking() function.
 *
 * \sa spin_lock_blocking()
 */
__STATIC_FORCEINLINE void spin_unlock(spin_lock_t *lock, uint32_t saved_irq) {
    spin_unlock_unsafe(lock);
    restore_interrupts_from_disabled(saved_irq);
}

/*! \brief Initialise a spin lock
 *
 * The spin lock is initially unlocked
 *
 * \param lock_num The spin lock number
 * \return The spin lock instance
 */
spin_lock_t *spin_lock_init(uint32_t lock_num);

/*! \brief Release all spin locks
 */
void spin_locks_reset(void);

#endif
