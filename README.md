# avrtmon
AVR Temperature Monitor (Project for Sistemi Operativi)

## Abstract

**avrtmon** is a portable temperature monitor, which registers the external
temperature at a constant rate, and save each registration in a non-volatile
memory storage device. 
From a hardware prespective, it makes use of:

* An AVR board (AT2560) as the central controller
* A temperature sensor (LM35)
* A couple of hardware buttons to control the registering flow (i.e. start/stop)

The end user shall use a sort of power supply (e.g. batteries) in order to make
the temperature monitor a meaningful, effectively usable object.

The AVR board must be able, once connected to the PC via serial port, to:

* Send to the PC operating system all the performed temperature registrations
(deleting them from its internal NVM)
* Receive commands (e.g. set a different registration rate, download data etc..)

### Terminology

From now to the end of this document, we use the following terminology:

* **tmon** is the AVR board (i.e. the temperature monitor itself)
* **NVM** is the _Non-Volatile Memory_ of the tmon, likely the internal EEPROM.
We refer to it in abstract terms as we could use another device (e.g. an SD card)

### Developer notes

* After a brief statistical analysis of communication error probability, the use
of CRC-8 might dropped in favour of CRC-16

* The structure of the packet header in the communication protocol is defined in
its overall structure, hence not yet definitive and susceptible to changes


## Serial/Packet protocol

The communication protocol between the tmon and the PC is interrupt-based (from
the tmon prespective) and packet-switched. Packet can vary in dimension, which
nevertheless can _never_ exceed the size of 51 bytes (2 bytes of header + 48
bytes of data + 1 byte CRC-8), and the size must be specified in the packet 
header; this is done to minimize the error rate and impact (lots of errors in
serial communication with AVR if you send a lot of data in one stroke).

### Packet header

The size of the packet header is always 2 bytes (16 bits), except for
acknowledgement and error packets, and its structure is the following:

Field | Size (bits) | Description
--:|:-:|---
type |4 | Type of the packet (see below)
id   |4 | Packet ID
size |8 | Size of the packet, in bytes. Notice that 8 bits are redundant

The type of the packet can be one of the following:

Type | C Macro | Code (binary) | Description
--:|:-:|:-:|---
Command | `CMD` | `0001` | Command sent from the PC to the tmon
Data | `DAT` | `0000` | Data (i.e. temperatures) sent from the tmon to the PC
Acknowledgement | `ACK` | `0010` | Acknowledgement message
Error | `ERR` | `0011` | Communication error (e.g. CRC mismatch)

<!-- TODO: Decide what to do with packet type MSBs later (no hurry) -->
The redundant bits (most significant ones) are reserved for future use, or might
be used to extend the packet id field.

The packet id is not unique in an entire session, as the crucial thing is to
avoid collisions of (i.e. distinguish) successive packets.

### Acknowledgement packets

A communication endpoint must wait for an acknowledgement message from the
counterpart once it sent a packet in order to send a new one. 
Acknowledgement packets are treated in a special way and do not bring any data.
Moreover, the packet id specified in the ID header field belongs to the
previously sent, to-be-acknowledged packet.


<!-- The contents below are a stub, do not consider them
## Configuration

The tmon configuration is stored at the beginning of its NVM, without any offset,
as a raw data structure.

Field | Size (bytes) | Description
--:|:-:|---
Registering Interval | 2 | Temperature registering interval in tenths of a second
ADC Channel | 1 | Identifies the analog pin used by the LM35
Unit of Measure | 1 | Unit of measure of the temperature (Celsius, Kelvin ...)

-->
