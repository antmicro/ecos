// eCos memory layout - Wed Jun 27 18:44:55 2012

// =============================================================================

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>

#endif
#define CYGMEM_REGION_boot          (0x0)
#define CYGMEM_REGION_boot_SIZE     (0x4000)
#define CYGMEM_REGION_boot_ATTR     (CYGMEM_REGION_ATTR_R)
#define CYGMEM_REGION_sram0         (0xFFFC0000)  //sram start
#define CYGMEM_REGION_sram0_SIZE    (0x00040000)  //256K
#define CYGMEM_REGION_sram0_ATTR    (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)
#define CYGMEM_REGION_sdram         (0x00000000)
#define CYGMEM_REGION_sdram_SIZE    (0x40000000) // 1 GB DDR3 SDRAM memory
#define CYGMEM_REGION_sdram_ATTR    (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

#define CYGMEM_REGION_rom       CYGMEM_REGION_boot
#define CYGMEM_REGION_rom_SIZE  CYGMEM_REGION_boot_SIZE
#define CYGMEM_REGION_rom_ATTR  CYGMEM_REGION_boot_ATTR
#define CYGMEM_REGION_ram       CYGMEM_REGION_sdram
#define CYGMEM_REGION_ram_SIZE  CYGMEM_REGION_sdram_SIZE
#define CYGMEM_REGION_ram_ATTR  CYGMEM_REGION_sdram_ATTR

#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (CYGMEM_REGION_sdram + CYGMEM_REGION_sdram_SIZE - (size_t) CYG_LABEL_NAME (__heap1))

