#include "console.h"
#include "debug.h"
#include "gdt.h"
#include "heap.h"
#include "idt.h"
#include "pmm.h"
#include "sched.h"
#include "task.h"
#include "timer.h"
#include "vmm.h"

// 内核初始化函数
void kern_init();

// 开启分页机制之后的 Multiboot 数据指针
multiboot_t *glb_mboot_ptr;

// 开启分页机制之后的内核栈
char kern_stack[STACK_SIZE];

// 内核栈的栈顶，高地址端，CPU使用栈为从高地址扩展到低地址
// 栈地址从高到低为 [kern_stack_top->kern_stack]
uint32_t kern_stack_top;

// 内核使用的临时页表和页目录
// 该地址必须是页对齐的地址，内存 0−640KB 肯定是空闲的
__attribute__((section(".init.data"))) pgd_t *pgd_tmp = (pgd_t *)0x1000;
__attribute__((section(".init.data"))) pte_t *pte_low = (pte_t *)0x2000;
__attribute__((section(".init.data"))) pte_t *pte_hign = (pte_t *)0x3000;

/**
 * 内核入口函数
 * 从segmention进入到paging，并将物理地址的低端 4M 映射到虚拟地址 3G 开始的空间
 * 先组织好临时页表，再启动分页，系统基于临时页表运行
 */
__attribute__((section(".init.text"))) void kern_entry() {
  pgd_tmp[0] = (uint32_t)pte_low | PAGE_PRESENT | PAGE_WRITE;
  pgd_tmp[PGD_INDEX(PAGE_OFFSET)] =
      (uint32_t)pte_hign | PAGE_PRESENT | PAGE_WRITE;

  int i;
  // phy [0x00000000, 0x00400000) => vir [0x00000000, 0x00400000)
  // 将物理地址空间的低端4MB映射到虚拟地址的低端4MB，以保证下面的代码开启分页后，
  // 本函数内使用虚拟地址寻址正确。因为本函数是以1MB为起始地址生成的。
  for (i = 0; i < 1024; i++)
    pte_low[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;
  // phy [0x00000000, 0x00400000) => vir [0xC0000000, 0xC0400000)
  // 将物理地址的低端4MB映射到虚拟地址的0xC0000000开始的4MB，以保证开启分页后，
  // 其他代码能运行正确，因为其他代码是以0xC0000000为起始地址生成的。
  for (i = 0; i < 1024; i++)
    pte_hign[i] = (i << 12) | PAGE_PRESENT | PAGE_WRITE;

  // 设置临时页表
  asm volatile("mov %0, %%cr3" : : "r"(pgd_tmp));

  uint32_t cr0;
  // 将 cr0 寄存器的分页位置为 1，启用分页
  asm volatile("mov %%cr0, %0" : "=r"(cr0));
  cr0 |= 0x80000000;
  asm volatile("mov %0, %%cr0" : : "r"(cr0));

  // 切换内核栈
  kern_stack_top = (uint32_t)(kern_stack + STACK_SIZE) & 0xFFFFFFF0;
  asm volatile("mov %0, %%esp\n\t"
               "xor %%ebp, %%ebp"
               :
               : "r"(kern_stack_top));

  // 更新全局 multiboot_t 指针
  glb_mboot_ptr = (multiboot_t *)((uint32_t)mboot_ptr_tmp + PAGE_OFFSET);

  // 调用内核初始化函数
  kern_init();
}

int flag = 0;

int thread_A(void *arg) {
  while (1)
    if (flag == 0) {
      printk_color(rc_black, rc_red, "A");
      flag = 1;
    }

  return 0;
}

int thread_B(void *arg) {
  while (1)
    if (flag == 1) {
      printk_color(rc_black, rc_green, "B");
      flag = 0;
    }

  return 0;
}

void kern_init() {
  init_debug();
  init_gdt();
  init_idt();

  console_clear();
  console_write_color("Hello world!\n", rc_black, rc_green);

  // panic("test");
  // 手动触发两个中断
  // asm volatile ("int $0x3");
  // asm volatile ("int $0x4");

  init_timer(200);

  printk("Kernel in memory start: 0x%08X\n", kern_start);
  printk("Kernel in memory end:   0x%08X\n", kern_end);
  printk("Kernel in memory used:   %d KB\n\n",
         (kern_end - kern_start + 1023) / 1024);

  show_mm_map();
  init_pmm();
  init_vmm();
  init_heap();

  test_pmm();
  test_heap();

  init_sched();
  kernel_thread(thread_A, NULL);
  kernel_thread(thread_B, NULL);

  // 开启中断
  asm volatile("sti");

  while (1)
    asm volatile("hlt");
}