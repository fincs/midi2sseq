#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SSeqConv.h"

typedef struct
{
	uint wait;
	uint pos;
} TrackStat;

static inline uint CnvTime(uint time, uint tpb)
{
	return (time * 48) / tpb;
}

bool SSeqConv::ConvertMidi(MidiReader& midi)
{
	int ntracks = midi.tracks.size();
	int tpb = midi.GetTicksPerBeat();
	TrackStat* tracksts = (TrackStat*) calloc(ntracks, sizeof(TrackStat));
	int finishtracks = 0;
	memset(chnusage, 0, sizeof(chnusage));

	for (uint time = 0; finishtracks < ntracks; time ++)
	{
		uint cnvtime = CnvTime(time, tpb);

		for (int i = 0; i < ntracks; i ++)
		{
			TrackStat* trackst = tracksts + i;

			if (trackst->wait)
			{
				trackst->wait --;
				if (trackst->wait) continue;
			}

			vector<MidiEvent>& track = midi.tracks[i];
			if (track.size() == trackst->pos) continue;

			while (!trackst->wait)
			{
				MidiEvent& midiev = track[trackst->pos];

				CnvEvent ev;
				ev.time = cnvtime;
				ev.duration = 0;
				ev.cmd = CNV_UNKNOWN;

				switch(midiev.cmd)
				{
					case EV_WAIT:
					{
						trackst->wait = midiev.delta;
						break;
					}

					case EV_NOTEON:
					{
						ev.cmd = CNV_NOTE;
						ev.param1 = midiev.note;
						ev.param2 = midiev.vel;
						chn[midiev.chn].push_back(ev);
						chnusage[midiev.chn] = 1;
						relData[midiev.chn].insert(HeldNote(midiev.note, chn[midiev.chn].size() - 1));
						break;
					}

					case EV_NOTEOFF:
					{
						HeldNoteIter it;
						pair<HeldNoteIter, HeldNoteIter> ret = relData[midiev.chn].equal_range(midiev.note);
						for (it = ret.first; it != ret.second; it ++)
						{
							CnvEvent& oldev = chn[midiev.chn][(*it).second];
							oldev.duration = ev.time - oldev.time;
						}
						relData[midiev.chn].erase(midiev.note);
						break;
					}

					case EV_CONTROLLER:
					{
						switch(midiev.number)
						{
							case 7:
								ev.cmd = CNV_VOL;
								ev.param1 = midiev.val;
								break;
							case 10:
								ev.cmd = CNV_PAN;
								ev.param1 = midiev.val;
								break;
							case 11:
								ev.cmd = CNV_EXPR;
								ev.param1 = midiev.val;
								break;
						}
						if (ev.cmd) chn[midiev.chn].push_back(ev); //, chnusage[midiev.chn] = 1;
						break;
					}

					case EV_PATCH:
					{
						ev.cmd = CNV_PATCH;
						ev.param1 = midiev.patch;
						chn[midiev.chn].push_back(ev);
						//chnusage[midiev.chn] = 1;
						break;
					}

					case EV_MARKER:
					{
						if (stricmp(midiev.text, "loopStart") == 0)
						{
							ev.cmd = CNV_LOOPSTART;
							for (int j = 0; j < 16; j ++) chn[j].push_back(ev);
						}else if (stricmp(midiev.text, "loopEnd") == 0)
						{
							ev.cmd = CNV_LOOPEND;
							for (int j = 0; j < 16; j ++) chn[j].push_back(ev);
						}
					}
			
					case EV_TEMPO:
					{
						ev.cmd = CNV_TEMPO;
						ev.paramwide = (ushort) midiev.tempo;
						chn[0].push_back(ev);
						chnusage[0] = 1;
						break;
					}
				}

				trackst->pos ++;
				if (track.size() == trackst->pos)
				{
					finishtracks ++;
					break;
				}
			}
		}
	}

	free(tracksts);
	return true;
}

