#ifndef CYGONCE_HAL_VAR_INTR_H
#define CYGONCE_HAL_VAR_INTR_H

#include <cyg/hal/hal_smp.h>

#ifdef CYGPKG_HAL_SMP_SUPPORT
__externC cyg_uint32 hal_cpu_get_current(void);
__externC void hal_interrupt_set_cpu_target(cyg_uint32, cyg_uint32);

#define CYGNUM_HAL_SMP_CPU_INTERRUPT_VECTOR(n) (n + 0)

#define HAL_INTERRUPT_SET_CPU(_vector_,_cpu_)  hal_interrupt_set_cpu_target(_vector_,_cpu_)
#define HAL_INTERRUPT_GET_CPU(_vector_,_cpu_)  _cpu_=hal_cpu_get_current()

#endif

#endif // CYGONCE_HAL_VAR_INTR_H
