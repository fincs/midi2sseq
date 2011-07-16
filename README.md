midi2sseq
=========

Introduction
------------

This is a program that converts standard MIDI files into the SSEQ format.

Build prerequisites
-------------------

- Windows: MinGW
- Everything else: edit the makefile to remove the .exe extension first (TODO: autodetect)

Supported events
----------------

- Note on/off
- Volume, pan & expression
- Pitch bend & pitch bend range
- Patch change
- Tempo
- Loop points via `loopStart` and `loopEnd` markers

To do
-----

- Implement more MIDI commands
- Some SSEQ commands are not implemented yet (modulation, portamento, pitch sweep): please help!
