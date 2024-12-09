MEMORY
{
  rom (rxa!w) : ORIGIN = 0x8000000, LENGTH = 16K
  ram (rwa) : ORIGIN = 0x0000000, LENGTH = 4K
}
SECTIONS
{
    . = 0x0000;
    .data : { *(.rodata .rodata.* .rdata .gnu.linkonce.r.*)
              *(.data .data.* .srodata* .gnu.linkonce.d.*)
              *(.sdata .sdata.*)
              *(.bss .bss.* .gnu.linkonce.b.*)
              *(.sbss .sbss.*) } > ram

    . = 0x8000000;
    .text : { runtime/_start.o(.text)
              *(.text, .text.* .gnu.linkonce.t.*) } > rom
}
ENTRY(_start)
