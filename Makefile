CXX=clang++
OPENOCD=openocd

build/librp2350.a: build/faults.s.o
	ar -r $@ $<

build/faults.s.o: include/rp2350/faults.s
	mkdir -p build build/examples
	clang++ @compile_flags.txt -xassembler -c -o $@ $<

clean:
	rm -rf build/

# Examples

build/examples/%.elf: \
      examples/%.cc include/**/*.h layout.ld compile_flags.txt build/librp2350.a
	mkdir -p build build/examples
	clang++                  \
		@compile_flags.txt     \
		-fuse-ld=lld           \
		-flto                  \
		-target arm-none-eabi  \
		-Wl,-T,layout.ld       \
		-nostdlib              \
		-g -Wl,--gdb-index     \
		-gsplit-dwarf          \
		-L build               \
		-lrp2350               \
		-ffreestanding         \
		-o $@                  \
		$<

# Programming flash

start_openocd:
	# See "Debug with a second Pico or Pico 2" here:
	#   https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf
	#
	# NOTE:  My brew-installed openocd failed to establish a connection with lldb.
	# A build from source worked better: get openocd from git, and do:
	#   brew install automake jimtcl ; cd openocd ; ./bootstrap ; ./configure --prefix=/opt
	#
	$(OPENOCD)                    \
		-f interface/cmsis-dap.cfg  \
		-f target/rp2350.cfg        \
		-c "adapter speed 1000"

# (See also `flash.sh`)
flash: build/examples/UARTHello.elf
	echo "program $< verify reset" | nc localhost 4444

# Debugging etc.

lldb: build/examples/UARTHello.elf
	lldb $< \
		-O "platform select remote-gdb-server" \
		-O "platform connect connect://localhost:3333"

gdb: build/examples/UARTHello.elf
	gdb $< \
		-ex "target extended-remote localhost:3333" \
		-ex "b hardFault" \

dump: build/examples/UARTHello.elf
	llvm-readelf --all $<
	llvm-objdump -ds --debug-file-directory=/foo $<
