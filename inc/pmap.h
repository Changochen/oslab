/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PMAP_H
#define JOS_KERN_PMAP_H

#include "inc/memlayout.h"
#include "lib/common.h"
#include "inc/memory.h"
#define npages (1<<15)


extern struct PageInfo pages[];

extern pde_t *kern_pgdir;


/* This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address.  It panics if you pass it a
 * non-kernel virtual address.
 */
#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)

    static inline physaddr_t
_paddr(const char *file, int line, void *kva)
{
    if ((uint32_t)kva < KERNBASE)
        printf(file, line, "PADDR called with invalid kva 0x%x", kva);
    return (physaddr_t)kva - KERNBASE;
}

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address.  It panics if you pass an invalid physical address. */
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

    static inline void*
_kaddr(const char *file, int line, physaddr_t pa)
{
    if (PGNUM(pa) >= npages)
        printf("KADDR called with invalid pa 0x%x", pa);
    return (void *)(pa + KERNBASE);
}


enum {
    // For page_alloc, zero the returned physical page.
    ALLOC_ZERO = 1<<0,
};

void	mem_init(void);

void	page_init(void);
void    init_page();
struct PageInfo *page_alloc(int alloc_flags);
void	page_free(struct PageInfo *pp);
int	page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm);
void	page_remove(pde_t *pgdir, void *va);
struct PageInfo *page_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
void	page_decref(struct PageInfo *pp);

void	tlb_invalidate(pde_t *pgdir, void *va);


    static inline physaddr_t
page2pa(struct PageInfo *pp)
{
    return (pp - pages) << PGSHIFT;
}

    static inline struct PageInfo*
pa2page(physaddr_t pa)
{
    if (PGNUM(pa) >= npages)
        printf("pa2page called with invalid pa");
    return &pages[PGNUM(pa)];
}

    static inline void*
page2kva(struct PageInfo *pp)
{
    return KADDR(page2pa(pp));
}

pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);
void mm_alloc(pde_t *pgdir, uint32_t va, size_t len);
#endif /* !JOS_KERN_PMAP_H */
