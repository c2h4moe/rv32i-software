MEMORY
{
  rom (rxa!w) : ORIGIN = 0x8000000, LENGTH = 4K
  ram (rwa) : ORIGIN = 0x0000000, LENGTH = 4K
}
SECTIONS
{
    . = 0x0000;
    .data : { *(.rodata .rodata.*)
              *(.data .data.*)
              *(.sdata .sdata.*)
              *(.bss .bss.*)
              *(.sbss .sbss.*) } > ram

    . = 0x8000000;
    .text : { runtime/_start.o(.text)
              *(.text, .text.*) } > rom
}
ENTRY(_start)
