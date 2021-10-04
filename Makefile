# Workflow helper, build is specified in CMakeLists.txt

.PHONY: build clean gdb-server gdb-client

all: clean build

build:
	mkdir -p build
	cd build && cmake .. && make

clean:
	rm -rf build/

gdb-server:
	openocd -f ./openocd.cfg

gdb-client: 
	arm-none-eabi-gdb -tui build/main.elf
