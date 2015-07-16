// eCos memory layout

#ifndef __ASSEMBLER__
#include <cyg/infra/cyg_type.h>
#include <stddef.h>
#endif

#define CYGMEM_REGION_sram (0x0f000000)
#define CYGMEM_REGION_sram_SIZE (0x9FFFF0) //10MB
#define CYGMEM_REGION_sram_ATTR (CYGMEM_REGION_ATTR_R | CYGMEM_REGION_ATTR_W)

#define CYGMEM_REGION_ram (CYGMEM_REGION_sram)
#define CYGMEM_REGION_ram_SIZE (CYGMEM_REGION_sram_SIZE)
#define CYGMEM_REGION_ram_ATTR (CYGMEM_REGION_sram_ATTR)


#ifndef __ASSEMBLER__
extern char CYG_LABEL_NAME (__heap1) [];
#endif
#define CYGMEM_SECTION_heap1 (CYG_LABEL_NAME (__heap1))
#define CYGMEM_SECTION_heap1_SIZE (CYGMEM_REGION_ram+CYGMEM_REGION_ram_SIZE - (size_t) CYG_LABEL_NAME (__heap1))
