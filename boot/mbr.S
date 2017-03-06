#include <inc/mmu.h>
.code16               # 告诉 gcc 之后生成 16 位机器指令
                      # 开启保护模式之前 QEMU 只能执行 16 位机器指令
.set PROT_MODE_CSEG, 0x8         # kernel code segment selector
.set PROT_MODE_DSEG, 0x10        # kernel data segment selector
.set CR0_PE_ON,      0x1         # protected mode enable flag

.globl start          # 将符号 start 暴露给链接器
start:                # start 表示代码的起始地址
  cli                 # 关中断
  mov $0x13, %ah      # 设置模式：显式字符串
  mov $0x01, %al      # 设置显式模式：(1) 使用 BL 指定的样式 (2) 更新光标
  mov $0x00, %bh      # 设置代码页，与字体相关； QEMU 下仅第一个代码页可用
  mov $0x4F, %bl      # 设置字符样式：红底白字无高亮
  mov $(end-str), %cx # 优雅地计算字符串的长度
  mov $str, %dx       # 将 ES:BP 指向字符串 (Step 1)
  mov %dx, %bp        # 将 ES:BP 指向字符串 (Step 2)
  mov $0x00, %dh      # 首字符输出在第 1 行
  mov $0x00, %dl      # 首字符输出在第 1 列
  int $0x10           # BIOS 中断，VGA 相关服务
.code16                     # Assemble for 16-bit mode
  cli                         # Disable interrupts
  cld                         # String operations increment

  # Set up the important data segment registers (DS, ES, SS).
  xorw    %ax,%ax             # Segment number zero
  movw    %ax,%ds             # -> Data Segment
  movw    %ax,%es             # -> Extra Segment
  movw    %ax,%ss             # -> Stack Segment

  # Enable A20:
  #   For backwards compatibility with the earliest PCs, physical
  #   address line 20 is tied low, so that addresses higher than
  #   1MB wrap around to zero by default.  This code undoes this.
seta20.1:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.1

  movb    $0xd1,%al               # 0xd1 -> port 0x64
  outb    %al,$0x64

seta20.2:
  inb     $0x64,%al               # Wait for not busy
  testb   $0x2,%al
  jnz     seta20.2

  movb    $0xdf,%al               # 0xdf -> port 0x60
  outb    %al,$0x60

  # Switch from real to protected mode, using a bootstrap GDT
  # and segment translation that makes virtual addresses 
  # identical to their physical addresses, so that the 
  # effective memory map does not change during the switch.
  lgdt    gdtdesc
  movl    %cr0, %eax
  orl     $CR0_PE_ON, %eax
  movl    %eax, %cr0
  
  # Jump to next instruction, but in 32-bit code segment.
  # Switches processor into 32-bit mode.
  ljmp    $PROT_MODE_CSEG, $protcseg

  .code32                     # Assemble for 32-bit mode
protcseg:
  # Set up the protected-mode data segment registers
  movw    $PROT_MODE_DSEG, %ax    # Our data segment selector
  movw    %ax, %ds                # -> DS: Data Segment
  movw    %ax, %es                # -> ES: Extra Segment
  movw    %ax, %fs                # -> FS
  movw    %ax, %gs                # -> GS
  movw    %ax, %ss                # -> SS: Stack Segment
  
  # Set up the stack pointer and call into C.
  movl    $start, %esp
  call bootmain

  # If bootmain returns (it shouldn't), loop.
spin:
  jmp spin

# Bootstrap GDT
.p2align 2                                # force 4 byte alignment
gdt:
  SEG_NULL				# null seg
  SEG(STA_X|STA_R, 0x0, 0xffffffff)	# code seg
  SEG(STA_W, 0x0, 0xffffffff)	        # data seg

gdtdesc:
  .word   0x17                            # sizeof(gdt) - 1
  .long   gdt                             # address gdt

str:                  # str 表示字符串首地址
  .ascii "Hello, World!"
end:                  # end 表示字符串之后的地址，用于计算长度
