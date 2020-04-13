# avrtmon
AVR Temperature Monitor (Project for Sistemi Operativi)

## Abstract

**avrtmon** is a portable temperature monitor, which registers the external
temperature at a constant rate, and save each registration in a non-volatile
memory storage device. 
From a hardware prespective, it makes use of:

* An AVR board (AT2560) as the central controller
* A temperature sensor (LM35)
* 3 hardware buttons to control the registering flow (i.e. start/stop) and to
power on/off the board
* 3 LEDs

The end user shall use a sort of power supply (e.g. batteries) in order to make
the temperature monitor a meaningful, effectively usable object.

The AVR board is able, once connected to the PC via serial port, to:

* Send to the PC operating system all the performed temperature registrations
(deleting them from its internal NVM)
* Receive commands (e.g. set a different registration rate, download data etc..)

### Installation
The _GNU Make_ build system is used, and the codebase is in common for both
host-side and AVR-side. For the AVR-side:
```
# Compile and link the full ELF executable and pack it into a HEX firmware
make

# 'make' and then flash the HEX firmware into the AVR microcontroller
make flash
```

For the host-side, instead:
```
# Compile and link the full executable
make host

# Install the -ALREADY COMPILED- executable
make install

# Generate (requires pandoc) and install man page
make install-docs
```

### Testing
The codebase is shipped with a test suite (based on an own-developed test framework).
Standard tests are per

### Terminology

From now to the end of this document, we use the following terminology:

* **tmon** is the AVR board (i.e. the temperature monitor itself)
* **NVM** is the _Non-Volatile Memory_ of the tmon, likely the internal EEPROM.
We refer to it in abstract terms as we could use another device (e.g. an SD card)
* **host-side** refers to all the stuff which happens PC
* **AVR-side** refers to all the stuff which happens in the AVR board


## Serial/Packet protocol

The communication protocol between the tmon and the PC is interrupt-based (from
the tmon prespective) and packet-switched. Packet can vary in dimension, which
nevertheless can _never_ exceed the size of 31 bytes (2 bytes of header + 28
bytes of data + 1 byte CRC-8), and the size must be specified in the packet 
header; this is done to minimize the rate of some common serial-related errors
(e.g. data overrun)

### Packet header

The size of the packet header is always 2 bytes (16 bits), and its structure is
the following:

Field | Size (bits) | Description
--:|:-:|---
type |4 | Type of the packet (see below)
id   |4 | Packet ID
size |7 | Size of the packet, in bytes. Notice that 7 bits are redundant
header\_par |1 | (Even) Parity bit of the header

The type of the packet can be one of the following:

Type | Numeric Code | Description
--:|:-:|:-:|---
HND | 0x00 | Handshake
ACK | 0x01 | Acknowledgement
ERR | 0x02 | Communication error (e.g. CRC mismatch)
CMD | 0x03 | Command (sent from the PC to the tmon)
CTR | 0x04 | Control sequence (e.g. for commands)
DAT | 0x05 | Data (e.g. temperatures) sent from the tmon to the PC

The redundant bits (most significant ones) of the type field are reserved for
future use.

The packet ID is not unique in an entire session, as the crucial thing is to
avoid collisions of (i.e. distinguish) successive packets. 
In fact, I suspect that everything would work if packet IDs were removed at all,
but I think they make the code much more resilient to eventual modifications to
the communication protocol (e.g. implementing a bulk receiving of packets).

### Acknowledgement and Error packets

A communication endpoint must wait for an acknowledgement message from the
counterpart once it sent a packet in order to send a new one. 
Acknowledgement and error packets do not bring any data.

When a packet arrives, it is checked for integrity and sanity. If it is sane,
then an ACK packet is sent; if not, then an ERR packet is sent, and the
previously sent packet must be resent; ACK and ERR packets are simply discarded
if corrupted in some way.


## Configuration

The tmon configuration is stored at the beginning of its NVM, without any offset,
as a raw data structure. The default configuration is shipped with the program
image, and flashed to the NVM with it.

The configuration can be permanently changed using remote commands.


## Commands

The tmon supports the execution of remote, arbitrary commands sent from the PC
via CMD packets. Those commands are the following:

Command | Description
:-:|:--
CONFIG\_GET\_FIELD | Get a configuration field (value is transported in a DAT packet)
CONFIG\_SET\_FIELD | Set a configuration field to an arbitrary value
TEMPERATURES\_DOWNLOAD | Download all the temperatures (after that, the tmon DB will be reset)
TEMPERATURES\_RESET | Reset the temperatures DB
ECHO | Echo a message back to the PC (developed mostly for debug purposes)
