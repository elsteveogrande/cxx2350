/* RP2350 link script */

MEMORY {
  FLASH(rx)       : ORIGIN = 0x10000000, LENGTH = 2048k
  STACK(rw)       : ORIGIN = 0x20000100, LENGTH =    3k /* sp = 0x20000c00 */
  PANICSTACK(rw)  : ORIGIN = 0x20000c00, LENGTH =    1k /* sp = 0x20001000 when panicking */
  BSS(rw)         : ORIGIN = 0x20001000, LENGTH =   60k
  RAM(rw)         : ORIGIN = 0x20010000, LENGTH =  448k
  RAM_X(rw)       : ORIGIN = 0x20080000, LENGTH =    4k
  RAM_Y(rw)       : ORIGIN = 0x20081000, LENGTH =    4k
}

__reset_sp    = ORIGIN(STACK) + SIZEOF(STACK);
__panic_sp    = ORIGIN(PANICSTACK) + SIZEOF(PANICSTACK);
__bss_base    = ORIGIN(BSS);
__bss_size    = SIZEOF(BSS);
__ram_base    = ORIGIN(RAM);
__ram_size    = SIZEOF(RAM);
__ram_x_base  = ORIGIN(RAM_X);
__ram_x_size  = SIZEOF(RAM_X);
__ram_y_base  = ORIGIN(RAM_Y);
__ram_y_size  = SIZEOF(RAM_Y);

/* Not actually used but this will silence warnings */
ENTRY(0x10000000)

SECTIONS {
  .vec_table  (0x10000000)  : {} > FLASH
  .image_def  (0x10000100)  : {} > FLASH
  .text                     : {} > FLASH
  .rodata                   : {} > FLASH
  .data       (NOLOAD)      : {} > BSS
  .bss        (NOLOAD)      : {} > BSS
}
