CXX=clang++
OPENOCD=/opt/openocd/bin/openocd

.PHONY: all clean

all: main

pre:
	mkdir -p build

main: pre
	@$(MAKE) target
	
target: build/Test.elf

build/Test.elf: \
                layout.ld compile_flags.txt \
                build/Test.cc.o build/librp2350.a
  # -ffreestanding
	clang++ 			\
		-fuse-ld=lld           \
		-target arm-none-eabi	\
		-Wl,-T,layout.ld        \
		-nostdlib		\
		-g -Wl,--gdb-index	\
		-gsplit-dwarf		\
		-L build 		\
		-lrp2350 		\
		-o $@ 		        \
		build/Test.cc.o

build/Test.cc.o: Test.cc rp2350/*.h compile_flags.txt
	clang++ @compile_flags.txt -c -o $@ $<

build/librp2350.a: \
		build/Faults.s.o
	ar -r $@ $<

build/%.cc.o: rp2350/%.cc rp2350/*.h compile_flags.txt
	clang++ @compile_flags.txt -c -o $@ $<

build/%.s.o: rp2350/%.s
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
	$(OPENOCD) 										\
		-f interface/cmsis-dap.cfg 	\
		-f target/rp2350.cfg 				\
		-c "adapter speed 1000"

flash: build/hdmi2350.elf
	# Uploads .ELF directly (not bin / uf2) via openocd
	echo "program build/hdmi2350.elf verify reset" | nc localhost 4444

lldb: build/hdmi2350.elf
	# platform select remote-gdb-server
	# platform connect connect://localhost:3333
	lldb build/hdmi2350.elf

gdb:
	# target extended-remote localhost:3333
	gdb build/hdmi2350.elf
