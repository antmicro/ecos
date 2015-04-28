#ifndef CYGONCE_VAR_MMU_H
#define CYGONCE_VAR_MMU_H
// -------------------------------------------------------------------------
// MMU initialization:
//
// These structures are laid down in memory to define the translation
// table.
//

/*
 * ARM Domain Access Control Bit Masks
 */
#define ARC_ARM_ACCESS_TYPE_NO_ACCESS(domain_num)    (0x0 << (domain_num)*2)
#define ARC_ARM_ACCESS_TYPE_CLIENT(domain_num)       (0x1 << (domain_num)*2)
#define ARC_ARM_ACCESS_TYPE_MANAGER(domain_num)      (0x3 << (domain_num)*2)

#define ARC_ARM_FIRST_LEVEL_PAGE_TABLE_SIZE          0x4000
#define ARC_ARM_SECOND_LEVEL_PAGE_TABLE_SIZE         0x400
#define ARC_ARM_SECOND_LEVEL_FINE_TABLE_SIZE         0x1000
#define ARC_ARM_SECTION_SIZE                         0x100000
#define ARC_ARM_LARGE_PAGE_SIZE                      0x10000
#define ARC_ARM_SMALL_PAGE_SIZE                      0x1000
#define ARC_ARM_TINY_PAGE_SIZE                       0x400


#define ARC_ARM_MMU_FIRST_LEVEL_FAULT_ID             0x0
#define ARC_ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID        0x1
#define ARC_ARM_MMU_FIRST_LEVEL_SECTION_ID           0x2
#define ARC_ARM_MMU_FIRST_LEVEL_FINE_TABLE_ID        0x3

struct ARC_ARM_MMU_FIRST_LEVEL_FAULT {
    unsigned id : 2;
    unsigned sbz : 30;
};

struct ARC_ARM_MMU_FIRST_LEVEL_PAGE_TABLE {
    unsigned id : 2;
    unsigned sbz0 : 2;
    unsigned imp : 1;
    unsigned domain : 4;
    unsigned sbz1 : 1;
    unsigned base_address : 22;
};

struct ARC_ARM_MMU_FIRST_LEVEL_SECTION {
    unsigned id : 2;
    unsigned b : 1;
    unsigned c : 1;
    unsigned imp : 1;
    unsigned domain : 4;
    unsigned sbz0 : 1;
    unsigned ap : 2;
    unsigned sbz1 : 8;
    unsigned base_address : 12;
};

struct ARC_ARM_MMU_FIRST_LEVEL_FINE_TABLE {
    unsigned id : 2;
    unsigned sbz0 : 2;
    unsigned imp : 1;
    unsigned domain : 4;
    unsigned sbz : 3;
    unsigned base_address : 20;
};

union ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARC_ARM_MMU_FIRST_LEVEL_FAULT fault;
    struct ARC_ARM_MMU_FIRST_LEVEL_PAGE_TABLE page_table;
    struct ARC_ARM_MMU_FIRST_LEVEL_SECTION section;
    struct ARC_ARM_MMU_FIRST_LEVEL_FINE_TABLE fine_table;
};

#define ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS(ttb_base, table_index) \
   (unsigned long *)((unsigned long)(ttb_base) + ((table_index) << 2))


#define ARC_ARM_MMU_SECOND_LEVEL_FAULT_ID            0x0
#define ARC_ARM_MMU_SECOND_LEVEL_LARGE_PAGE_ID       0x1
#define ARC_ARM_MMU_SECOND_LEVEL_SMALL_PAGE_ID       0x2
#define ARC_ARM_MMU_SECOND_LEVEL_TINY_PAGE_ID        0x3

struct ARC_ARM_MMU_SECOND_LEVEL_FAULT {
    unsigned id : 2;
    unsigned sbz : 30;
};

struct ARC_ARM_MMU_SECOND_LEVEL_LARGE_PAGE {
    unsigned id : 2;
    unsigned b : 1;
    unsigned c : 1;
    unsigned ap0 : 2;
    unsigned ap1 : 2;
    unsigned ap2 : 2;
    unsigned ap3 : 2;
    unsigned sbz : 4;
    unsigned base_address : 16;
};

struct ARC_ARM_MMU_SECOND_LEVEL_SMALL_PAGE {
    unsigned id : 2;
    unsigned b : 1;
    unsigned c : 1;
    unsigned ap0 : 2;
    unsigned ap1 : 2;
    unsigned ap2 : 2;
    unsigned ap3 : 2;
    unsigned base_address : 20;
};

