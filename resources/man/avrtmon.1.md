% AVRTMON(1) | AVR Temperature Monitor

NAME
====

**avrtmon** —  AVR Temperature Monitor client

SYNOPSIS
========

**avrtmon** \[**-s** _\[script]_] \[**-c** _device-file_]  
**avrtmon** -h

DESCRIPTION
===========

**avrtmon** is a temperature monitor device realized with an AVR microcontroller;
this program is an ad-hoc client to interface it.

Options
-------

-c _device-file_
:   Connect to the temperature monitor identified by _device-file_ (e.g. /dev/ttyACM0)

-s _\[script]_
:   Execute in script mode (do not print prompt, exit on error etc...), being
_script_ a file containing a command each line. If _script_ is not given, standard
input is used.

-h
:   Display help message and exit

Commands
--------

**help** \[_command_]
:   Show help, also for a specific command if an argument is given

**connect** _device\_path_
:   Connect to an avrtmon, given its device file (usually under /dev)

**disconnect**
:   Close an existing connection - Has no effect on the tmon

**download**
:   Download all the temperatures from the tmon. creating a new database

**tmon-reset**
:   Reset the internal temperatures DB of the tmon

**tmon-config** \<list|get _field_|set _field_ _value_\>
:   Manipulate the tmon configuration

**tmon-start**
:   Start registering temperatures

**tmon-stop**
:   Stop registering temperatures

**tmon-set-resolution** _value_
:   Set the timer resolution for the next DB until the tmon is reset

**tmon-set-interval** _value_
:   Set the timer interval for the next DB until the tmon is reset

**tmon-echo** _arg_ \[_arg2 arg3 ..._]
:   Send a string to the tmon, which should send it back

**list**
:   List the databases present in the storage

**export** _db_id_ _output_filepath_
:   Export a database in a gnuplot-friendly compatible format

AUTHOR
======

Paolo Lucchesi <paololucchesi@protonmail.com>

https://www.github.com/jcondor98/avrtmon
