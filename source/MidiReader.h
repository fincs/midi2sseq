#pragma once
#include <vector>
#include "FileClass.h"

using std::vector;

enum
{
	EV_UNKNOWN = 0, EV_WAIT,

	EV_NOTEON, EV_NOTEOFF, EV_AFTERTOUCH, EV_CONTROLLER, EV_PATCH, EV_AFTERTOUCH2, EV_PITCHBEND,
	EV_TEXT, EV_MARKER, EV_TEMPO
};

typedef struct
{
	uint cmd;
	union
	{
		uint delta;
		struct
		{
			uchar chn;
			union
			{
				struct
				{
					union { uchar note, number, patch, aft; };
					union { uchar vel, val; };
				};
				ushort valwide;
			};
		};
		const char* text;
		uint tempo;
	};
} MidiEvent;

class MidiReader
{
private:
	int tpb;

	bool LoadTrack(FileClass&, vector<MidiEvent>&);

public:
	vector< vector<MidiEvent> > tracks;

	MidiReader() : tpb(0)
	{
	}
	//~MidiReader();

	bool Load(const char*);
	int GetTicksPerBeat(){ return tpb; }
};
