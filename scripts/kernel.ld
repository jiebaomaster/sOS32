/*
 *      kernel.ld -- 针对 kernel 格式所写的链接脚本
 */

ENTRY(start)
SECTIONS
{
    /* 每段都对齐到4096(页) */
    /* 辅助代码，初始化临时页表并切换到分页模式， */
    PROVIDE( kern_start = 0xC0100000 );
    . = 0x100000;
    .init.text :
    {
        *(.init.text)
        . = ALIGN(4096);
    }
    .init.data :
    {
        *(.init.data)
        . = ALIGN(4096);
    }
    
    /*一般内核代码的虚拟地址从0xC0000000开始*/
    . += 0xC0000000;
    /*
     * AT（addr）：实现存放地址和加载地址不一致的功能，
     * AT表示在文件中存放的位置，而在内存里按照普通方式存储
     */
    .text : AT(ADDR(.text) - 0xC0000000)
    {
        *(.text)
        . = ALIGN(4096);
    }

    .data : AT(ADDR(.data) - 0xC0000000)
    {
        *(.data)
        *(.rodata)
        . = ALIGN(4096);
    }

    .bss : AT(ADDR(.bss) - 0xC0000000)
    {
        *(.bss)
        . = ALIGN(4096);
    }

    .stab : AT(ADDR(.stab) - 0xC0000000)
    {
        *(.stab)
        . = ALIGN(4096);
    }
    
    .stabstr : AT(ADDR(.stabstr) - 0xC0000000)
    {
        *(.stabstr)
        . = ALIGN(4096);
    }
    PROVIDE( kern_end = . );

    /DISCARD/ : { *(.comment) *(.eh_frame) }
}