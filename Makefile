# avrtmon
# Makefile - < Here goes a description of this specific makefiles >
# Uses the rules defined by Prof. Giorgio Grisetti in the file 'avr.mk'
# NOTE: With vim, you can easily substitute a <...> snippet by going in its line
#   and typing (do not count spaces):
#     ESC f< cf>
# Paolo Lucchesi - Tue 06 Aug 2019 02:27:30 AM CEST

# Executable files containing the 'main' routine
BINS = 

# Object files to be generated by GNU Make
OBJECTS = 

# Additional head files not included in the head directory
HEADERS = include/*


# TODO: Learn to use GNU Make better to make a better GNU Make makefile :D
test_crc:
	gcc -Iinclude/ -o tests/bin/crc_test sources/crc.c tests/crc_test.c
	tests/bin/crc_test | less -F
	rm tests/bin/crc_test

test_packet:
	gcc -Iinclude/ -DDEBUG -ggdb -o tests/bin/packet_test sources/{packet,crc}.c tests/packet_test.c
	tests/bin/packet_test | less -F
	rm tests/bin/packet_test

#test_packet:
#	gcc -Iinclude/ -DDEBUG -ggdb -o tests/bin/packet_test sources/{packet,crc}.c tests/packet_test.c
#	gdb tests/bin/packet_test
#	rm tests/bin/packet_test


# This does the magick
include resources/gnu-make/avr.mk
