#include <stdio.h>
#include <stdlib.h>
#include "MidiReader.h"
#include "endian.h"

#define MIDIMAGIC  0x4D546864
#define MIDIHEADERSIZE 6
#define TRACKMAGIC 0x4D54726B

bool MidiReader::Load(const char* filename)
{
	FileClass f(filename, "rb");
	if (f.openerror()) return false;
	f.SetBigEndian();

	if (f.ReadUInt() != MIDIMAGIC) return false;
	if (f.ReadUInt() != MIDIHEADERSIZE) return false;
	ushort format = f.ReadUShort();
	if (format != 0 && format != 1) return false; // No format 2 for you yet
	ushort trkcnt = f.ReadUShort();
	ushort timediv = f.ReadUShort();
	if (timediv & 0x8000) return false; // No SMPTE time division for you yet
	tpb = timediv;

	for (int i = 0; i < trkcnt; i ++)
	{
		tracks.push_back(vector<MidiEvent>());
		LoadTrack(f, tracks.back());
	}

	return true;
}

bool MidiReader::LoadTrack(FileClass& f, vector<MidiEvent>& track)
{
	if (f.ReadUInt() != TRACKMAGIC) return false;
	f.Seek(4, SEEK_CUR);

	int lastcmd = 0, cmd;
	for(;;)
	{
		uint delta = f.ReadVL();
		if (delta)
		{
			MidiEvent ev;
			ev.cmd = EV_WAIT;
			ev.delta = delta;
			track.push_back(ev);
		}
		MidiEvent ev;
		ev.cmd = EV_UNKNOWN;
		cmd = f.ReadUChar();
		if (cmd < 0x80) f.Seek(-1, SEEK_CUR), cmd = lastcmd;
		else lastcmd = cmd;

		if (cmd != 0xFF)
		{
			ev.chn = cmd & 0xF;
			switch(cmd & 0xF0)
			{
				case 0x90:
					ev.cmd = EV_NOTEON;
					ev.note = f.ReadUChar();
					ev.vel = f.ReadUChar();
					if (ev.vel == 0) ev.cmd = EV_NOTEOFF;
					break;

				case 0x80:
					ev.cmd = EV_NOTEOFF;
					ev.note = f.ReadUChar();
					ev.vel = f.ReadUChar();
					break;

				case 0xA0:
					ev.cmd = EV_AFTERTOUCH;
					ev.note = f.ReadUChar();
					ev.val = f.ReadUChar();
					break;

				case 0xB0:
					ev.cmd = EV_CONTROLLER;
					ev.number = f.ReadUChar();
					ev.val = f.ReadUChar();
					break;

				case 0xC0:
					ev.cmd = EV_PATCH;
					ev.patch = f.ReadUChar();
					break;

				case 0xD0:
					ev.cmd = EV_AFTERTOUCH2;
					ev.aft = f.ReadUChar();
					break;

				case 0xE0:
					ev.cmd = EV_PITCHBEND;
					ev.valwide = (ushort)f.ReadUChar() | ((ushort)f.ReadUChar() << 7);
					break;
			}
		}else
		{
			int metacmd = f.ReadUChar();
			int metasize = f.ReadVL();
			void* buf = NULL;
			if (metasize)
			{
				buf = malloc(metasize+1);
				f.ReadRaw(buf, metasize);
				((char*)buf)[metasize] = 0; // done for strings
			}

			if (metacmd == 0x2F) break;

			switch(metacmd)
			{
				case 1:
					ev.cmd = EV_TEXT;
					ev.text = (const char*) buf;
					buf = NULL;
					break;

				case 6:
					ev.cmd = EV_MARKER;
					ev.text = (const char*) buf;
					buf = NULL;
					break;

				case 81:
					ev.cmd = EV_TEMPO;
					ev.tempo = 60000000 / be_eswap_u24(*((uint*)buf));
					break;

				default:
					continue;
			}

			if (buf) free(buf);
		}

		track.push_back(ev);
	}

	return true;
}
