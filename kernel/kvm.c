#include "inc/x86.h"

//#include "process.h"
#include "inc/memory.h"
#include "inc/string.h"
#include "inc/process.h"
#include "inc/pmap.h"

/* This source file involves some hardware details. Please refer to
 *  _ ____   ___    __    __  __                         _
 * (_)___ \ / _ \  / /   |  \/  |                       | |
 *  _  __) | (_) |/ /_   | \  / | __ _ _ __  _   _  __ _| |
 * | ||__ < > _ <| '_ \  | |\/| |/ _` | '_ \| | | |/ _` | |
 * | |___) | (_) | (_) | | |  | | (_| | | | | |_| | (_| | |
 * |_|____/ \___/ \___/  |_|  |_|\__,_|_| |_|\__,_|\__,_|_|
 */

static TSS tss;

static void set_tss(SegDesc *ptr) {
	tss.ss0 = SELECTOR_KERNEL(SEG_KERNEL_DATA);		// only one ring 0 stack segment
	tss.esp0 = 0x800000;
	uint32_t base = (uint32_t)&tss;
	uint32_t limit = sizeof(TSS) - 1;
	ptr->limit_15_0  = limit & 0xffff;
	ptr->base_15_0   = base & 0xffff;
	ptr->base_23_16  = (base >> 16) & 0xff;
	ptr->type = SEG_TSS_32BIT;
	ptr->segment_type = 0;
	ptr->privilege_level = DPL_USER;
	ptr->present = 1;
	ptr->limit_19_16 = limit >> 16;
	ptr->soft_use = 0;
	ptr->operation_size = 0;
	ptr->pad0 = 1;
	ptr->granularity = 0;
	ptr->base_31_24  = base >> 24;
}

static SegDesc gdt[NR_SEGMENTS];

static void
set_segment(SegDesc *ptr, uint32_t pl, uint32_t type) {
	ptr->limit_15_0  = 0xFFFF;
	ptr->base_15_0   = 0x0;
	ptr->base_23_16  = 0x0;
	ptr->type = type;
	ptr->segment_type = 1;
	ptr->privilege_level = pl;
	ptr->present = 1;
	ptr->limit_19_16 = 0xF;
	ptr->soft_use = 0;
	ptr->operation_size = 0;
	ptr->pad0 = 1;
	ptr->granularity = 1;
	ptr->base_31_24  = 0x0;
}

void
init_segment(void) {
	memset(gdt, 0, sizeof(gdt));
	set_segment(&gdt[SEG_KERNEL_CODE], DPL_KERNEL, SEG_EXECUTABLE | SEG_READABLE);
	set_segment(&gdt[SEG_KERNEL_DATA], DPL_KERNEL, SEG_WRITABLE );
	set_segment(&gdt[SEG_USER_CODE], DPL_USER, SEG_EXECUTABLE | SEG_READABLE);
	set_segment(&gdt[SEG_USER_DATA], DPL_USER, SEG_WRITABLE );


	//set_segment(&gdt[SEG_TSS], DPL_USER, SEG_EXECUTABLE | SEG_READABLE );

	write_gdtr(gdt, sizeof(gdt));

	set_tss(&gdt[SEG_TSS]);
	write_tr( SELECTOR_USER(SEG_TSS) );
}


void set_tss_esp0(uint32_t esp) {
	tss.esp0 = esp;
}
void pcb_enter(PCB* pcb)
{
    lcr3(PADDR(pcb->pgdir));
    set_tss_esp0((uint32_t)(pcb->user_stacktop));
    struct TrapFrame *tf = pcb->tf;
    printf("eip:%x\n",tf->eip);
    asm volatile("mov %0, %%ds" : : "r"(tf->ds));
    asm volatile("mov %0, %%es" : : "r"(tf->es));
    asm volatile("mov %0, %%fs" : : "r"(tf->fs));
    asm volatile("mov %0, %%gs" : : "r"(tf->gs));
    asm volatile("pushl %0" : : "r"((uint32_t)tf->ss));
    asm volatile("pushl %0" : : "r"(tf->esp));
    asm volatile("pushl %0" : : "r"(tf->eflags));
    asm volatile("pushl %0" : : "r"((uint32_t)tf->cs));
    asm volatile("pushl %0" : : "r"(tf->eip));
    asm volatile("iret");
}

void init_page(void) {
    page_init();
}

void switch_proc();
extern PCB* cur_pcb;
void scheduler_switch(PCB* pcb){
    cur_pcb = pcb;
    pcb_enter(pcb);
    lcr3(PADDR(pcb->pgdir));
    set_tss_esp0((uint32_t)(pcb->user_stacktop));
    printf("seccess\n");
    asm volatile("mov %0, %%esp" : : "r"(pcb->tf));
    printf("seccess2\n");
    asm volatile("jmp %0" : : "r"(switch_proc));
    printf("seccess3\n");

}
