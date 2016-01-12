#if !defined(__SPINLOCK_H__)
#define __SPINLOCK_H__

#ifdef CYGPKG_HAL_SMP_SUPPORT
//! @brief Typedef for a spinlock.
typedef struct _spinlock {
    cyg_uint32 owner;     // CPU
    cyg_uint32 _cacheLineFiller[7]; // Padding to make the lock consume a full cache line (32 bytes).
} spinlock_t __attribute__ ((aligned (32)));

enum
{
    Locked = 0x00,
    Unlocked = 0xff
};

enum spinlock_timeouts
{
    SpinlockNoWait = 0,
    SpinlockWaitForever = 0xffffffff
};

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////

#if defined(__cplusplus)
extern "C" {
#endif
void hal_spinlock_lock(spinlock_t * lock, cyg_uint32 timeout);
cyg_bool hal_spinlock_trylock(spinlock_t * lock, cyg_uint32 timeout);
cyg_bool hal_spinlock_unlock(spinlock_t * lock);
void spinlock_spin(spinlock_t *lock);
void spinlock_clear(spinlock_t *lock);
void spinlock_init_clear(spinlock_t *lock);
void spinlock_init_set(spinlock_t *lock);
cyg_bool hal_spinlock_try(spinlock_t *lock);
void hal_spinlock_init(spinlock_t *lock);
cyg_bool hal_spinlock_is_locked(spinlock_t *lock);

#if defined(__cplusplus)
}
#endif

#endif
#endif // __SPINLOCK_H__
////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////
