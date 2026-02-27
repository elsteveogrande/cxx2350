CXX=clang++
OPENOCD=openocd
EXAMPLE=ychdmi2354

all: build/examples/$(EXAMPLE).elf

.PRECIOUS: build/examples/$(EXAMPLE).cc.o

build/examples/%.elf: build/examples/%.cc.o link_flags.txt layout.ld
	$(CXX) @link_flags.txt -o $@ $<

build/examples/%.cc.o: examples/%.cc include/**/* compile_flags.txt
	mkdir -p build/examples
	$(CXX) @compile_flags.txt -I.. -o $@ -c $<

clean:
	rm -rf build/

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
		-ex "hb rp2350::hardFault" \
		-ex "hb __start" \

dump: build/examples/$(EXAMPLE).elf
	llvm-objdump -f --headers $<
	llvm-nm --demangle $< | sort
	llvm-objdump                    \
		-s                            \
		--section=.bootv              \
		--section=.image_def          \
		--section=.init_array         \
		--section=.rodata             \
		--section=.data               \
		$<
	llvm-objdump -d --demangle --source --section=.text $<
