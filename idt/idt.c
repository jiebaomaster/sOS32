#include "idt.h"
#include "debug.h"
#include "string.h"

// 中断描述符表
// Intel的处理器允许256个中断，中断号范围是0～255。
idt_entry_t idts[256];

// IDTR
idt_ptr_t idtr;

// 中断处理函数的指针数组
interrupt_handler_t interrupt_handlers[256];

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
                         uint8_t flags);

// 声明加载 IDTR 的函数
extern void idt_flush(uint32_t);

void isr_handler(pt_regs *regs) {
  // 如果该中断号注册了处理函数，则执行相应的函数，否则打印出中断号码
  if (interrupt_handlers[regs->int_no]) {
    interrupt_handlers[regs->int_no](regs);
  } else {
    printk_color(rc_black, rc_blue, "Unhandled interrupt: %d\n", regs->int_no);
  }
}

void init_idt() {
  bzero((uint8_t *)&interrupt_handlers, sizeof(interrupt_handler_t) * 256);

  idtr.limit = sizeof(idt_entry_t) * 256 - 1;
  idtr.base = (uint32_t *)&idts;

  bzero((uint8_t *)&idts, sizeof(idt_entry_t) * 256);

  // 0-32:  用于 CPU 的中断处理
  idt_set_gate(0, (uint32_t)isr0, 0x08, 0x8E);
  idt_set_gate(1, (uint32_t)isr1, 0x08, 0x8E);
  idt_set_gate(2, (uint32_t)isr2, 0x08, 0x8E);
  idt_set_gate(3, (uint32_t)isr3, 0x08, 0x8E);
  idt_set_gate(4, (uint32_t)isr4, 0x08, 0x8E);
  idt_set_gate(5, (uint32_t)isr5, 0x08, 0x8E);
  idt_set_gate(6, (uint32_t)isr6, 0x08, 0x8E);
  idt_set_gate(7, (uint32_t)isr7, 0x08, 0x8E);
  idt_set_gate(8, (uint32_t)isr8, 0x08, 0x8E);
  idt_set_gate(9, (uint32_t)isr9, 0x08, 0x8E);
  idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
  idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
  idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
  idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
  idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
  idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
  idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
  idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
  idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
  idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
  idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
  idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
  idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
  idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
  idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
  idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
  idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
  idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
  idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
  idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
  idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
  idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);

  idt_set_gate(255, (uint32_t)isr255, 0x08, 0x8E);

  idt_flush((uint32_t)&idtr);
}

// 设置中断描述符
static void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel,
                         uint8_t flags) {
  idts[num].base_lo = base & 0xFFFF;
  idts[num].base_hi = (base >> 16) & 0xFFFF;

  idts[num].sel = sel;
  idts[num].always0 = 0;

  // 先留下 0x60 这个魔数，以后实现用户态时候
  // 这个与运算可以设置中断门的特权级别为 3
  idts[num].flags = flags; // | 0x60
}