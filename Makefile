CXX=clang++
OPENOCD=/opt/openocd/bin/openocd

all: build/Test.elf

build/Test.elf: \
      layout.ld compile_flags.txt \
      build/Test.cc.o build/librp2350.a
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
		build/Test.cc.o

build/Test.cc.o: Test.cc rp2350/*.h compile_flags.txt
	mkdir -p build
	clang++ @compile_flags.txt -c -o $@ $<

build/librp2350.a: \
      build/faults.s.o
	ar -r $@ $<

build/%.cc.o: rp2350/%.cc rp2350/*.h compile_flags.txt
	mkdir -p build
	clang++                    \
	  @compile_flags.txt       \
	  -Wunused -Werror=unused  \
	  -c -o $@ $<

build/%.s.o: rp2350/%.s
	mkdir -p build
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
	$(OPENOCD)   								\
		-f interface/cmsis-dap.cfg  \
		-f target/rp2350.cfg   			\
		-c "adapter speed 1000"

flash: build/Test.elf
	# Uploads .ELF directly (not bin / uf2) via openocd
	echo "program build/Test.elf verify reset" | nc localhost 4444

lldb: build/Test.elf
	# platform select remote-gdb-server
	# platform connect connect://localhost:3333
	lldb build/Test.elf

gdb:
	gdb \
		build/Test.elf \
		-ex "target extended-remote localhost:3333"

dump: build/Test.elf
	llvm-readelf --all $<  &&  llvm-objdump -ds --debug-file-directory=/foo $<

