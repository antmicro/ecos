#ifndef CYGONCE_FPV4_SP_D16_H
#define CYGONCE_FPV4_SP_D16_H
//==========================================================================
//
//      fpv4_sp_d16.h
//
//      FPv4spD16 Floating Point Unit definitions 
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2012 Free Software Foundation, Inc.                        
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):      ilijak
// Contributor(s):
// Date:           2012-04-25
// Description:    FPv4spD16 Floating Point Unit definitions and macros
// Usage:          include <cyg/hal/fpv4_sp_d16.h>
//
//####DESCRIPTIONEND####
//
//========================================================================

#if defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL
#define CYGARC_CORTEXM_FPU_EXC_AUTOSAVE
#endif

//===========================================================================
// Floating-point Context Control Register
#define CYGARC_REG_FPU_FPCCR 0xE000EF34

#define CYGARC_REG_FPU_FPCCR_LSPACT   0x1
#define CYGARC_REG_FPU_FPCCR_USER     0x2
#define CYGARC_REG_FPU_FPCCR_THREAD   0x8
#define CYGARC_REG_FPU_FPCCR_HFRDY    0x10
#define CYGARC_REG_FPU_FPCCR_MMRDY    0x20
#define CYGARC_REG_FPU_FPCCR_BFRDY    0x40
#define CYGARC_REG_FPU_FPCCR_MONRDY   0x100
#define CYGARC_REG_FPU_FPCCR_LSPEN    0x40000000
#define CYGARC_REG_FPU_FPCCR_ASPEN    0x80000000

#define HAL_CORTEXM_FPU_ENABLE()                      \
CYG_MACRO_START                                       \
    cyg_uint32 regval;                                \
    HAL_READ_UINT32(CYGARC_REG_FPU_CPACR, regval);    \
    regval |= CYGARC_REG_FPU_CPACR_ENABLE;            \
    HAL_WRITE_UINT32(CYGARC_REG_FPU_CPACR, regval);   \
    HAL_MEMORY_BARRIER();                             \
CYG_MACRO_END

#define HAL_CORTEXM_FPU_DISABLE()                     \
CYG_MACRO_START                                       \
    cyg_uint32 regval;                                \
    HAL_READ_UINT32(CYGARC_REG_FPU_CPACR, regval);    \
    regval &= ~CYGARC_REG_FPU_CPACR_ENABLE;           \
    HAL_WRITE_UINT32(CYGARC_REG_FPU_CPACR, regval);   \
    HAL_MEMORY_BARRIER();                             \
CYG_MACRO_END

#ifndef __ASSEMBLER__
__externC void hal_init_fpu(void);
#endif

// Floating-point Context Address Register
#define CYGARC_REG_FPU_FPCAR 0xE000EF38

// Floating-point Default Status Control Register
#define CYGARC_REG_FPU_FPDSCR 0xE000EF3C

#define CYGARC_REG_FPU_FPDSCR_FZ            BIT_(24)
#define CYGARC_REG_FPU_FPDSCR_DN            BIT_(25)
#define CYGARC_REG_FPU_FPDSCR_AHP           BIT_(26)

#define CYGARC_REG_FPU_FPDSCR_ROUND(__mode) VALUE_(22, (__mode))
// where __mode is:
#define CYGARC_REG_FPU_FPDSCR_ROUND_RN      0
#define CYGARC_REG_FPU_FPDSCR_ROUND_RP      1
#define CYGARC_REG_FPU_FPDSCR_ROUND_RM      2
#define CYGARC_REG_FPU_FPDSCR_ROUND_RZ      3

//==========================================================================
// FPU Context
#define HAL_SAVEDREGISTERS_WITH_FPU        0x80

#define HAL_SAVEDREGISTERS_THREAD_FPU      (HAL_SAVEDREGISTERS_THREAD | \
                                            HAL_SAVEDREGISTERS_WITH_FPU)

#define HAL_SAVEDREGISTERS_EXCEPTION_FPU   (HAL_SAVEDREGISTERS_EXCEPTION | \
                                            HAL_SAVEDREGISTERS_WITH_FPU)

#ifndef CYGARC_CORTEXM_FPU_EXC_AUTOSAVE

