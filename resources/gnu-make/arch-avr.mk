# avrtmon
# GNU Make common rules for AVR compilation
# Rights to Prof. Giorgio Grisetti - This file was also heavily modified

# AVR Compiler and Programmer setup
CC := avr-gcc
AS := avr-gcc
CFLAGS := -Wall --std=gnu99 -DF_CPU=16000000UL -O3 -funsigned-char \
		-funsigned-bitfields  -fshort-enums -Wstrict-prototypes \
		-mmcu=atmega2560 -I$(INCDIR)/avr -I$(INCDIR) -DAVR -D__AVR_3_BYTE_PC__
ASFLAGS := -x assembler-with-cpp $(CFLAGS)

OBJECTS += $(patsubst sources/avr/commands/%.c, $(OBJDIR)/commands/%.o, \
		   $(wildcard sources/avr/commands/*.c))

AVRDUDE := avrdude
# com1 = serial port. Use lpt1 to connect to parallel port.
AVRDUDE_PORT := /dev/ttyACM0    # programmer connected to serial device
AVRDUDE_CONFIG != find /usr/share/arduino/hardware -name avrdude.conf

AVRDUDE_FLAGS := -p m2560 -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER) -b \
  115200 $(AVRDUDE_NO_VERIFY) $(AVRDUDE_VERBOSE) $(AVRDUDE_ERASE_COUNTER) \
  -D -q -V -C $(AVRDUDE_CONFIG) -c wiring

# Use these as functions
avrdude_write_flash = -U flash:w:$(strip $(1)):i
avrdude_write_eeprom = -U eeprom:w:$(strip $(1)):i

# LED preprocessor macros
CFLAGS += -DPOWER_ON_LED=D22 -DPOWER_ACT_LED=D24 \
  -DTEMPERATURE_REGISTERING_LED=D26


%.eep:	%.elf
	avr-objcopy -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	  --change-section-lma .eeprom=0 --no-change-warnings \
	  -O ihex $< $@ || exit 1

%.hex:	%.eep %.elf
	avr-objcopy -O ihex -R .eeprom $(patsubst %.hex, %.elf, $@) $@
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(call avrdude_write_flash, $@) \
	  $(call avrdude_write_eeprom, $(patsubst %.hex, %.eep, $@))


# AVR-specific binaries recipes
avr-test-%: target/avr/test-%.hex ;

TARGET := target/avr/avrtmon.elf
$(TARGET):	$(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test-serial.elf: tests/serial/avr-test-serial.c \
  $(addprefix $(OBJDIR), serial.o ringbuffer.o)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test-timer.elf: tests/avr-test-timer.c \
  $(addprefix $(OBJDIR), serial.o ringbuffer.o)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test-meta.elf: tests/avr-test-meta.c \
  $(addprefix $(OBJDIR), serial.o packet.o crc.o ringbuffer.o)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test-buttons.elf: tests/avr-test-buttons.c \
  $(addprefix $(OBJDIR), buttons.o serial.o ringbuffer.o)
	$(CC) $(CFLAGS) -o $@ $^

target/avr/test-nvm.elf: tests/avr-test-nvm.c \
  $(addprefix $(OBJDIR), nvm.o serial.o ringbuffer.o)
	$(CC) $(CFLAGS) -o $@ $^


.PHONY: avr-test-%
