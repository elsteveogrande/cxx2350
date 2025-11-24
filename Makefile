CXX=clang++
OPENOCD=openocd

all: build/examples/Blink.elf

build/examples/Blink.elf: \
      layout.ld compile_flags.txt \
      build/examples/Blink.cc.o build/librp2350.a
	clang++                  \
		-fuse-ld=lld           \
		-target arm-none-eabi  \
		-Wl,-T,layout.ld       \
		-nostdlib              \
		-g -Wl,--gdb-index     \
		-gsplit-dwarf          \
		-L build               \
		-lrp2350               \
		-ffreestanding         \
		-o $@                  \
		build/examples/Blink.cc.o

build/examples/Blink.cc.o: examples/Blink.cc include/**/*.h compile_flags.txt
	mkdir -p build build/examples
	clang++ @compile_flags.txt -c -o $@ $<

build/librp2350.a: build/faults.s.o
	ar -r $@ $<

# build/%.cc.o: rp2350/%.cc include/**/*.h compile_flags.txt
# 	clang++                    \
# 	  @compile_flags.txt       \
# 	  -Wunused -Werror=unused  \
# 	  -c -o $@ $<

build/faults.s.o: include/rp2350/faults.s
	mkdir -p build build/examples
	clang++ @compile_flags.txt -xassembler -c -o $@ $<

clean:
	rm -rf build/

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

flash: build/examples/Blink.elf
	echo "program build/examples/Blink.elf verify reset" | nc localhost 4444

lldb: build/examples/Blink.elf
	lldb \
		build/examples/Blink.elf \
		-O "platform select remote-gdb-server" \
		-O "platform connect connect://localhost:3333"

gdb:
	gdb \
		build/examples/Blink.elf \
		-ex "target extended-remote localhost:3333"

dump: build/examples/Blink.elf
	llvm-readelf --all $<
	llvm-objdump -ds --debug-file-directory=/foo $<
