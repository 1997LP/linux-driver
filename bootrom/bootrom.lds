MEMORY { .srom : ORIGIN = 0x00000000,
                LENGTH = 0x40000 }
MEMORY { .sram : ORIGIN = 0x30040000,
                LENGTH = 0x4000 }

OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64")
OUTPUT_ARCH(aarch64)
ENTRY(_start)
SECTIONS
{
	. = 0;
	.text :
	{
		start.o (.text*)
		*(.text*)
	} >.srom

	. = ALIGN(8);
	.rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) } >.srom

	. = ALIGN(8);
	.data : {
		*(.data*)
	} >.sram

	. = ALIGN(8);

	_end = .;

	. = ALIGN(8);

	.bss_start (NOLOAD) : {
		KEEP(*(.__bss_start));
	} >.sram

	.bss (NOLOAD) : {
		*(.bss*)
		 . = ALIGN(8);
	} >.sram

	.bss_end (NOLOAD) : {
		KEEP(*(.__bss_end));
	} >.sram

	/DISCARD/ : { *(.dynsym) }
	/DISCARD/ : { *(.dynstr*) }
	/DISCARD/ : { *(.dynamic*) }
	/DISCARD/ : { *(.plt*) }
	/DISCARD/ : { *(.interp*) }
	/DISCARD/ : { *(.gnu*) }
}
