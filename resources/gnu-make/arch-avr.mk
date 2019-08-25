# avrtmon
# GNU Make common rules for AVR compilation
# All rights to Prof. Giorgio Grisetti - This file was also heavily modified

# AVR Compiler and Programmer setup
CC := avr-gcc
AS := avr-gcc
CFLAGS := -Wall --std=c99 -DF_CPU=16000000UL -O3 -funsigned-char \
		-funsigned-bitfields  -fshort-enums -Wstrict-prototypes \
		-mmcu=atmega2560 -I$(INCDIR) -D__AVR_3_BYTE_PC__
ASFLAGS := -x assembler-with-cpp $(CFLAGS)

AVRDUDE := avrdude
# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT := /dev/ttyACM0    # programmer connected to serial device
AVRDUDE_CONFIG != find /usr/share/arduino/hardware -name avrdude.conf

AVRDUDE_WRITE_FLASH := -U flash:w:$(TARGET):i
AVRDUDE_FLAGS := -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -D -q -V -C $(AVRDUDE_CONFIG)
AVRDUDE_FLAGS += -c wiring


# AVR-specific binaries recipes
%.elf:	%.o $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $< $(OBJECTS) $(LIBS)

%.hex:	%.elf
	avr-objcopy -O ihex -R .eeprom $< $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$@:i #$(AVRDUDE_WRITE_EEPROM) 
