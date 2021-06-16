#ifndef INCLUDE_PMM_H_
#define INCLUDE_PMM_H_

#include "multiboot.h"

// 线程栈的大小
#define STACK_SIZE 8192

// 支持的最大物理内存(512MB)
#define PMM_MAX_SIZE 0x20000000

// 物理内存页框大小
#define PMM_PAGE_SIZE 0x1000

// 最多支持的物理页面个数
#define PAGE_MAX_SIZE (PMM_MAX_SIZE / PMM_PAGE_SIZE)

// 页掩码，按照 4096 对齐地址
#define PHY_PAGE_MASK 0xFFFFF000

// 内核文件在内存中的起始和结束位置
// 在链接器脚本中要求链接器定义
extern uint8_t kern_start[];
extern uint8_t kern_end[];

// 系统中实际存在的物理页数量
extern uint32_t phy_page_count;

// 输出 BIOS 提供的物理内存布局
void show_mm_map();

// 初始化物理内存管理
void init_pmm();

// 申请一个物理页，并返回其物理地址
uint32_t pmm_alloc_page();

// 释放物理页
void pmm_free_page(uint32_t p);

#endif