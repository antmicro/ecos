    .code 32
    .section ".text","ax"

    .global zynq_secondary_trampoline
zynq_secondary_trampoline:
    ldr r2, zynq_secondary_trampoline_jump
    bx r2
    .global zynq_secondary_trampoline_jump
zynq_secondary_trampoline_jump:
    .word

    .global hal_cpu_get_current
    @ int hal_cpu_get_current(void)@
    @ get current CPU ID
    .func hal_cpu_get_current
hal_cpu_get_current:
    mrc   p15, 0, r0, c0, c0, 5
    and   r0, r0, #3
    bx    lr
  .endfunc    @cpu_get_current()@

#define UNLOCKED 0xFF

// void hal_spinlock_lock(spinlock_t * lock, cyg_uint32 timeout)
    .global hal_spinlock_lock
    .func hal_spinlock_lock
hal_spinlock_lock:
    stmfd   sp!,{r2}
hal_spinlock_retry:
    ldrex   r1, [r0]
    cmp     r1, #UNLOCKED           // check if spinlock currently unlocked
    wfene                           // wait for an event signal
    bne     hal_spinlock_retry

    mrc     p15, 0, r1, c0, c0, 5   // get our CPU ID
    and     r1, r1, #3
    strex   r2, r1, [r0]            // attempt to grab lock by writing CPU number into spinlock
    cmp     r2, #0                  // check if the write was successful
    bne     hal_spinlock_retry      // if the write failed, start over

    dmb                             // Ensure that accesses to shared resource have completed
                                    // This makes sure the protected data is not touched until
                                    // the lock is taken

    ldmfd   sp!, {r2}
    bx      lr                      // return to caller

    .endfunc // spinlock_lock

    // cyg_bool hal_spinlock_trylock(spinlock_t * lock, cyg_uint32 timeout)
    .global hal_spinlock_trylock
    .func hal_spinlock_trylock
hal_spinlock_trylock:
    stmfd   sp!,{r2}

    ldrex   r1, [r0]
    cmp     r1, #UNLOCKED           // check if spinlock currently unlocked
    movne   r0,#1
    bne     hal_spinlock_trylock_return

    mrc     p15, 0, r1, c0, c0, 5   // get our CPU ID
    and     r1, r1, #3
    strex   r2, r1, [r0]            // attempt to grab lock by writing CPU number into spinlock
    cmp     r2, #0                  // check if the write was successful
    movne   r0,#1
    bne     hal_spinlock_trylock_return

    dmb                             // Ensure that accesses to shared resource have completed
                                    // This makes sure the protected data is not touched until
                                    // the lock is taken
    mov      r0,#0
hal_spinlock_trylock_return:
    ldmfd   sp!, {r2}
    bx      lr                      // return to caller

    .endfunc // spinlock_trylock

// cyg_uint32 hal_spinlock_unlock(spinlock_t * lock)
    .global hal_spinlock_unlock
    .func hal_spinlock_unlock
hal_spinlock_unlock:
    stmfd   sp!,{r1, r2}

    mrc     p15, 0, r1, c0, c0, 5   // get our CPU ID
    and     r1, r1, #3

    ldr     r2, [r0]                // read lock field of spinlock
    cmp     r1, r2                  // compare lock field with our CPU ID
    movne   r0, #1                  // doesn't match, so exit with failure
    bne       hal_spinlock_unlock_return

    mov     r1, #UNLOCKED           // load unlocked value

    dmb                             // Ensure that accesses to shared resource have completed
                                    // This makes sure all data protected by the lock is seen
                                    // by all CPUs before we unlock.
    str     r1, [r0]                // Write into lock field of spinlock
                                    // A simple store is ok because only once CPU at a time will
                                    // release the lock
    dsb                             // Ensure that no instructions following the barrier execute until
                                    // all memory accesses prior to the barrier have completed.
                                    // This prevents any instructions from executing until the lock
                                    // is released, thus preventing the event from firing to soon.

    sev                             // send event to wake up other cores waiting on spinlock

    mov     r0, #0                  // return success
hal_spinlock_unlock_return:
    ldmfd   sp!, {r1, r2}
    bx      lr

    .endfunc

// ------------------------
    .global hal_scu_enable
    @ void hal_scu_enable(void)
    @ Enables the SCU
    .func hal_scu_enable
hal_scu_enable:

    mrc     p15, 4, r0, c15, c0, 0  @ Read periph base address

    ldr     r1, [r0, #0x0]          @ Read the SCU Control Register
    orr     r1, r1, #0x1            @ Set bit 0 (The Enable bit)
    str     r1, [r0, #0x0]          @ Write back modifed value

    bx      lr
    .endfunc

    .global  hal_scu_join_smp
    @ void hal_scu_join_smp(void)
    @ Set this CPU as participating in SMP
    .func hal_scu_join_smp
hal_scu_join_smp:

    @ SMP status is controlled by bit 6 of the CP15 Aux Ctrl Reg

    mrc     p15, 0, r0, c1, c0, 1   @ Read ACTLR
    orr     r0, r0, #0x040          @ Set bit 6
    mcr     p15, 0, r0, c1, c0, 1   @ Write ACTLR

    bx      lr
    .endfunc

    .end
