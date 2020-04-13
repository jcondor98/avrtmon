# avrtmon
# Makefile
# Uses the rules defined by Prof. Giorgio Grisetti in the file 'avr.mk'
# Refer to files located under 'resources/gnu-make/' for architecture-specific
# directives
.POSIX:
.SUFFIXES: # Reset all implicit rules

# Preserve object files to be automatically deleted as intermediate targets
.PRECIOUS: $(OBJDIR)/%.o

# Detect passed target architecture, passed in the ARCH variable (can be passed
# also from the shell).
# Possible values are 'avr' or 'host'. Defaults to 'avr'
ARCH ?= avr

# Codebase directory structure
SRCDIR := sources
INCDIR := include
OBJDIR := objects/$(ARCH)
RESDIR := resources

# Search Paths
vpath %.c $(SRCDIR)/$(ARCH)
vpath %.c $(SRCDIR)
vpath %.s $(SRCDIR)/$(ARCH)
vpath %.s $(SRCDIR)
vpath %.o $(OBJDIR)

# Object files to be generated by GNU Make (e.g. foobar.o)
# Do not prefix the object directory (e.g. foo.o is OK, objects/foo.o is not)
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/*.c)) \
  $(patsubst $(SRCDIR)/$(ARCH)/%.c, $(OBJDIR)/%.o, $(wildcard $(SRCDIR)/$(ARCH)/*.c))


# The file containing specific rules for an architecture must be located in
# $(RESDIR)/gnu-make/arch-<ARCH>.mk
include $(RESDIR)/gnu-make/arch-$(ARCH).mk


# Objects and binaries recipes

$(OBJDIR)/%.o:	%.c 
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	%.s 
	$(AS) $(ASFLAGS) -c -o $@ $<


target: $(TARGET) ;

all:
	@make host
	@make avr

avr:
	@ARCH=avr make target

host:
	@ARCH=host make target

flash:
	@ARCH=avr make target/avr/avrtmon.hex

install:
	install -m 0755 target/host/avrtmon /usr/bin/avrtmon
	strip /usr/bin/avrtmon

install-docs:
	install -m 0644 resources/man/avrtmon.1.gz /usr/share/man/man1/

docs: resources/man/avrtmon.1.gz ;

resources/man/%.gz: resources/man/%.md
	pandoc --standalone --to man $< | gzip --stdout - > $@


# Generate the configuration sources
config-gen:
	resources/bin/config-gen -c $(RESDIR)/config/default.csv -N $(SRCDIR)/avr/nvm.c


# Test (from the PC's OS) a module of the project
# Tests are done by performing:
#   $(call host_test [additional compiler args])
# The function expects to find a source file named 'tests/<test_name>.c'
# You can also choose a test method defining the variable 'TEST_WITH' in your
# shell. Supported option are none (i.e. execute as-is), 'gdb' and 'less'
test-%: CFLAGS := $(TESTFLAGS) $(CFLAGS)

# Generate a mock configuration for testing
define test_config_gen =
	$(RESDIR)/bin/config-gen -c $(RESDIR)/config/test.csv -S tests/config.c \
	  -H tests/include/config.h -N tests/nvm.c
endef

# Remove the mock configuration for testing
define test_config_clean =
	rm tests/{config.c,include/config.h,nvm.c}
endef

test-crc: $(addprefix $(OBJDIR)/, crc.o)
	$(call host_test)

test-packet: $(addprefix $(OBJDIR)/, crc.o packet.o)
	$(call host_test)

test-temperature:
	make -s config-gen
	$(call host_test, $(SRCDIR)/avr/temperature_specific.c \
	  $(SRCDIR)/temperature.c $(SRCDIR)/avr/nvm.c tests/mock_nvm.c)

test-config:
	$(call test_config_gen)
	$(call host_test, tests/config.c tests/nvm.c tests/mock_nvm.c)
	$(call test_config_clean)

test-shell:
	$(call host_test, $(SRCDIR)/host/shell.c)

test-list:
	$(call host_test, $(SRCDIR)/host/list.c)

test-ringbuffer:
	$(call host_test, $(SRCDIR)/avr/ringbuffer.c)

test-meta:
	$(call host_test)

# Perform all tests in a stroke
test:
	@make -s test-crc
	@make -s test-packet
	@make -s test-config
	@make -s test-temperature
	@make -s test-ringbuffer
	@make -s test-list


clean:	
	rm -f $(OBJDIR)/../**/*.o $(BINS) tests/bin/* tests/{config.c,include/config.h} \
	  target/{host,avr}/*


.PHONY:	clean all config-gen test avr host flash install docs install-docs test-%
