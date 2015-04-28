// eCos memory layout - Wed Jun 27 18:44:55 2012

// This is a generated file - do not edit
// =============================================================================

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>
#endif

// Boot region.
#define CYGMEM_REGION_boot          (0x0)
#define CYGMEM_REGION_boot_SIZE     (0x00100000)
#define CYGMEM_REGION_boot_ATTR     (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

// Internal ROM
#define CYGMEM_REGION_irom          (0x00100000)
#define CYGMEM_REGION_irom_SIZE     (0x8000)
#define CYGMEM_REGION_irom_ATTR     (CYGMEM_REGION_ATTR_R)

// Internal SRAM (OCM) memory.
#define CYGMEM_REGION_sram0         (0xFFFC0000)
#define CYGMEM_REGION_sram0_SIZE    (0x00020000)
#define CYGMEM_REGION_sram0_ATTR    (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

// 1GB DDR3 SDRAM
#define CYGMEM_REGION_sdram         (0x00000000)
#define CYGMEM_REGION_sdram_SIZE    (0x40000000)
#define CYGMEM_REGION_sdram_ATTR    (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

// Map region names to eCos names.
// SDRAM size adjusted for bootloader areas.
#define CYGMEM_REGION_rom       CYGMEM_REGION_flash
#define CYGMEM_REGION_rom_SIZE  CYGMEM_REGION_flash_SIZE
#define CYGMEM_REGION_rom_ATTR  CYGMEM_REGION_flash_ATTR
#define CYGMEM_REGION_ram       (CYGMEM_REGION_sdram + 0x00040000)
#define CYGMEM_REGION_ram_SIZE  (CYGMEM_REGION_sdram_SIZE - 0x0005F000)
#define CYGMEM_REGION_ram_ATTR  CYGMEM_REGION_sdram_ATTR

#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (CYGMEM_REGION_ram + CYGMEM_REGION_ram_SIZE - (size_t) CYG_LABEL_NAME (__heap1))

