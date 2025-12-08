CXX=clang++
OPENOCD=openocd
EXAMPLE=HDMI

all: build/examples/$(EXAMPLE).elf

build/librp2350.a: build/interrupts.s.o
	ar -r $@ $<

build/%.s.o: include/rp2350/%.s
	mkdir -p build build/examples
	$(CXX) @compile_flags.txt -xassembler -c -o $@ $<

clean:
	rm -rf build/

# Examples

build/examples/%.elf: build/examples/%.cc.o layout.ld build/librp2350.a link_flags.txt
	$(CXX) @link_flags.txt -o $@ $<

build/examples/%.cc.o: examples/%.cc include/**/*.h layout.ld compile_flags.txt
	mkdir -p build build/examples
	$(CXX) @compile_flags.txt -I.. -o $@ -c $<

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
flash: build/examples/$(EXAMPLE).elf
	echo "program $< verify reset" | nc localhost 4444

# Debugging etc.

lldb: build/examples/$(EXAMPLE).elf
	lldb $< \
		-O "platform select remote-gdb-server" \
		-O "platform connect connect://localhost:3333"

gdb: build/examples/$(EXAMPLE).elf
	gdb $< \
		-ex "target extended-remote localhost:3333" \
		-ex "b hardFault" \

dump: build/examples/$(EXAMPLE).elf
	llvm-readelf --all $<
	llvm-objdump -ds --debug-file-directory=/foo $<
