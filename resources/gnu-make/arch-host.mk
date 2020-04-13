# avrtmon
# GNU Make common rules for host-side compilation
# Paolo Lucchesi

# Host Compiler setup
CC := gcc
CFLAGS := -std=gnu99 -Wall -lrt -lpthread -I$(INCDIR)/host -I$(INCDIR) \
  -funsigned-bitfields -fshort-enums -Wno-missing-braces
TESTFLAGS := -Itests/include -I$(INCDIR)/avr -DAVR -DTEST -Wno-format
NDEBUGFLAGS := -O2 -DNDEBUG
DEBUGFLAGS := -O0 -ggdb -DDEBUG

ifndef DEBUG
  CFLAGS += $(NDEBUGFLAGS)
else
  CFLAGS += $(DEBUGFLAGS)
endif


TARGET := target/host/avrtmon
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^


resources/bin/crc-table-generator: \
  resources/crc-table-generator.c $(OBJDIR)/crc.o
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


host-test-%: CFLAGS += $(TESTFLAGS)

host-test-serial: $(OBJDIR)/ringbuffer.o $(OBJDIR)/serial.o
	$(call host_test)

host-test-ringbuffer: $(OBJDIR)/ringbuffer.o
	$(call host_test)


.PHONY: install-host install-docs host-test-%
