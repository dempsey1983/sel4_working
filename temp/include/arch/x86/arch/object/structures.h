/*
 * Copyright 2014, General Dynamics C4 Systems
 *
 * This software may be distributed and modified according to the terms of
 * the GNU General Public License version 2. Note that NO WARRANTY is provided.
 * See "LICENSE_GPLv2.txt" for details.
 *
 * @TAG(GD_GPL)
 */

#ifndef __ARCH_OBJECT_STRUCTURES_H
#define __ARCH_OBJECT_STRUCTURES_H

#include <assert.h>
#include <config.h>
#include <util.h>
#include <api/types.h>
#include <arch/types.h>
#include <arch/object/structures_gen.h>
#include <arch/machine/hardware.h>
#include <arch/machine/registerset.h>

/* Object sizes*/
#define EP_SIZE_BITS  4
#define NTFN_SIZE_BITS 4
#define CTE_SIZE_BITS 4
#define TCB_BLOCK_SIZE_BITS 10
typedef struct arch_tcb {
    user_context_t tcbContext;
} arch_tcb_t;

/* update this when you modify the tcb struct */
#define EXPECTED_TCB_SIZE 660

#define GDT_NULL    0
#define GDT_CS_0    1
#define GDT_DS_0    2
#define GDT_CS_3    3
#define GDT_DS_3    4
#define GDT_TSS     5
#define GDT_TLS     6
#define GDT_IPCBUF  7
#define GDT_ENTRIES 8

#define SEL_NULL    GDT_NULL
#define SEL_CS_0    (GDT_CS_0 << 3)
#define SEL_DS_0    (GDT_DS_0 << 3)
#define SEL_CS_3    ((GDT_CS_3 << 3) | 3)
#define SEL_DS_3    ((GDT_DS_3 << 3) | 3)
#define SEL_TSS     (GDT_TSS << 3)
#define SEL_TLS     ((GDT_TLS << 3) | 3)
#define SEL_IPCBUF  ((GDT_IPCBUF << 3) | 3)

#define IDT_ENTRIES 256

#ifdef CONFIG_PAE_PAGING
#define PDPTE_SIZE_BITS 3
#define PDPT_BITS    2
#define PDE_SIZE_BITS  3
#define PD_BITS      9
#define PTE_SIZE_BITS 3
#define PT_BITS      9
#else
#define PDPTE_SIZE_BITS 0
#define PDPT_BITS 0
#define PDE_SIZE_BITS  2
#define PD_BITS      10
#define PTE_SIZE_BITS 2
#define PT_BITS      10
#endif

#define PDPTE_PTR(r)   ((pdpte_t *)(r))
#define PDPTE_PTR_PTR(r) ((pdpte_t**)(r))
#define PDPTE_REF(p)   ((unsigned int)(p))

#define PDPT_SIZE_BITS (PDPT_BITS + PDPTE_SIZE_BITS)
#define PDPT_PTR(r)    ((pdpte_t*)(r))
#define PDPT_PREF(p)   ((unsigned int)(p))

#define PDE_PTR(r)     ((pde_t *)(r))
#define PDE_PTR_PTR(r) ((pde_t **)(r))
#define PDE_REF(p)     ((unsigned int)(p))

#define PD_SIZE_BITS (PD_BITS + PDE_SIZE_BITS)
#define PD_PTR(r)    ((pde_t *)(r))
#define PD_REF(p)    ((unsigned int)(p))

#define PTE_PTR(r)    ((pte_t *)(r))
#define PTE_REF(p)    ((unsigned int)(p))

#define PT_SIZE_BITS (PT_BITS + PTE_SIZE_BITS)
#define PT_PTR(r)    ((pte_t *)(r))
#define PT_REF(p)    ((unsigned int)(p))

#ifdef CONFIG_IOMMU

#define VTD_RT_SIZE_BITS  12

#define VTD_CTE_SIZE_BITS 3
#define VTD_CTE_PTR(r)    ((vtd_cte_t*)(r))
#define VTD_CT_BITS       9
#define VTD_CT_SIZE_BITS  (VTD_CT_BITS + VTD_CTE_SIZE_BITS)

#define VTD_PTE_SIZE_BITS 3
#define VTD_PTE_PTR(r)    ((vtd_pte_t*)(r))
#define VTD_PT_BITS       9
#define VTD_PT_SIZE_BITS  (VTD_PT_BITS + VTD_PTE_SIZE_BITS)

#endif

/* helper structure for filling descriptor registers */
typedef struct gdt_idt_ptr {
    uint16_t limit;
    uint16_t basel;
    uint16_t baseh;
} gdt_idt_ptr_t;

