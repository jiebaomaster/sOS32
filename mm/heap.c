#include "heap.h"
#include "debug.h"
#include "pmm.h"
#include "vmm.h"

// 申请内存块
static void alloc_chunk(uint32_t start, uint32_t length);

// 释放内存块
static void free_chunk(header_t *chunk);

// 切分内存块
static void split_chunk(header_t *chunk, uint32_t len);

// 合并内存块
static void glue_chunk(header_t *chunk);

// 当前堆的堆顶地址
static uint32_t heap_max = HEAP_START;

// 内存块管理头指针
static header_t *heap_first;

void init_heap() { heap_first = 0; }

void *kmalloc(uint32_t len) {
  // 申请的内存大小要加上管理头的长度
  // 因为在内存申请和释放的时候要通过该结构去管理
  len += sizeof(header_t);

  header_t *cur_header = heap_first;
  header_t *prev_header = 0;

  // 从头指针开始遍历整个内存块管理链表
  while (cur_header) {
    // 如果当前内存块未占用，且空间足够大
    if (cur_header->allocated == 0 && cur_header->length >= len) {
      // 将空闲的内存块分割为占用部分和剩余部分
      split_chunk(cur_header, len);
      // 当前内存块被占用
      cur_header->allocated = 1;
      // 返回的时候必须将指针挪到管理结构之后
      return (void *)((uint32_t)cur_header + sizeof(header_t));
    }

    // 下一个内存块
    prev_header = cur_header;
    cur_header = cur_header->next;
  }

  uint32_t chunk_start;
  if (prev_header) { // perv_header不为0说明遍历到堆尾还未成功分配，需要扩大堆
    chunk_start = (uint32_t)prev_header + prev_header->length;
  } else { // 为0说明是第一次分配内存
    // 初始化内存块起始位置，并给堆第一次分配空间
    chunk_start = HEAP_START;
    heap_first = (header_t *)chunk_start;
  }

  // 给堆分配物理页
  alloc_chunk(chunk_start, len);
  // 在堆尾空间初始化内存块管理头
  cur_header = (header_t *)chunk_start;
  cur_header->prev = prev_header;
  cur_header->next = 0;
  cur_header->allocated = 1;
  cur_header->length = len;

  if (prev_header)
    prev_header->next = cur_header;

  return (void *)(chunk_start + sizeof(header_t));
}

void kfree(void *p) {
  // p指针的前面就是header
  header_t *header = (header_t *)((uint32_t)p - sizeof(header_t));
  // 将已使用标记置 0
  header->allocated = 0;
  // 释放完的空间尝试与前后相邻空间合并
  glue_chunk(header);
}

void alloc_chunk(uint32_t start, uint32_t len) {
  // 如果当前堆尾部剩余空间不够分配，申请物理内存页直到有到足够的可用内存
  while (start + len > heap_max) {
    // 分配物理页面
    uint32_t page = pmm_alloc_page();
    // 在页表中建立虚拟地址到物理页面的映射
    map(pgd_kern, heap_max, page, PAGE_PRESENT | PAGE_WRITE);
    // 更新堆顶指针
    heap_max += PAGE_SIZE;
  }
}

void free_chunk(header_t *chunk) {
  if (chunk->prev == 0) { // 释放的是第一个内存块
    heap_first = 0;
  } else { // 释放堆尾的内存块，断开链表链接即可
    chunk->prev->next = 0;
  }

  // 当前 chunk ~ heap_max 都是空闲的
  // 空闲的内存超过 1 页的话就释放掉该物理页
  while ((heap_max - PAGE_SIZE) >= (uint32_t)chunk) {
    // 释放最后一页，堆顶向前移动一页
    heap_max -= PAGE_SIZE;
    uint32_t page;
    // 得到堆顶所在物理页的物理地址
    get_mapping(pgd_kern, heap_max, &page);
    // 在页表中解除虚拟地址映射
    unmap(pgd_kern, heap_max);
    // 释放堆顶所在物理页
    pmm_free_page(page);
  }
}

void split_chunk(header_t *chunk, uint32_t len) {
  // 切分内存块之前得保证，切分后剩余内存能容纳一个内存块管理头
  if (chunk->length - len > sizeof(header_t)) {
    header_t *new_chunk = (header_t *)((uint32_t)chunk + chunk->length);
    new_chunk->prev = chunk;
    new_chunk->next = chunk->next;
    new_chunk->allocated = 0;
    new_chunk->length = chunk->length - len;

    chunk->next = new_chunk;
    chunk->length = len;
  }
}

void glue_chunk(header_t *chunk) {
  // 和chunk之后的内存块进行合并
  if (chunk->next && chunk->next->allocated == 0) {
    chunk->length = chunk->length + chunk->next->length;
    if (chunk->next->next)
      chunk->next->next->prev = chunk;
    chunk->next = chunk->next->next;
  }

  // 和chunk之前的内存块进行合并
  if (chunk->prev && chunk->prev->allocated == 0) {
    chunk->prev->length = chunk->prev->length + chunk->length;
    chunk->prev->next = chunk->next;
    if (chunk->next)
      chunk->next->prev = chunk->prev;
    // chunk 向前合并之后，使用前面的内存块管理头
    chunk = chunk->prev;
  }

  // 该空闲的内存块是堆中的最后一个内存块，则可以释放这个内存块
  // 并尝试释放堆占用的物理页面
  if (chunk->next == 0)
    free_chunk(chunk);
}

// 遍历整个堆控制头链表，输出当前堆的管理信息
void heap_header_walk() {
  if (heap_first == 0) {
    printk("heap is empty.\n");
    return;
  }

  header_t *heap_walker = heap_first;
  while(1) {
    printk("header 0x%X, %s, len:%d", (uint32_t)heap_walker, heap_walker->allocated ? "used" : "free", heap_walker->length);

    heap_walker = heap_walker->next;
    if (heap_walker) {
      printk(" ==> ");
    } else
      break;
  }
  printk("\nheap_max = 0x%X\n", heap_max);
}

// 测试内核堆分配释放
void test_heap() {
  printk_color(rc_black, rc_magenta, "Test kmalloc() && kfree() now ...\n\n");

  void *addr1 = kmalloc(50);
  printk("kmalloc    50 byte in 0x%X\n", addr1);
  void *addr2 = kmalloc(500);
  printk("kmalloc   500 byte in 0x%X\n", addr2);
  void *addr3 = kmalloc(5000);
  printk("kmalloc  5000 byte in 0x%X\n", addr3);
  void *addr4 = kmalloc(50000);
  printk("kmalloc 50000 byte in 0x%X\n\n", addr4);
  heap_header_walk();
  printk("\n");

  printk("free mem in 0x%X\n", addr1);
  kfree(addr1);
  printk("free mem in+ 0x%X\n", addr2);
  kfree(addr2);
  printk("free mem in 0x%X\n", addr3);
  kfree(addr3);
  printk("free mem in 0x%X\n\n", addr4);
  kfree(addr4);
  printk("\n");
}