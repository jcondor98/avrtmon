# avrtmon
# GNU Make common rules for host-side compilation
# Paolo Lucchesi

# Host Compiler setup
CC := gcc
CFLAGS := -Wall --std=c99 -O2 -funsigned-bitfields -fshort-enums \
  -Wstrict-prototypes -I$(INCDIR)
TESTFLAGS := -Itests/include -DTEST -ggdb -Wno-format


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