struct ARC_ARM_MMU_SECOND_LEVEL_TINY_PAGE {
    unsigned id : 2;
    unsigned b : 1;
    unsigned c : 1;
    unsigned ap : 2;
    unsigned sbz : 4;
    unsigned base_address : 22;
};

union ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR {
    unsigned long word;
    struct ARC_ARM_MMU_SECOND_LEVEL_FAULT fault;
    struct ARC_ARM_MMU_SECOND_LEVEL_LARGE_PAGE large_page;
    struct ARC_ARM_MMU_SECOND_LEVEL_SMALL_PAGE small_page;
    struct ARC_ARM_MMU_SECOND_LEVEL_TINY_PAGE tiny_page;
};

#define ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR_ADDRESS(page_table, table_index) \
   (unsigned long *)((unsigned long)(page_table) + ((table_index) << 2))



#define ARC_ARM_MMU_SECTION(ttb_base, actual_base, virtual_base,            \
                            cacheable, bufferable, perm)                    \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;                 \
    desc.word = 0;                                                          \
    desc.section.id = ARC_ARM_MMU_FIRST_LEVEL_SECTION_ID;                   \
    desc.section.imp = 1;                                                   \
    desc.section.domain = 0;                                                \
    desc.section.c = (cacheable);                                           \
    desc.section.b = (bufferable);                                          \
    desc.section.ap = (perm);                                               \
    desc.section.base_address = (actual_base);							    \
    *ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS((ttb_base), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

#define ARC_ARM_MMU_PAGE_TABLE(ttb_base, actual_base, virtual_base)         \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;                 \
    desc.word = 0;                                                          \
    desc.page_table.id = ARC_ARM_MMU_FIRST_LEVEL_PAGE_TABLE_ID;             \
    desc.page_table.imp = 1;                                                \
    desc.page_table.domain = 0;                                             \
    desc.page_table.base_address = (actual_base);                           \
    *ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS((ttb_base), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

#define ARC_ARM_MMU_FINE_TABLE(ttb_base, actual_base, virtual_base)         \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR desc;                 \
    desc.word = 0;                                                          \
    desc.fine_table.id = ARC_ARM_MMU_FIRST_LEVEL_FINE_TABLE_ID;             \
    desc.fine_table.imp = 1;                                                \
    desc.fine_table.domain = 0;                                             \
    desc.fine_table.base_address = (actual_base);                           \
    *ARC_ARM_MMU_FIRST_LEVEL_DESCRIPTOR_ADDRESS((ttb_base), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

#define ARC_ARM_MMU_LARGE_PAGE(page_table, actual_base, virtual_base,       \
                               cacheable, bufferable, perm0, perm1, perm2, perm3) \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR desc;                \
    desc.word = 0;                                                          \
    desc.large_page.id = ARC_ARM_MMU_SECOND_LEVEL_LARGE_PAGE_ID;            \
    desc.large_page.c = (cacheable);                                        \
    desc.large_page.b = (bufferable);                                       \
    desc.large_page.ap0 = (perm0);                                          \
    desc.large_page.ap1 = (perm1);                                          \
    desc.large_page.ap2 = (perm2);                                          \
    desc.large_page.ap3 = (perm3);                                          \
    desc.large_page.base_address = (actual_base);                           \
    *ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR_ADDRESS((page_table), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

#define ARC_ARM_MMU_SMALL_PAGE(page_table, actual_base, virtual_base,       \
                               cacheable, bufferable, perm0, perm1, perm2, perm3) \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR desc;                \
    desc.word = 0;                                                          \
    desc.small_page.id = ARC_ARM_MMU_SECOND_LEVEL_SMALL_PAGE_ID;            \
    desc.small_page.c = (cacheable);                                        \
    desc.small_page.b = (bufferable);                                       \
    desc.small_page.ap0 = (perm0);                                          \
    desc.small_page.ap1 = (perm1);                                          \
    desc.small_page.ap2 = (perm2);                                          \
    desc.small_page.ap3 = (perm3);                                          \
    desc.small_page.base_address = (actual_base);                           \
    *ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR_ADDRESS((page_table), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

#define ARC_ARM_MMU_TINY_PAGE(page_table, actual_base, virtual_base,        \
                              cacheable, bufferable, perm)                  \
    CYG_MACRO_START                                                         \
    register union ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR desc;                \
    desc.word = 0;                                                          \
    desc.tiny_page.id = ARC_ARM_MMU_SECOND_LEVEL_TINY_PAGE_ID;              \
    desc.tiny_page.c = (cacheable);                                         \
    desc.tiny_page.b = (bufferable);                                        \
    desc.tiny_page.ap = (perm);                                             \
    desc.tiny_page.base_address = (actual_base);                            \
    *ARC_ARM_MMU_SECOND_LEVEL_DESCRIPTOR_ADDRESS((page_table), (virtual_base)) \
                            = desc.word;                                    \
    CYG_MACRO_END

/* The macro ARC_X_ARM_MMU_SECTION is used to define a memory map.
 *
 * abase, vbase, size : in units of MB
 * abase  : physical memory start address in MB
 * vbase  : virtual memory atart address in MB
 * size   : memory size in MB
 * cache  : if memory is cacheable or not
 * buff   : if memory is write back or write thru
 * access : access rights to be used
 *
 * note: the variable ttb_base is used implicitly
 * ttb_base is the physical address of the first level page table
 */
#define ARC_X_ARM_MMU_SECTION(abase,vbase,size,cache,buff,access)           \
    CYG_MACRO_START                                                         \
    int numpages; int phys = (abase); int virt = (vbase);                   \
    for (numpages = (size); numpages > 0 ; numpages--, phys++, virt++)      \
    {                                                                       \
        ARC_ARM_MMU_SECTION(ttb_base, phys, virt,                           \
                            (cache), (buff), (access));                     \
    }                                                                       \
    CYG_MACRO_END

/* The macro ARC_X_ARM_MMU_PAGE_TABLE is used to define a memory map.
 *
 * abase, vbase, vprotect, vend, atable : in units of byte
 * abase, vbase, vend must be a multiple of 4K
 * abase    : physical memory start address
 * vbase    : virtual memory start address
 * vprotect : virtual address of .data segment
 * vend     : virtual memory end address
 * atable   : physical address of second level page table
 * cache    : if memory is cacheable or not
 * buff     : if memory is write back or write thru
 * access0  : access rights for memory below vprotect
 * access1  : access rights for memory at or above vprotect
 *
 * vbase .. vprotect use access0
 * vprotect .. vend use access1
 * vprotect is rounded down to 1K regions
 *
 * atable is the physical address of a
 * 1K memory block used as coarse page table
 * atable must be a multiple of 1K
 *
 * note: the variable ttb_base is used implicitly
 * ttb_base is the physical address of the first level page table
 */
#define ARC_X_ARM_MMU_PAGE_TABLE(abase,vbase,vprotect,vend,atable,          \
                                 cache,buff,access0,access1)                \
    CYG_MACRO_START                                                         \
    int numpages = (vend)-(vbase);                                          \
    int numprotect = (vprotect)-(vbase);                                    \
    int phys = (abase) >> 12; int virt = (vbase) >> 12;                     \
    numpages = numpages >> 12;                                              \
    numprotect = numprotect >> 10;                                          \
    while( numpages > 0 )                                                   \
    {                                                                       \
        if ( !(phys & 0xFF) && !(virt & 0xFF) && (numpages >= 0x100) &&     \
             !((numprotect > 0) && (numprotect < 0x400)) )                  \
        {                                                                   \
            ARC_ARM_MMU_SECTION(ttb_base, phys>>8, virt>>8, (cache), (buff),\
                                numprotect>0 ? (access0) : (access1));      \
            phys += 0x100; virt += 0x100; numpages -= 0x100;                \
            numprotect -= 0x400;                                            \
        }                                                                   \
        else                                                                \
        {                                                                   \
            ARC_ARM_MMU_PAGE_TABLE(ttb_base, (atable)>>10, virt>>8);        \
            if ( !(phys & 0xF) && !(virt & 0xF) && (numpages >= 0x10) &&    \
                 !((numprotect > 0) && (numprotect < 0x40) && (numprotect & 0xF)) ) \
            {                                                               \
                do                                                          \
                ARC_ARM_MMU_LARGE_PAGE((atable), phys>>4, virt++ & 0xFF,    \
                  (cache), (buff), numprotect>0x00 ? (access0) : (access1), \
                                   numprotect>0x10 ? (access0) : (access1), \
                                   numprotect>0x20 ? (access0) : (access1), \
                                   numprotect>0x30 ? (access0) : (access1));\
                while(virt & 0xF);                                          \
                phys += 0x10; numpages -= 0x10;                             \
                numprotect -= 0x40;                                         \
            }                                                               \
            else                                                            \
            {                                                               \
                ARC_ARM_MMU_SMALL_PAGE((atable), phys, virt++ & 0xFF,       \
                  (cache), (buff), numprotect>0x0 ? (access0) : (access1),  \
                                   numprotect>0x1 ? (access0) : (access1),  \
                                   numprotect>0x2 ? (access0) : (access1),  \
                                   numprotect>0x3 ? (access0) : (access1)); \
                phys ++; numpages --; numprotect -= 4;                      \
            }                                                               \
        }                                                                   \
    }                                                                       \
    CYG_MACRO_END

/* The macro ARC_X_ARM_MMU_FINE_TABLE is used to define a memory map.
 *
 * abase, vbase, vend, atable : in units of byte
 * abase, vbase, vend must be a multiple of 1K
 * abase  : physical memory start address
 * vbase  : virtual memory start address
 * vend   : virtual memory end address
 * atable : physical address of second level page table
 * cache  : if memory is cacheable or not
 * buff   : if memory is write back or write thru
 * access : access rights to be used
 *
 * atable is the physical address of a
 * 4K memory block used as fine page table
 * atable must be a multiple of 4K
 *
 * note: the variable ttb_base is used implicitly
 * ttb_base is the physical address of the first level page table
 */
#define ARC_X_ARM_MMU_FINE_TABLE(abase,vbase,vend,atable,cache,buff,access) \
    CYG_MACRO_START                                                         \
    int numpages = (vend)-(vbase);                                          \
    int phys = (abase) >> 10; int virt = (vbase) >> 10;                     \
    numpages = numpages >> 10;                                              \
    while( numpages > 0 )                                                   \
    {                                                                       \
        if ( !(phys & 0x3FF) && !(virt & 0x3FF) && (numpages >= 0x400) )    \
        {                                                                   \
            ARC_ARM_MMU_SECTION(ttb_base, phys>>10, virt>>10,               \
                                (cache), (buff), (access));                 \
            phys += 0x400; virt += 0x400; numpages -= 0x400;                \
        }                                                                   \
        else                                                                \
        {                                                                   \
            ARC_ARM_MMU_FINE_TABLE(ttb_base, (atable)>>12, virt>>10);       \
            if ( !(phys & 0x3F) && !(virt & 0x3F) && (numpages >= 0x40) )   \
            {                                                               \
                do                                                          \
                ARC_ARM_MMU_LARGE_PAGE((atable), phys>>6, virt++ & 0x3FF,   \
                  (cache), (buff), (access), (access), (access), (access)); \
                while(virt & 0x3F);                                         \
                phys += 0x40; numpages -= 0x40;                             \
            }                                                               \
            else if ( !(phys & 0x3) && !(virt & 0x3) && (numpages >= 0x4) ) \
            {                                                               \
                do                                                          \
                ARC_ARM_MMU_SMALL_PAGE((atable), phys>>2, virt++ & 0x3FF,   \
                  (cache), (buff), (access), (access), (access), (access)); \
                while(virt & 0x3);                                          \
                phys += 0x4; numpages -= 0x4;                               \
            }                                                               \
            else                                                            \
            {                                                               \
                ARC_ARM_MMU_TINY_PAGE((atable), phys, virt++ & 0x3FF,       \
                                      (cache), (buff), (access));           \
                phys ++; numpages --;                                       \
            }                                                               \
        }                                                                   \
    }                                                                       \
    CYG_MACRO_END

#define ARC_ARM_UNCACHEABLE                         0
#define ARC_ARM_CACHEABLE                           1
#define ARC_ARM_UNBUFFERABLE                        0
#define ARC_ARM_BUFFERABLE                          1

#define ARC_ARM_ACCESS_PERM_NONE_NONE               0
#define ARC_ARM_ACCESS_PERM_RO_NONE                 0
#define ARC_ARM_ACCESS_PERM_RO_RO                   0
#define ARC_ARM_ACCESS_PERM_RW_NONE                 1
#define ARC_ARM_ACCESS_PERM_RW_RO                   2
#define ARC_ARM_ACCESS_PERM_RW_RW                   3

#endif//CYGONCE_VAR_MMU_H

