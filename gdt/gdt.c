#include "gdt.h"
#include "string.h"

// 全局描述符表长度
#define GDT_LENGTH 5

// 全局描述符表定义
gdt_entry_t gdts[GDT_LENGTH];

// GDTR
gdt_ptr_t gdtr;

static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran);

// 声明内核栈地址
extern uint32_t stack;

// 初始化全局描述符表
void init_gdt() {
  // 全局描述符表界限 e.g. 从 0 开始，所以总长要 − 1
  gdtr.limit = sizeof(gdt_entry_t) * GDT_LENGTH - 1;
  gdtr.base = (uint32_t)&gdts;

  // 采用 Intel 平坦模型
  gdt_set_gate(0, 0, 0, 0, 0); // 按照 Intel 文档要求，第一个描述符必须全 0
  gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // 指令段
  gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // 数据段
  gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // 用户模式代码段
  gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // 用户模式数据段

  // 加载全局描述符表地址到 GPTR 寄存器
  gdt_flush((uint32_t)&gdtr);
}

// 全局描述符表构造函数，根据下标构造
// 参数分别是数组下标、基地址、限长、访问标志，其它访问标志
static void gdt_set_gate(int32_t num, uint32_t base, uint32_t limit,
                         uint8_t access, uint8_t gran) {
  gdts[num].base_low = (base & 0xFFFF);
  gdts[num].base_middle = (base >> 16) & 0xFF;
  gdts[num].base_high = (base >> 24) & 0xFF;

  gdts[num].limit_low = (limit & 0xFFFF);
  gdts[num].granularity = (limit >> 16) & 0x0F;

  gdts[num].granularity |= gran & 0xF0;
  gdts[num].access = access;
}