#include "idt.h"
#include "gdt.h"
#include "console.h"
#include "debug.h"

int kern_entry() {
  init_debug();
  init_gdt();
  init_idt();

  console_clear();
  console_write_color("Hello world!\n", rc_black, rc_green);
  
  // panic("test");
  // 手动触发两个中断
  asm volatile ("int $0x3");
  asm volatile ("int $0x4");

  return 0;
}