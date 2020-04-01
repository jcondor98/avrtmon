# avrtmon
# GNU Make common rules for AVR compilation
# All rights to Prof. Giorgio Grisetti - This file was also heavily modified

# AVR Compiler and Programmer setup
CC := avr-gcc
AS := avr-gcc
CFLAGS := -Wall --std=c99 -DF_CPU=16000000UL -O3 -funsigned-char \
		-funsigned-bitfields  -fshort-enums -Wstrict-prototypes \
		-mmcu=atmega2560 -I$(INCDIR)/avr -I$(INCDIR) -DAVR -D__AVR_3_BYTE_PC__
ASFLAGS := -x assembler-with-cpp $(CFLAGS)

OBJECTS += $(patsubst sources/avr/commands/%.c, $(OBJDIR)/commands/%.o, $(wildcard sources/avr/commands/*.c))

AVRDUDE := avrdude
# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT := /dev/ttyACM0    # programmer connected to serial device
AVRDUDE_CONFIG != find /usr/share/arduino/hardware -name avrdude.conf

AVRDUDE_FLAGS := -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b 115200
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += -D -q -V -C $(AVRDUDE_CONFIG)
AVRDUDE_FLAGS += -c wiring

# Use these as functions
avrdude_write_flash = -U flash:w:$(strip $(1)):i
avrdude_write_eeprom = -U eeprom:w:$(strip $(1)):i


# AVR-specific binaries recipes
target/avr/avrtmon.elf:	$(OBJECTS)
	@#echo Objects to compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test_serial.elf: tests/serial/avr_test_serial.c $(OBJDIR)/serial.o $(OBJDIR)/ringbuffer.o
	@echo Objects to compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test_timer.elf: tests/avr_test_timer.c $(OBJDIR)/serial.o $(OBJDIR)/ringbuffer.o
	@echo Objects to compile: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test_meta.elf: tests/avr_test_meta.c $(OBJDIR)/serial.o $(OBJDIR)/packet.o $(OBJDIR)/crc.o $(OBJDIR)/ringbuffer.o
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test_buttons.elf: tests/avr_test_buttons.c $(OBJDIR)/buttons.o $(OBJDIR)/serial.o $(OBJDIR)/ringbuffer.o
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test_nvm.elf: tests/avr_test_nvm.c $(OBJDIR)/nvm.o $(OBJDIR)/serial.o $(OBJDIR)/ringbuffer.o
	$(CC) $(CFLAGS) -o $@ $^

avr_test_buttons: target/avr/test_buttons.hex ;
avr_test_serial: target/avr/test_serial.hex ;
avr_test_timer: target/avr/test_timer.hex ;
avr_test_meta: target/avr/test_meta.hex ;
avr_test_nvm: target/avr/test_nvm.hex ;

%.eep:	%.elf
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	  --change-section-lma .eeprom=0 --no-change-warnings -O ihex $< $@ || exit 1

%.hex:	%.eep %.elf
	avr-objcopy -O ihex -R .eeprom $(patsubst %.hex, %.elf, $@) $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(call avrdude_write_flash, $@) \
	  $(call avrdude_write_eeprom, $(patsubst %.hex, %.eep, $@))

flash:
	make target/avr/avrtmon.hex

TARGET := target/avr/avrtmon.elf
