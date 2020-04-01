# avrtmon
# GNU Make common rules for host-side compilation
# Paolo Lucchesi

# Host Compiler setup
# TODO: Remove -ggdb, restore -std=c99
CC := gcc
CFLAGS := -Wall -std=gnu99 -O2 -funsigned-bitfields -fshort-enums \
  -Wstrict-prototypes -I$(INCDIR)/host -I$(INCDIR) -DDEBUG -lrt -pthread -ggdb
TESTFLAGS := -Itests/include -I$(INCDIR)/avr -DAVR -DTEST -ggdb -Wno-format

TARGET := target/host/avrtmon
$(TARGET): $(OBJECTS)
	make -s config_gen
	$(CC) $(CFLAGS) -o $@ $^

resources/bin/crc_table_generator: resources/crc_table_generator.c $(OBJDIR)/crc.o
	$(CC) -O2 -I$(INCDIR) -o $@ $^


# Host-side testing canned recipe
define host_test =
	$(CC) $(CFLAGS) -o tests/bin/$@ tests/$@.c $^ $1 tests/test_framework.c
	@if  [ -z "$(TEST_WITH)"        ]; then \
		tests/bin/$@;	\
	elif [ '$(TEST_WITH)' == 'less' ]; then \
		tests/bin/$@ | less -F; \
	elif [ '$(TEST_WITH)' == 'gdb'  ]; then \
		gdb tests/bin/$@; \
	fi
	rm tests/bin/$@
endef

host_test_serial: $(OBJDIR)/ringbuffer.o $(OBJDIR)/serial.o
	$(CC) $(CFLAGS) -o tests/bin/serial_host $^ tests/serial/host_test_serial.c
	./tests/bin/serial_host