// Without automatic contex saving during exception or interrupt
# define HAL_SAVEDREGISTERS_FPU_THREAD_CONTEXT_SIZE   (HAL_SAVEDREG_THREAD_FPU_N*4+4)
# define HAL_SAVEDREG_AUTO_FRAME_SIZE                 (8*4)

# define HAL_SAVEDREG_AUTO_FPU_EXCEPTION_S

#else //  !CYGARC_CORTEXM_FPU_EXC_AUTOSAVE

// With automatic contex saving during exception or interrupt enabled
# if defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL

#  define HAL_SAVEDREG_AUTO_EXCEPTION_FPU_N    16
#  define HAL_SAVEDREG_AUTO_FRAME_SIZE         (8*4 + 16*4 + 4 + 4)

// HAL_SavedRegisters entries for floating point registers
//     see hal_arch.h for HAL_SavedRegisters definition.

#  define HAL_SAVEDREG_AUTO_FPU_EXCEPTION_S                        \
            cyg_uint32  s_auto[HAL_SAVEDREG_AUTO_EXCEPTION_FPU_N]; \
            cyg_uint32  fpscr_auto;                                \
            cyg_uint32  aligner

# else // defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL
#  error  "Automatic FPU context saving is not supported in LAZY and NONE modes."
# endif // defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL
#endif //  !CYGARC_CORTEXM_FPU_EXC_AUTOSAVE

// Common for AUTOSAVE and non AUTOSAVE
#if defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

// HAL_SavedRegisters entries for floating point registers
//     see hal_arch.h for HAL_SavedRegisters definition.

# define HAL_SAVEDREG_THREAD_FPU_N         32
# define HAL_SAVEDREG_EXCEPTION_FPU_N      32

# define HAL_SAVEDREG_FPU_THREAD_S                      \
           cyg_uint32  fpscr;                           \
           cyg_uint32  s[HAL_SAVEDREG_THREAD_FPU_N]

# define HAL_SAVEDREG_FPU_EXCEPTION_S                   \
           cyg_uint32  s[HAL_SAVEDREG_EXCEPTION_FPU_N]; \
           cyg_uint32  fpscr;                           \
           cyg_uint32  cpacr

// Thread FP context initialization
# define HAL_THREAD_INIT_FPU_REGS(__regs_p)                                 \
CYG_MACRO_START                                                             \
    int __reg_i;                                                            \
    for(__reg_i = 0; __reg_i < HAL_SAVEDREG_THREAD_FPU_N; __reg_i++)    \
        (__regs_p)->u.thread.s[__reg_i] = 0;                                \
CYG_MACRO_END

# define HAL_THREAD_INIT_FPU_CONTEXT(__regs_p)                              \
CYG_MACRO_START                                                             \
    HAL_THREAD_INIT_FPU_REGS(__regs_p);                                     \
    (__regs_p)->u.thread.fpscr = 0;                                         \
CYG_MACRO_END
#else //defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

#  define HAL_SAVEDREG_FPU_THREAD_S
#  define HAL_THREAD_INIT_FPU_CONTEXT(__regs) CYG_EMPTY_STATEMENT

#endif //defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY
//==========================================================================
// hal_arch.h GDB stub support

// Register layout expected by GDB VFP
#ifndef __ASSEMBLER__
typedef struct {
    cyg_uint32 gpr[16];
    cyg_uint32 xpsr;
    cyg_uint32 s[32];
    cyg_uint32 fpscr;
} HAL_CORTEXM_GDB_Registers;
#endif

#if defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

# define GDB_STUB_SAVEDREG_FRAME_TYPE(__regs)                        \
         ((__regs)->u.type & ~HAL_SAVEDREGISTERS_WITH_FPU)

# define GDB_STUB_SAVEDREG_FPU_THREAD_GET(__gdbreg,__regs)           \
CYG_MACRO_START                                                      \
    cyg_uint32 reg_i;                                                \
    for( reg_i = 0; reg_i < HAL_SAVEDREG_THREAD_FPU_N; reg_i++ ) \
        (__gdbreg)->s[reg_i] = (__regs)->u.thread.s[reg_i];          \
    (__gdbreg)->fpscr = (__regs)->u.thread.fpscr;                    \
CYG_MACRO_END

