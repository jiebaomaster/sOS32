#include "pmm.h"
#include "common.h"
#include "debug.h"
#include "multiboot.h"

// 物理内存页面管理的栈，存储物理页的首地址
static uint32_t pmm_stack[PAGE_MAX_SIZE + 1];

// 物理内存管理的栈指针
static uint32_t pmm_stack_top;

// 系统中实际存在的物理页数量
uint32_t phy_page_count;

void show_mm_map() {
  uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
  uint32_t mmap_length = glb_mboot_ptr->mmap_length;

  printk("Memory map:\n");

  mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
  for (; (uint32_t)mmap < mmap_addr + mmap_length; mmap++) {
    printk("base_addr = 0x%X%08X, length = 0x%X%08X, type = 0x%X\n",
           (uint32_t)mmap->base_addr_high, (uint32_t)mmap->base_addr_low,
           (uint32_t)mmap->length_high, (uint32_t)mmap->length_low,
           (uint32_t)mmap->type);
  }
}

void init_pmm() {
  uint32_t mmap_addr = glb_mboot_ptr->mmap_addr;
  uint32_t mmap_length = glb_mboot_ptr->mmap_length;

  mmap_entry_t *mmap = (mmap_entry_t *)mmap_addr;
  for (; (uint32_t)mmap < mmap_addr + mmap_length; mmap++) {
    // 简单起见，我们只选用物理内存中0x100000开始的可用空间
    if (mmap->type == 1 && mmap->base_addr_low == 0x100000) {
      // 可用地址从 被内核占用的空间 之后开始
      uint32_t page_addr = 
          mmap->base_addr_low + (uint32_t)(kern_end - kern_start);
      uint32_t page_addr_end = mmap->base_addr_low + mmap->length_low;

      // 将可用空间按页划分，并将每一页的首地址存储到页管理栈里
      // 最多支持512的物理内存MB
      while (page_addr < page_addr_end && page_addr <= PMM_MAX_SIZE) {
        pmm_free_page(page_addr);
        page_addr += PMM_PAGE_SIZE;
        phy_page_count++;
      }
    }
  }
}

uint32_t pmm_alloc_page() {
  assert(pmm_stack_top != 0, "out of memory");
  uint32_t page = pmm_stack[pmm_stack_top--];

  return page;
}

void pmm_free_page(uint32_t p) {
  assert(pmm_stack_top != PAGE_MAX_SIZE, "out of pmm_stack stack");

  pmm_stack[++pmm_stack_top] = p;
}