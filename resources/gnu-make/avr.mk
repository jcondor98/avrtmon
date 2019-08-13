# avrtmon
# GNU Make common rules
# All rights to Prof. Giorgio Grisetti

# AVR Compiler and Programmer setup
CC=avr-gcc
AS=avr-gcc
INCLUDE=-Iinclude/
CC_OPTS=-Wall --std=gnu99 -DF_CPU=16000000UL -O3 -funsigned-char \
		-funsigned-bitfields  -fshort-enums -Wall -Wstrict-prototypes \
		-mmcu=atmega2560 $(INCLUDE)  -D__AVR_3_BYTE_PC__
AS_OPTS=-x assembler-with-cpp $(CC_OPTS)

AVRDUDE=avrdude
# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT = /dev/ttyACM0    # programmer connected to serial device

# Beware of the second last line, I use ArchLinux. Locate the gemma config file
# for your distro/installation-of-avrdude
AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET):i
AVRDUDE_FLAGS = -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -D -q -V -C /usr/share/arduino/hardware/archlinux-arduino/avr/bootloaders/gemma/avrdude.conf
#AVRDUDE_FLAGS += -D -q -V -C /usr/share/arduino/hardware/tools/avr/bootloaders/gemma/avrdude.conf
AVRDUDE_FLAGS += -c wiring


# Objects and binaries rules
%.o:	%.c 
	$(CC) $(CC_OPTS) -c  -o $@ $<

%.o:	%.s 
	$(AS) $(AS_OPTS) -c  -o $@ $<

%.elf:	%.o $(OBJECTS)
	$(CC) $(CC_OPTS) -o $@ $< $(OBJECTS) $(LIBS)

%.hex:	%.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$@:i #$(AVRDUDE_WRITE_EEPROM) 


.phony:	clean all

all:	$(BINS) 

clean:	
	rm -rf $(OBJECTS) $(BINS) *.hex *~ *.o

.SECONDARY:	$(OBJECTS)
