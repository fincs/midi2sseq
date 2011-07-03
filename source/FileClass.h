#pragma once
#include <stdio.h>
#include "endian.h"

class FileClass
{
	FILE* f;
	bool LittleEndian;

public:
	FileClass(const char* file, const char* mode) : LittleEndian(true)
	{
		f = fopen(file, mode);
	}
	~FileClass()
	{
		if (f) fclose(f);
	}

	void SetLittleEndian() { LittleEndian = true; }
	void SetBigEndian() { LittleEndian = false; }

	FILE* get_ptr() { return f; }
	bool openerror() { return f == NULL; }

	uint ReadUInt()
	{
		uint value;
		fread(&value, sizeof(uint), 1, f);
		return LittleEndian ? le_eswap_uint(value) : be_eswap_uint(value);
	}

	void WriteUInt(uint value)
	{
		value = LittleEndian ? le_eswap_uint(value) : be_eswap_uint(value);
		fwrite(&value, sizeof(uint), 1, f);
	}

	ushort ReadUShort()
	{
		ushort value;
		fread(&value, sizeof(ushort), 1, f);
		return LittleEndian ? le_eswap_ushort(value) : be_eswap_ushort(value);
	}

	void WriteUShort(ushort value)
	{
		value = LittleEndian ? le_eswap_ushort(value) : be_eswap_ushort(value);
		fwrite(&value, sizeof(ushort), 1, f);
	}

	uchar ReadUChar()
	{
		uchar value;
		fread(&value, sizeof(uchar), 1, f);
		return value;
	}

	void WriteUChar(uchar value)
	{
		fwrite(&value, sizeof(uchar), 1, f);
	}

	uint ReadVL()
	{
		uint value = 0;
		uchar data = 0;
		do
			data = ReadUChar(), value = (value << 7) | ((uint)data & 0x7F);
		while(data & 0x80);
		return value;
	}

	void WriteVL(uint value)
	{
		int size = 1;
		for(uint temp = value; temp > 0x7F; temp >>= 7, size ++);
		int shift = 7*(size - 1);
		for(int i = 0; i < (size-1); i ++, shift -= 7)
			WriteUChar(0x80 | ((value >> shift) & 0x7F));
		WriteUChar((value >> shift) & 0x7F);
	}

	bool ReadRaw(void* buffer, size_t size) { return fread(buffer, 1, size, f) == size; }
	bool WriteRaw(void* buffer, size_t size) { return fwrite(buffer, 1, size, f) == size; }

	void Seek(int pos, int mode) { fseek(f, pos, mode); }
	int Tell() { return ftell(f); }
	void Flush() { fflush(f); }
};
