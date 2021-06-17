#include "elf.h"
#include "common.h"
#include "string.h"
#include "vmm.h"

// 从 multiboot_t 结构获取信息ELF
elf_t elf_from_multiboot(multiboot_t *mb) {
  int i;
  elf_t elf;
  // section头表
  elf_section_header_t *sh = (elf_section_header_t *)mb->addr;

  // section名称字符串表
  uint32_t shstrtab = sh[mb->shndx].addr;
  for (i = 0; i < mb->num; i++) {
    // sh[i].name为当前section的名称字符串在其字符串表中的偏移
    const char *name = (const char *)(shstrtab + sh[i].name) + PAGE_OFFSET;
    // 在 GRUB 提供的 multiboot 信息中寻找 内核ELF文件 中
    // 包含的 符号名称字符串表 和 符号表 的地址和大小
    if (strcmp(name, ".strtab") == 0) { // 符号名称字符串表
      elf.strtab = (const char *)sh[i].addr + PAGE_OFFSET;
      elf.strtabsz = sh[i].size;
    }
    if (strcmp(name, ".symtab") == 0) { // 符号表
      elf.symtab = (elf_symbol_t *)sh[i].addr + PAGE_OFFSET;
      elf.symtabsz = sh[i].size;
    }
  }
  return elf;
}

// 通过 函数调用发生的地址 查找 调用者函数 名字
const char *elf_lookup_symbol(uint32_t addr, elf_t *elf) {
  int i;
  // 遍历内核elf文件中所有的符号
  for (i = 0; i < (elf->symtabsz / sizeof(elf_symbol_t)); i++) {
    if (ELF32_ST_TYPE(elf->symtab[i].info) != 0x2) {
      continue;
    }
    // 若调用地址在某个函数的地址范围内，则调用者就是该函数
    if ((addr >= elf->symtab[i].value) &&
        (addr < (elf->symtab[i].value + elf->symtab[i].size))) {
      return (const char *)((uint32_t)elf->strtab + elf->symtab[i].name);
    }
  }
  return NULL;
}