#pragma once
#include <vector>
#include "endian.h"
#include "MidiReader.h"

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
	uint relData[16][128];
	uchar chnusage[16];
	bool SaveTrack(FileClass&, CnvTrack&);
public:
	bool ConvertMidi(MidiReader&);
	bool SaveToFile(const char*);
};
