CXX=clang++
OPENOCD=openocd
EXAMPLE=HDMI

all: build/examples/$(EXAMPLE).elf

# Eventually, drop the .a and .s.o rules

build/librp2350.a: build/interrupts.s.o
	ar -r $@ $<

build/%.s.o: include/rp2350/asm/%.s
	mkdir -p build build/examples
	$(CXX) @compile_flags.txt -xassembler -c -o $@ $<

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
		-ex "b hardFault" \

dump: build/examples/$(EXAMPLE).elf
	llvm-readelf --all $<
	llvm-objdump -ds --debug-file-directory=/foo $<

# Examples

build/examples/HDMI.elf: build/examples/HDMI.cc.o layout.ld build/librp2350.a examples/link_flags.txt examples/HDMI.Image.h
	mkdir -p build build/examples
	$(CXX) @examples/link_flags.txt -o $@ $<

build/examples/%.cc.o: examples/%.cc include/**/* compile_flags.txt
	mkdir -p build build/examples
	mkdir -p build build/examples
	$(CXX) @compile_flags.txt -I.. -o $@ -c $<

examples/HDMI.Image.h: misc/testpattern.864.486.png misc/hstxpixels.py
	mkdir -p build build/examples
	magick -size 864x486 $< pnm:- \
	    | python3 misc/hstxpixels.py testPattern > $@

misc/testpattern.864.486.png: misc/testpattern.svg
	rsvg-convert -f png --width 864 --height 486 -o $@ $<

misc/testpattern.svg: misc/testpattern.PM5644.svg
	cat $< \
		| sed 's/#0000bf/#0000f4/gi' \
		| sed 's/#00bf00/#00f400/gi' \
		| sed 's/#00bfbf/#00f4f4/gi' \
		| sed 's/#bf0000/#f40000/gi' \
		| sed 's/#bf00bf/#f400f4/gi' \
		| sed 's/#bfbf00/#f4f400/gi' \
		| sed 's/#bfbfbf/#f4f4f4/gi' \
		> $@

misc/testpattern.PM5644.svg:
	curl -sf https://upload.wikimedia.org/wikipedia/commons/c/c1/PM5644.svg > $@
