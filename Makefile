CC:=arm-none-eabi-gcc

DEVICE_NAME:=LT-7AB-27A

CFLAGS:=\
-ffreestanding\
-nostartfiles\
-Wall\

CFLAGS_CM4_AAPCS_FPV4_SP_D16:=\
-mcpu=cortex-m4\
-mthumb\
-mabi=aapcs\
-mfloat-abi=hard\
-mfpu=fpv4-sp-d16\
-D_DESC_ARCHCPU=4\
-D_DESC_ARCHFLAGS=1\

leds.elf: main.c leds.h linker_script.ld Makefile
	arm-none-eabi-gcc $(CFLAGS) $(CFLAGS_CM4_AAPCS_FPV4_SP_D16) -T linker_script.ld main.c -o $@

install: leds.elf
	uapputil install -i leds.elf -n $(DEVICE_NAME)

run: leds.elf
	uapputil run -i leds.elf -n $(DEVICE_NAME)
