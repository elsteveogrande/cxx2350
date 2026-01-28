# cxx2350


### C++20 library for the Raspberry Pi RP2350 and Pico2


This project aims to provide a simple C++ library supporting the Pico2.

The [official SDK](https://github.com/raspberrypi/pico-sdk) is not required for this.
The SDK is supported by Raspberry Pi and is of course complete in terms of hardware support.

I found the SDK a bit hard to work with since I'm not good enough at using CMake,
I don't particularly like using C, and the code itself is somewhat hard to read unless you already know what you're doing.

This alternate SDK aims to provide a header-only representation to make integration
as simple as possible.  It's C++-native so C++ structs and features make things a little easier (if you like C++).

See the `examples/` directory and `Makefile` to see how this all comes together, and how you can integrate this into your projects.


## Requirements

To compile:

* `make`
* `clang++` supporting C++20 and up

To run on a Pico2:

* your target Pico2 board
* a spare Pico1 or Pico2 running [debugprobe](https://github.com/raspberrypi/debugprobe)
* `openocd` to flash and debug the board

To debug:

* `make gdb` or `make lldb` to debug board (while `make start_openocd` is running)
* `make dump` to print out the ELF binary via `llvm-objdump` and `llvm-readelf`


## Build and flash

* `make start_openocd` in one terminal window, and then:
* `make flash` to upload it to board
* `make gdb` or `make lldb` to start debugger


## Current limitations

* Not all peripherals are supported; working on this.
* RISC-V not yet supported
* The 2nd core is not yet supported
* Currently there's a bug (?) where `make flash`, when trying to upload the ELF via openocd
  will fail with `stalled AP operation, issuing ABORT`, `DP initialisation failed`.
  To work around this I remove power from the target board, reconnect power
  while holding the `BOOTSEL` button.  Inconvenient, and only sometimes works.
* Right now this only supports the Clang toolchain
* I want to implement write-combining on hardware registers,
  so that each individual field within a register doesn't become its own
  read-then-AND-some-bits-OR-more-bits-then-write instruction sequence (which eats code bytes as well as cycles)

The following section lists things I consider complete / partially complete / TODO.


## Support matrix

At present we have only a small subset of all this.
(It's literally _just_ enough to blink the LED.)
As I get ideas or discover something is needed, I'm adding to this TODO list.

I'm adding bits and pieces of this as I go, but ultimately want to
achieve a decent portion of the [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html).


## "Standard" C / C++ things

Since this uses no standard libraries or SDK whatsoever, this needs to implement everything itself.

I usd air-quotes around "standard" because I'm not aiming for an ISO C++-compliant library.
Though one day it would be nice to have something resembling `array` or `vector` even if it's not the same as standard C++.


|                | File         | Notes                                               |    | Issue
|----------------|--------------|-----------------------------------------------------|----|----
| Types          | `base.h`     | `size_t`, `uint32_t`, etc.                          | Ⓧ |
| Memory         | `memory.h`   | `malloc` and `free`                                 | Ⓧ |
|                |              | `operator new` and `delete`                         | Ⓧ |
| Panic/abort    | `panic.h`    | Dump info out to serial UART (also missing!)        | Ⓧ |
| Concurrency    | `mutex.h`    | mutex and lock_guard                                | Ⓧ |
| ABI            | `abi.h`      | `eabi_memcpy` etc.                                  | ✅ |
|                |              | `vtable`s (for virtual methods), `typeinfo`         | Ⓧ |
|                | `unwind.h`   | Stack unwinding                                     | Ⓧ |
|                | `exception.h`| exceptions                                          | Ⓧ |
| Thread-locals  |              | per-core `thread_local` objs                        | Ⓧ |
|                |              |                                                     | Ⓧ |
| "Standard" lib | `cxx.h`      | Semi-resembling a dumbed-down `std::`.  Essentials: | Ⓧ |
|                |              | `array` (a C array with compile-time-known size)    | Ⓧ |
|                |              | `vector` (a resizable array)                        | Ⓧ |
|                |              | `map` (an ordered map, likely binary tree), + `set` | Ⓧ |
|                |              | `unordered_map` (+ `_set`); i.e. hashtables         | Ⓧ |
|                |              | `string` and `string_view`                          | Ⓧ |
|                |              | polymorphic (`pmr`) `allocator`                     | Ⓧ |


## RP2350 hardware

https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf

| Datasheet Section                                               | File            |    | Issue
|-----------------------------------------------------------------|-----------------|----|----
| **3.1 SIO**                                                     |                 | partial |
| 3.1.2. CPUID                                                    |                 | Ⓧ |
| 3.1.3. GPIO control                                             | `gpio.h`        | ✅ |
| 3.1.4. Hardware spinlocks                                       |                 | Ⓧ |
| 3.1.5. Inter-processor FIFOs (Mailboxes)                        |                 | Ⓧ |
| 3.1.7. Integer divider                                          |                 | Ⓧ |
| 3.1.8. RISC-V platform timer                                    |                 | Ⓧ |
| 3.1.9. TMDS encoder                                             |                 | Ⓧ |
| 3.1.10. Interpolator                                            |                 | Ⓧ |
||||
| **3.2. Interrupts**                                             | `interrupts.{h,s}`  | ✅ |
| 3.2.1. Non-maskable interrupt (NMI)                             |                 | Ⓧ |
||||
| **3.3. Event signals (Arm)**                                    |                 | Ⓧ |
||||
| **3.4. Event signals (RISC-V)**                                 |                 | Ⓧ |
||||
| **3.5. Debug**                                                  |                 | Ⓧ |
| 3.5.5. Software control of SWD pins                             |                 | Ⓧ |
||||
| **3.6. Cortex-M33 coprocessors**                                |                 | Ⓧ |
| 3.6.1. GPIO coprocessor (GPIOC)                                 |                 | Ⓧ |
| 3.6.2. Double-precision coprocessor (DCP)                       |                 | Ⓧ |
| 3.6.3. Redundancy coprocessor (RCP)                             |                 | Ⓧ |
| 3.6.4. Floating point unit                                      |                 | Ⓧ |
||||
| **3.7. Cortex-M33 processor**                                   | `m33.h`         | partial |
||||
| **3.8. Hazard3 processor**                                      |                 | Ⓧ |
||||
| **3.9. Arm/RISC-V architecture switching**                      |                 | Ⓧ |
||||
| **6. Power**                                                    |                 | Ⓧ |
||||
| **7. Resets**                                                   | `resets.h`      | ✅ |
||||
| **8. Clocks**                                                   | `clocks.h`      | partial |
| 8.2. Crystal oscillator (XOSC)                                  | `clocks.h`      | ✅ |
| 8.3. Ring oscillator (ROSC)                                     |                 | Ⓧ |
| 8.4. Low Power oscillator (LPOSC)                               |                 | Ⓧ |
| 8.5. Tick generators                                            | `ticks.h`       | ✅ |
| 8.6. PLL                                                        | `clocks.h`      | ✅ |
||||
| **9. GPIO**                                                     | `gpio.h`        | ✅ |
||||
| **10. Security**                                                |                 | Ⓧ |
||||
| **11. PIO**                                                     |                 | Ⓧ |
||||
| **12. Peripherals**                                             |                 | Ⓧ |
| 12.1. UART                                                      |                 | Ⓧ |
| 12.2. I2C                                                       |                 | Ⓧ |
| 12.3. SPI                                                       |                 | Ⓧ |
| 12.4. ADC and Temperature Sensor                                |                 | Ⓧ |
| 12.5. PWM                                                       |                 | Ⓧ |
| 12.6. DMA                                                       |                 | Ⓧ |
| 12.7. USB                                                       |                 | Ⓧ |
| 12.8. System Timers                                             |                 | Ⓧ |
| 12.9. Watchdog                                                  |                 | Ⓧ |
| 12.10. Always-on Timer                                          |                 | Ⓧ |
| 12.11. HSTX                                                     |                 | Ⓧ |
| 12.12. TRNG                                                     |                 | Ⓧ |
| 12.13. SHA-256 accelerator                                      |                 | Ⓧ |
| 12.14. QSPI memory interface (QMI)                              |                 | Ⓧ |
| 12.15.1 SYSINFO (System Control Registers)                      |                 | Ⓧ |
| 12.15.2 SYSCFG (System Control Registers)                       |                 | Ⓧ |
| 12.15.3 TBMAN (System Control Registers)                        |                 | Ⓧ |
| 12.15.4 BUSCTRL (System Control Registers)                      |                 | Ⓧ |


## RP2350 Errata

A list of errata related to this chip, so we're conscious of anything that might need to be addressed in code as workarounds.

|            |                | Short description                                                                             | Workarounds implemented |
|------------|----------------|-----------------------------------------------------------------------------------------------|-------------------------|
| ACCESSCTRL | RP2350-E3      | (RP2350 A2, QFN-60) , GPIO_NSMASK controls wrong PADS registers                               |
| Bootrom    | RP2350-E10     | UF2 drag-and-drop doesn’t work with partition tables                                          | Not needed in `cxx2350`
|            | RP2350-E13     | explicitly invalid IMAGE_DEF followed by a valid IMAGE_DEF (in that order) fails to boot      |
|            | RP2350-E14     | bootrom connect_internal_flash() function always uses pin 0, ignoring FLASH_DEVINFO           |
|            | RP2350-E15     | bootrom otp_access() function applies incorrect access permission                             |
|            | RP2350-E18     | forever fail to boot if FLASH_PARTITION_SLOT_SIZE contains an invalid ECC                     |
|            | RP2350-E19     | reboot hangs if certain bits are set in FRCE_OFF when rebooting.                              |
|            | RP2350-E20     | ability to physically "glitch" the CPU at precise times, could cause unsigned code execution  |
|            | RP2350-E21     | glitching could potentially extract sensitive data from OTP                                   |
|            | RP2350-E22     | malformed "lollipop" block loop will cause a hang                                             |
|            | RP2350-E23     | PICOBOOT GET_INFO command always returns zero                                                 |
|            | RP2350-E24     | glitch , could cause unsigned code execution on a secured RP2350                              |
|            | RP2350-E25     | LOAD_MAP that uses non-word sizes doesn’t cause an error                                      |
| Bus Fabric | RP2350-E27     | Bus priority controls apply to wrong managers for APB and FASTPERI                            |
| DMA        | RP2350-E5      | Interactions between CHAIN_TO and ABORT of active channels                                    |
|            | RP2350-E8      | CHAIN_TO might not fire for zero-length transfers                                             |
| GPIO       | RP2350-E9      | Increased leakage current on Bank 0 GPIO when pad input is enabled                            |
| Hazard3    | RP2350-E4      | System Bus Access stalls indefinitely when core 1 is in clock-gated sleep                     |
|            | RP2350-E6      | PMPCFGx RWX fields are transposed                                                             |
|            | RP2350-E7      | U-mode doesn’t ignore mstatus.mie                                                             |
| OTP        | RP2350-E16     | USB_OTP_VDD disruption can result in corrupt OTP row read data                                | Not needed in `cxx2350`
|            | RP2350-E17     | Performing a guarded read on a single ECC OTP row causes a fault                              | Not needed in `cxx2350`
|            | RP2350-E28     | OTP keys for pages 62/63 are applied to all lock words 0 through 63                           | Not needed in `cxx2350`
| RCP        | RP2350-E26     | RCP random delays can create a side-channel                                                   |
| SIO        | RP2350-E1      | Interpolator OVERF bits are broken by new right-rotate behaviour                              |
|            | RP2350-E2      | SIO SPINLOCK writes are mirrored at +0x80 offset                                              |
| XIP        | RP2350-E11     | XIP cache clean by set/way operation modifies the tag of dirty lines                          |
| USB        | RP2350-E12     | Inadequate synchronisation of USB status signals                                              |



## Further reading

* https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
* https://developer.arm.com/documentation/100235/0100/The-Cortex-M33-Processor/Exception-model/Vector-table


## License

Open source and made available under the MIT License.
No warranties, no refunds, not for use in life-saving devices.
