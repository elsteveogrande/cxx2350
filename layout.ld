/* RP2350 link script */

MEMORY {
  FLASH(rx)       : ORIGIN = 0x10000000, LENGTH = 2048k
  STACK(rw)       : ORIGIN = 0x20000100, LENGTH =    3k /* sp = 0x20000bf8 */
  PANICSTACK(rw)  : ORIGIN = 0x20000c00, LENGTH =    1k /* sp = 0x20001000 when panicking */
  BSS(rw)         : ORIGIN = 0x20001000, LENGTH =   60k
  HEAP(rw)        : ORIGIN = 0x20010000, LENGTH =  448k
  BUF_X(rw)       : ORIGIN = 0x20080000, LENGTH =    4k
  BUF_Y(rw)       : ORIGIN = 0x20081000, LENGTH =    4k
}

__reset_sp   = ORIGIN(STACK) + SIZEOF(STACK) - (4 * 2);
__heap_base  = ORIGIN(HEAP);
__heap_end   = ORIGIN(HEAP) + LENGTH(HEAP);
__buf_x_base = ORIGIN(BUF_X);
__buf_x_end  = ORIGIN(BUF_X) + LENGTH(BUF_X);
__buf_y_base = ORIGIN(BUF_Y);
__buf_y_end  = ORIGIN(BUF_Y) + LENGTH(BUF_Y);

/* Not actually used but this will silence warnings */
ENTRY(0x10000000)

SECTIONS {
  .vec_table  (0x10000000)  : {} > FLASH
  .image_def  (0x10000100)  : {} > FLASH

  .text                     : {*(.text*)} > FLASH

  .init_array               : {
    __init_array_base = .;
    *(.init_array*)
    __init_array_end = .;
  } > FLASH

  .rodata                   : {*(.rodata*)} > FLASH

  .bss       (NOLOAD)       : {
    __bss_base = .;
    *(.data*)
    *(.bss*)
    __bss_end = .;
  } > BSS

  .heap      (NOLOAD)       : {} > HEAP
}
