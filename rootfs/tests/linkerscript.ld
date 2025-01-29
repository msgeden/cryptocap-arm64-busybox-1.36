SECTIONS {
  /* Set the start of the file */
  . = 0x400000;

  /* Provide __ehdr_start at the beginning of the file */
  PROVIDE (__ehdr_start = .);

  /* Place .fixed_section at 0x400000 */
  .fixed_section 0x400000 :
  {
    KEEP(*(.fixed_section))
  } :text

  .text : {
    *(.text*)
  } :text

  .rodata : {
    *(.rodata*)
  }

  /* Provide .init_array, .fini_array, and .preinit_array for static linking */
  .preinit_array :
  {
    __preinit_array_start = .;
    KEEP (*(.preinit_array*))
    __preinit_array_end = .;
  }

  .init_array :
  {
    __init_array_start = .;
    KEEP (*(.init_array*))
    __init_array_end = .;
  }

  .fini_array :
  {
    __fini_array_start = .;
    KEEP (*(.fini_array*))
    __fini_array_end = .;
  }



  .data : {
    *(.data*)
  } :data

  .bss : {
    *(.bss*)
  } :data

    /* You may also need to define __ehdr_start if required. Typically, this points
     to the start of the ELF header. One approach is to place it at the start
     of the first loadable segment. If you're using PT_PHDR and PHDRS, you might
     do something like:
     
     PROVIDE (__ehdr_start = 0x400000); 
     
     or ensure that __ehdr_start = SIZEOF_HEADERS; depends on your setup.
  */
}

PHDRS {
  text PT_LOAD FLAGS(RX);
  data PT_LOAD FLAGS(RW);
}
