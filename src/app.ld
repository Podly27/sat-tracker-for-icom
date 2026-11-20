ENTRY(app_main)
SECTIONS {
 . = 0x20831000;
 .text : { KEEP(*(.entry)) *(.text*) *(.rodata*) }
 .ARM.exidx : { *(.ARM.exidx*) }
 .data : { *(.data*) *(.got*) }
 . = ALIGN(4); __image_end = .;
 .bss (NOLOAD) : { *(.bss*) *(COMMON) }
 . = ALIGN(4); __bss_end = .;
 /DISCARD/ : { *(.comment*) *(.note*) *(.ARM.attributes*) }
}