bool SSeqConv::SaveToFile(const char* filename)
{
	ushort usagemask = 0;
	int ntracks = 0;
	for (int i = 0; i < 16; i ++)
		if (chnusage[i])
		{
			printf("Channel %d, %d event(s)\n", i+1, chn[i].size());
			usagemask |= 1 << i;
			ntracks ++;
		}

	if (!ntracks) return false;

	FileClass f(filename, "wb");
	if (f.openerror()) return false;

	f.WriteUInt(0x51455353); // SSEQ
	f.WriteUInt(0x0100feff);
	f.WriteUInt(0); // temp size
	f.WriteUShort(16); // struct size
	f.WriteUShort(1); // # of blocks

	f.WriteUInt(0x41544144); // DATA
	f.WriteUInt(0); // size of file - 16
	f.WriteUInt(0x1C); // offset of data

	f.WriteUChar(0xFE); // Multitrack
	f.WriteUShort(usagemask); // usage mask

	int tracktableoff = f.Tell();

	// Skip over track table
	f.Seek(5 * (ntracks - 1), SEEK_CUR);

	CnvTrack* tracks = new CnvTrack[ntracks];

	for (int i = 0, j = 0; i < 16; i ++)
	if (chnusage[i])
	{
		tracks[j].chnnum = i;
		tracks[j].offset = f.Tell() - 0x1C;
		tracks[j].trackdata = &chn[i];
		printf("Saving channel %d...\n", i+1);
		if (!SaveTrack(f, tracks[j]))
		{
			delete [] tracks;
			return false;
		}
		j ++;
	}

	// Come back to track table
	f.Seek(tracktableoff, SEEK_SET);

	for (int i = 1; i < ntracks; i ++)
	{
		f.WriteUChar(0x93);
		f.WriteUInt(tracks[i].chnnum | (tracks[i].offset << 8));
	}

	delete [] tracks;

	// Fix the size values
	f.Seek(0, SEEK_END);
	int fsize = f.Tell();
	f.Seek(8, SEEK_SET);
	f.WriteUInt(fsize);
	f.Seek(20, SEEK_SET);
	f.WriteUInt(fsize - 16);

	return true;
}

bool SSeqConv::SaveTrack(FileClass& f, CnvTrack& trinfo)
{
	// Notewait mode OFF
	f.WriteUChar(0xC7); f.WriteUChar(0);
	vector<CnvEvent>& data = *trinfo.trackdata;
	uint lasttime = 0;
	int loopOff = f.Tell() - 0x1C; // just in case

	for(uint i = 0; i < data.size(); i ++)
	{
		CnvEvent& ev = data[i];
		uint delta = ev.time - lasttime;
		//printf("%d TIME %u DELTA %u %d/", i, ev.time, delta, ev.cmd);
		if (ev.time < lasttime)
		{
			printf("ERROR: Out of order events! %u < %u\n",  ev.time, lasttime);
			return false;
		}

		if (delta)
		{
			f.WriteUChar(0x80); // REST
			f.WriteVL(delta);
		}

		lasttime = ev.time;

		switch(ev.cmd)
		{
			case CNV_NOTE:
				if (ev.duration)
				{
					f.WriteUChar(ev.param1 & 0x7F);
					f.WriteUChar(ev.param2 & 0x7F);
					f.WriteVL(ev.duration);
				}
				break;
			case CNV_VOL:
				f.WriteUChar(0xC1); // VOLUME
				f.WriteUChar(ev.param1 & 0x7F);
				break;
			case CNV_PAN:
				f.WriteUChar(0xC0); // PAN
				f.WriteUChar(ev.param1 & 0x7F);
				break;
			case CNV_EXPR:
				f.WriteUChar(0xD5); // EXPRESSION
				f.WriteUChar(ev.param1 & 0x7F);
				break;
			case CNV_PATCH:
				f.WriteUChar(0x81); // PATCH
				f.WriteUChar(ev.param1 & 0x7F);
				break;
			case CNV_LOOPSTART:
				loopOff = f.Tell() - 0x1C;
				break;
			case CNV_LOOPEND:
				f.WriteUChar(0x94); // JUMP
				f.WriteUInt(loopOff);
				break;
			case CNV_TEMPO:
				if (ev.paramwide > 240)
				{
					printf("WARNING: tempo too fast (%d)\n", ev.paramwide);
					break;
				}
				f.WriteUChar(0xE1); // TEMPO
				f.WriteUShort(ev.paramwide);
				break;
		}
	}

	f.WriteUChar(0xFF); // END OF TRACK

	return true;
}