compile_assert(gdt_idt_ptr_packed,
               sizeof(gdt_idt_ptr_t) == sizeof(uint16_t) * 3)

#define WORD_SIZE_BITS 2

enum vm_rights {
    VMKernelOnly = 1,
    VMReadOnly = 2,
    VMReadWrite = 3
};
typedef uint32_t vm_rights_t;

enum asidSizeConstants {
    asidHighBits = 6,
    asidLowBits = 10
};

struct asid_pool {
    void* array[BIT(asidLowBits)];
};

typedef struct asid_pool asid_pool_t;

#define ASID_POOL_BITS      asidLowBits
#define ASID_POOL_SIZE_BITS (ASID_POOL_BITS + WORD_SIZE_BITS)
#define ASID_POOL_PTR(r)    ((asid_pool_t*)r)
#define ASID_POOL_REF(p)    ((unsigned int)p)
#define ASID_BITS           (asidHighBits + asidLowBits)
#define nASIDPools          BIT(asidHighBits)
#define ASID_LOW(a)         (a & MASK(asidLowBits))
#define ASID_HIGH(a)        ((a >> asidLowBits) & MASK(asidHighBits))

static inline asid_t CONST
cap_frame_cap_get_capFMappedASID(cap_t cap)
{
    return
        (cap_frame_cap_get_capFMappedASIDHigh(cap) << asidLowBits) +
        cap_frame_cap_get_capFMappedASIDLow(cap);
}

static inline cap_t CONST
cap_frame_cap_set_capFMappedASID(cap_t cap, word_t asid)
{
    cap = cap_frame_cap_set_capFMappedASIDLow(cap, ASID_LOW(asid));
    return cap_frame_cap_set_capFMappedASIDHigh(cap, ASID_HIGH(asid));
}

static inline asid_t PURE
cap_frame_cap_ptr_get_capFMappedASID(cap_t* cap)
{
    return cap_frame_cap_get_capFMappedASID(*cap);
}

static inline void
cap_frame_cap_ptr_set_capFMappedASID(cap_t* cap, asid_t asid)
{
    *cap = cap_frame_cap_set_capFMappedASID(*cap, asid);
}

static inline asid_t PURE
cap_get_capMappedASID(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {
    case cap_pdpt_cap:
        return cap_pdpt_cap_get_capPDPTMappedASID(cap);

    case cap_page_directory_cap:
        return cap_page_directory_cap_get_capPDMappedASID(cap);

    default:
        fail("Invalid arch cap type");
    }
}

static inline unsigned int CONST
cap_get_archCapSizeBits(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {
    case cap_frame_cap:
        return pageBitsForSize(cap_frame_cap_get_capFSize(cap));

    case cap_page_table_cap:
        return PT_SIZE_BITS;

    case cap_page_directory_cap:
        return PD_SIZE_BITS;

    case cap_pdpt_cap:
        return PDPT_SIZE_BITS;

    case cap_io_port_cap:
        return 0;
#ifdef CONFIG_IOMMU
    case cap_io_space_cap:
        return 0;

    case cap_io_page_table_cap:
        return VTD_PT_SIZE_BITS;
#endif
    case cap_asid_control_cap:
        return 0;

    case cap_asid_pool_cap:
        return ASID_POOL_SIZE_BITS;

    default:
        fail("Invalid arch cap type");
    }
}

static inline void * CONST
cap_get_archCapPtr(cap_t cap)
{
    cap_tag_t ctag;

    ctag = cap_get_capType(cap);

    switch (ctag) {

    case cap_frame_cap:
        return (void *)(cap_frame_cap_get_capFBasePtr(cap));

    case cap_page_table_cap:
        return PD_PTR(cap_page_table_cap_get_capPTBasePtr(cap));

    case cap_page_directory_cap:
        return PT_PTR(cap_page_directory_cap_get_capPDBasePtr(cap));

    case cap_pdpt_cap:
        return PDPT_PTR(cap_pdpt_cap_get_capPDPTBasePtr(cap));

    case cap_io_port_cap:
        return NULL;
#ifdef CONFIG_IOMMU
    case cap_io_space_cap:
        return NULL;

    case cap_io_page_table_cap:
        return (void *)(cap_io_page_table_cap_get_capIOPTBasePtr(cap));
#endif
    case cap_asid_control_cap:
        return NULL;

    case cap_asid_pool_cap:
        return ASID_POOL_PTR(cap_asid_pool_cap_get_capASIDPool(cap));

    default:
        fail("Invalid arch cap type");
    }
}

#endif
