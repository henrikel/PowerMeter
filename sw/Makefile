# Name: Makefile
# Author: <insert your name here>
# Copyright: <insert your copyright message here>
# License: <insert your license reference here>

# This is a prototype Makefile. Modify it according to your needs.
# You should at least check the settings for
# DEVICE ....... The AVR device you compile for
# CLOCK ........ Target AVR clock rate in Hertz
# OBJECTS ...... The object files created from your source files. This list is
#                usually the same as the list of source files with suffix ".o".
# PROGRAMMER ... Options to avrdude which define the hardware you use for
#                uploading to the AVR and the interface where this hardware
#                is connected. We recommend that you leave it undefined and
#                add settings like this to your ~/.avrduderc file:
#                   default_programmer = "stk500v2"
#                   default_serial = "avrdoper"
# FUSES ........ Parameters for avrdude to flash the fuses appropriately.

DEVICE     = attiny2313
CLOCK      = 8000000
PROGRAMMER = -c usbtiny -P usb
OBJECTS    = main.o
FUSES      = -U hfuse:w:0xde:m -U lfuse:w:0x62:m -U efuse:w:0xff:m

# ATtiny84 fuse bits used above (fuse bits for other devices are different!):
# Fuse high byte:
# 0xdf = 1 1 0 1   1 1 1 0 
#        ^ ^ ^ ^   ^ \-+-/
#        | | | |   |   +------ BODLEVEL0..2 (Brown-out Detector trigger level)
#        | | | |   +---------- EESAVE (set to 0 to preserve EEPROM over chip erase)
#        | | | +-------------- WDTON (Watchdog timer always on)
#        | | +---------------- SPIEN (if set to 1, serial programming is disabled)
#        | +------------------ DWEN (debugWIRE Enable)
#        +-------------------- RSTDISBL (if set to 0, RESET pin is disabled)
# Fuse low byte:
# 0xe2 = 1 1 1 0   0 0 1 0
#        ^ ^ \ /   \--+--/
#        | |  |       +------- CKSEL 3..0 (8M internal crystal oscillator)
#        | |  +--------------- SUT 1..0 (slowly rising power)
#        | +------------------ CKOUT (Clock output)
#        +-------------------- CKDIV8 (Divide clock by 8)
#
# Fuse extended byte:
# 0xff = 1 1 1 1   1 1 1 1
#        \------+------/ |
#               |        +--- SELFPRGEN (Self-Programming Enable)
#               +------------- Unused
#
# For computing fuse byte values for other devices and options see
# the fuse bit calculator at http://www.engbedded.com/fusecalc/


# Tune the lines below only if you know what you are doing:

AVRDUDE = avrdude $(PROGRAMMER) -p $(DEVICE)
CC = avr-gcc
CPP = avr-c++
CFLAGS = -Wall -Os -DF_CPU=$(CLOCK) -mmcu=$(DEVICE)

# symbolic targets:
all:	main.hex

.c.o:
	$(CC) -c $< -o $@

.S.o:
	$(CC) -x assembler-with-cpp -c $< -o $@
# "-x assembler-with-cpp" should not be necessary since this is the default
# file type for the .S (with capital S) extension. However, upper case
# characters are not always preserved on Windows. To ensure WinAVR
# compatibility define the file type manually.

%.o : %.c
	$(CC) $(CFLAGS) -c $<
    
%.o : %.cpp
	$(CPP) $(CFLAGS) -c $<

.c.s:
	$(CC) $(CFLAGS) -S $< -o $@

flash:	all
	$(AVRDUDE) -U flash:w:main.hex:i

fuse:
	$(AVRDUDE) $(FUSES)

# Xcode uses the Makefile targets "", "clean" and "install"
install: flash fuse

# if you use a bootloader, change the command below appropriately:
load: all
	bootloadHID main.hex

clean:
	rm -f main.hex main.elf $(OBJECTS)

# file targets:
main.elf: $(OBJECTS)
	$(CC) $(CFLAGS) -o main.elf $(OBJECTS)

main.hex: main.elf
	rm -f main.hex
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size --format=avr --mcu=$(DEVICE) main.elf
# If you have an EEPROM section, you must also create a hex file for the
# EEPROM and add it to the "flash" target.

# Targets for code debugging and analysis:
disasm:	main.elf
	avr-objdump -d main.elf

cpp:
	$(CC) $(CFLAGS) -E main.c
