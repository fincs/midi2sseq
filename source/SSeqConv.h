#pragma once
#include <vector>
#include <map>
#include "endian.h"
#include "MidiReader.h"

using std::multimap;
using std::pair;

typedef multimap<uchar, int> HeldNoteMap;
typedef HeldNoteMap::iterator HeldNoteIter;
typedef pair<uchar, int> HeldNote;

enum
{
	CNV_UNKNOWN = 0,

	CNV_NOTE, CNV_VOL, CNV_PAN, CNV_EXPR, CNV_PATCH, CNV_LOOPSTART, CNV_LOOPEND, CNV_TEMPO
};

typedef struct
{
	uint time;
	uint duration;
	ushort cmd;
	union
	{
		struct { uchar param1, param2; };
		ushort paramwide;
	};
} CnvEvent;

typedef struct
{
	int chnnum;
	int offset;
	vector<CnvEvent>* trackdata;
} CnvTrack;

class SSeqConv
{
	vector<CnvEvent> chn[16];
	HeldNoteMap relData[16];

	uchar chnusage[16];
	bool SaveTrack(FileClass&, CnvTrack&);
public:
	bool ConvertMidi(MidiReader&);
	bool SaveToFile(const char*);
};