# define GDB_STUB_SAVEDREG_FPU_THREAD_SET(__gdbreg,__regs)           \
CYG_MACRO_START                                                      \
    cyg_uint32 reg_i;                                                \
    for( reg_i = 0; reg_i < HAL_SAVEDREG_THREAD_FPU_N; reg_i++ ) \
        (__regs)->u.thread.s[reg_i] = (__gdbreg)->s[reg_i];          \
    (__regs)->u.thread.fpscr = (__gdbreg)->fpscr;                    \
CYG_MACRO_END

#else //  defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

# define GDB_STUB_SAVEDREG_FRAME_TYPE(__regs)              ((__regs)->u.type)
# define GDB_STUB_SAVEDREG_FPU_THREAD_GET(__gdbreg,__regs) CYG_EMPTY_STATEMENT
# define GDB_STUB_SAVEDREG_FPU_THREAD_SET(__gdbreg,__regs) CYG_EMPTY_STATEMENT

#endif //  defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_ALL || defined CYGHWR_HAL_CORTEXM_FPU_SWITCH_LAZY

#define GDB_STUB_SAVEDREG_FPU_EXCEPTION_GET(__gdbreg,__regs)        \
CYG_MACRO_START                                                     \
    cyg_uint32 reg_i;                                               \
    for( reg_i = 0; reg_i < HAL_SAVEDREG_EXCEPTION_FPU_N; reg_i++ ) \
        (__gdbreg)->s[reg_i] = (__regs)->u.exception.s[reg_i];      \
    (__gdbreg)->fpscr = (__regs)->u.exception.fpscr;                \
CYG_MACRO_END

#define GDB_STUB_SAVEDREG_FPU_EXCEPTION_SET(__gdbreg,__regs)                      \
CYG_MACRO_START                                                                   \
    cyg_uint32 reg_i;                                                             \
    for( reg_i = 0; reg_i < HAL_SAVEDREG_EXCEPTION_FPU_N; reg_i++ )               \
        (__regs)->u.exception.s[reg_i] = (__gdbreg)->s[reg_i];                    \
    (__regs)->u.exception.fpscr = (__gdbreg)->fpscr;                              \
    if(*(cyg_uint32 *)CYGARC_REG_FPU_FPCCR & CYGARC_REG_FPU_FPCCR_ASPEN) {        \
        for( reg_i = 0; reg_i < HAL_SAVEDREG_AUTO_EXCEPTION_FPU_N; reg_i++ )      \
            (__regs)->u.exception.s_auto[reg_i] = (__regs)->u.exception.s[reg_i]; \
        (__regs)->u.exception.fpscr_auto = (__regs)->u.exception.fpscr;           \
    }                                                                             \
CYG_MACRO_END

//==========================================================================
// hal_arch.h Minimal and sensible stack sizes:
// Override value in hal_arch.h
#define CYGNUM_HAL_STACK_CONTEXT_SIZE (4 * (20+32+4+4))

// GDB stub ==================================================================
// cortexm_stub.h definitions for FPV4-SP-D16

// The Cortex-M4F double registers are larger then target_register_t.
#define TARGET_HAS_LARGE_REGISTERS

// Cortex-M4F stub register handling macros
#define CYGARC_STUB_REGISTER_ACCESS_DEFINED 1
#define NUMREGS    (FPSCR+1)  // 16 GPR, XPSR, 10 non existent, 16 VFP, FPSCR
#define REGSIZE( _x_ ) (_x_ <= PC ? 4 :                         \
                        (_x_ < XPSR ? 0 :                        \
                         (_x_ == XPSR ? 4 :                      \
                          (((_x_ >= VD0) && (_x_ <= VD15)) ? 8 : \
                           (_x_ == FPSCR ? 4 : 0 )))))
#ifndef __ASSEMBLER__
# ifndef TARGET_REGISTER_T_DEFINED
#  define TARGET_REGISTER_T_DEFINED
typedef unsigned long target_register_t;
# endif

enum regnames {
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, SP, LR, PC,
    XPSR = 25,
    VD0 = 26, VD1, VD2, VD3, VD4, VD5, VD6, VD7,
    VD8, VD9, VD10, VD11, VD12, VD13, VD14, VD15,
    FPSCR
};
#endif // __ASSEMBLER__

//==========================================================================
#endif //CYGONCE_FPV4_SP_D16_H
