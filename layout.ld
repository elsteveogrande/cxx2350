MEMORY {
  FLASH(rx)       : ORIGIN = 0x10000000, LENGTH = 2048k
  SRAM(rw)        : ORIGIN = 0x20000000, LENGTH =  508k
}

__sram_begin = ORIGIN(SRAM);
__sram_end = ORIGIN(SRAM) + LENGTH(SRAM);

ENTRY(0) /* this isn't used for anything (other than to silence a warning) */

SECTIONS {

  /* Must be the first thing in flash at 0x10000000 */
  .bootv (0x10000000) : {
    LONG(__stack+1024) LONG(__reset)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
    LONG(__abort)      LONG(__abort)      LONG(__abort)      LONG(__abort)
  } > FLASH

  .image_def (0x10000080) : {
    *(.image_def*)
  } > FLASH

  . = ALIGN(64);
  .ARM.exidx : {
    *(.ARM.exidx*)
  } > FLASH

  . = ALIGN(64);
  .rodata    : {
    *(.rodata*)
  } > FLASH

  . = ALIGN(64);
  .init_array : {
    __init_array_begin = .;
    *(.init_array*)
    __init_array_end = .;
  } > FLASH

  . = ALIGN(64);
  .text      : {
    *(.systext*)
    *(.init*)
    *(.text*)
  } > FLASH

  . = ALIGN(64);
  __data_flash_begin = .;
  .data : {
    __data_sram_begin = .;
    *(.sysdata*)
    *(.data*)
    __data_sram_end = .;
  } > SRAM AT> FLASH

  . = ALIGN(64);
  .bss (NOLOAD) : {
    __bss_begin = .;
    *(.data*)
    *(.bss*)
    __bss_end = .;
  } > SRAM

  . = ALIGN(64);
  __heap = .;

}
