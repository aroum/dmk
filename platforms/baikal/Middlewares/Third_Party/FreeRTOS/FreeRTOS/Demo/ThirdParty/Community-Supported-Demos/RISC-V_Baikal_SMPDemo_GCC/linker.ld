/* Output format */
OUTPUT_ARCH(riscv)
OUTPUT_FORMAT(elf32-littleriscv)

/* Entry point of the program */
ENTRY(_start)

/* Memory regions */
/*  Hardware dependent values
    See MCU specification for more info */
MEMORY {
    TCMA        (rwx) : ORIGIN = 0x40010000, LENGTH = 64K
    TCMB        (rwx) : ORIGIN = 0x40020000, LENGTH = 96K
    FRONT_PORT0 (rw)  : ORIGIN = 0x50010000, LENGTH = 64K
    FRONT_PORT1 (rw)  : ORIGIN = 0x60000000, LENGTH = 256K
    SRAM        (rwx) : ORIGIN = 0x70000000, LENGTH = 32K
    XIP0_FLASH  (rx)  : ORIGIN = 0x80000000, LENGTH = 16M
    XIP1_FLASH  (rx)  : ORIGIN = 0x90000000, LENGTH = 16M
    EFLASH      (rx)  : ORIGIN = 0xA0000000, LENGTH = 256K
}

/* Heap and stack size */
__HEAP_SIZE = 0x00001000;
__STACK_SIZE = 0x00001000;

/* Program sections */
SECTIONS
{
    .text :
    {
        /* Code section */
        _stext = .;
        *(.text.start)
        . = _stext + 0x20;
        KEEP(*(.text.magic))
        KEEP(*(.text.isr_vector))
        *(.text)
        *(.text.*)
        _etext = .;
    } > TCMA

    .text1 :
    {
        *(.text.start)
        KEEP(*(.text1.isr_vector))
        *(.text1)
        *(.text1.*)
    } > TCMA

    .rodata :
    {
        /* Read-only data section */
        . = ALIGN(4);
        *(.srodata)
        *(.srodata.*)
        *(.rodata)
        *(.rodata.*)
    } > TCMA

    .note.gnu.build-id : ALIGN(4) {
        KEEP(*(.note.gnu.build-id))
    } > TCMA

    .data :
    {
        /* Initialized data section */
        . = ALIGN(4);
        _sdata = .;
        *(.sdata)
        *(.sdata.*)
        *(.data)
        *(.data.*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > TCMA

    _sidata = LOADADDR(.data);

    .bss (NOLOAD) :
    {
        /* Uninitialized data section */
        . = ALIGN(4);
        _sbss = .;
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM

    .heap (NOLOAD) :
    {
        /* Heap section */
        . = ALIGN(4); /* Ensure alignment of the heap */
        _heap_start = .;
        . += __HEAP_SIZE;
        _heap_end = .;
    } > SRAM

    .stack (NOLOAD) :
    {
        /* Stack section */
        . = ALIGN(16); /* Ensure alignment of the stack */
        . += __STACK_SIZE / 2;
        __StackOneTop = .;
        . += __STACK_SIZE / 2;
        __StackTop = .;
    } > SRAM

    .spinlock (NOLOAD) : ALIGN(4) {
        *(.spinlock)
        *(.spinlock.*)
    } > FRONT_PORT0

}
