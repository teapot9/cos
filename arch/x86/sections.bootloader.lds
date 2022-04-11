PHDRS
{
	hash PT_LOAD FLAGS(4);
	text PT_LOAD FLAGS(5); /* r-e */
	reloc PT_LOAD FLAGS(4);
	rdata PT_LOAD FLAGS(4); /* r-- */
	data PT_LOAD FLAGS(6); /* rw- */
	dyndata PT_LOAD FLAGS(6);
	dynamic PT_DYNAMIC FLAGS(6);
	notes PT_NOTE FLAGS(4);
}

SECTIONS
{
	. = 0x1000;
	.hash : { *(.hash) } :hash
	.gnu.hash : { *(.gnu.hash) } :hash
	. = ALIGN(4096);
	.eh_frame : { *(.eh_frame) } :rdata
	. = ALIGN(4096);
	.text :
	{
		_text = .;
		. = ALIGN(8);
		*(.multiboot)
		. = ALIGN(8);
		*(.text)
		*(.text.*)
		*(.gnu.linkonce.t.*)
		. = ALIGN(16);
	} :text
	_etext = .;
	_text_size = . - _text;
	. = ALIGN(4096);
	.reloc :
	{
		LONG(_data);
		LONG(10);
		SHORT(0);
		*(.reloc)
	} :reloc
	. = ALIGN(4096);
	.rdata :
	{
		_data = .;
		*(.got.plt)
		*(.got)
		*(.rodata*)
		*(.rdata*)
		*(.srodata)
		*(.gnu.linkonce.r.*)
		. = ALIGN(8);
		__initcalltest = .;
		KEEP(*(.initcall.test))
		__initcall_early = .;
		KEEP(*(.initcall.early))
		__initcall_core = .;
		KEEP(*(.initcall.core))
		__initcall_device = .;
		KEEP(*(.initcall.device))
		__initcall_misc = .;
		KEEP(*(.initcall.misc))
		__initcall_end = .;
		. = ALIGN(8);
		__constructors_start = .;
		KEEP(*(SORT(.ctors.*)))
		KEEP(*(.ctors))
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		__constructors_end = .;
	} :rdata
	. = ALIGN(4096);
	.data :
	{
		*(.data*)
		*(.sdata*)
		*(.gnu.linkonce.d.*)
	} :data
	. = ALIGN(4096);
	.bss :
	{
		*(.sbss)
		*(.scommon)
		*(.dynbss)
		*(.bss)
		*(.gnu.linkonce.b.*)
		*(COMMON)
	} :data
	. = ALIGN(4096);
	.note.gnu.build-id : { *(.note.gnu.build-id) } :notes

	_edata = .;
	_data_size = . - _etext;
	. = ALIGN(4096);
	.dynamic  : { *(.dynamic) } :dynamic :dyndata
	. = ALIGN(4096);
	.rel :
	{
		*(.rel.data)
		*(.rel.data.*)
		*(.rel.got)
		*(.rel.stab)
	} :reloc
	. = ALIGN(4096);
	.dynsym   : { *(.dynsym) } :dyndata
	. = ALIGN(4096);
	.dynstr   : { *(.dynstr) } :dyndata
	. = ALIGN(4096);
	/DISCARD/ :
	{
		*(.note*)
		*(.comment*)
	}
}
