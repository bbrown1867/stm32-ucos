# Workflow helper, build is specified in CMakeLists.txt

.PHONY: build clean gdb-server gdb-client serial-console format

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

serial-console:
	python3 -m serial /dev/cu.usbmodem1103 115200 --raw

ASTYLE_OPTS  = -n --style=allman -s4
ASTYLE_OPTS += --break-blocks --pad-oper --pad-header
format:
	astyle $(ASTYLE_OPTS) --recursive Source/*.c,*.h --exclude=Source/os_app_hooks
	astyle $(ASTYLE_OPTS) BSP/ST/STM32F7xx_Nucleo_144/*.c,*.h
	astyle $(ASTYLE_OPTS) BSP/ST/STM32F7xx_Nucleo_144/WeatherShield/*.c,*.h
