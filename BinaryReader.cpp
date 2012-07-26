#include <stdio.h> // file operations
#include <string>
using namespace std;

#include "BinaryReader.h"

/**
	BinaryReader.cpp
	This class is my implementation of Microsoft's C# BinaryReader
	It reads bytes from a file and returns the following:
	 - number values (short, int, etc)
	 - single characters or strings of characters
	It can read Microsoft-style strings (one byte before the string specifies the length)

	@arg s The file to read
*/

BinaryReader::BinaryReader(string s)
{
	this->fname = s;
	this->pos = 0;
	this->file = fopen(s.c_str(), "rb");
	fseek(file, 0, SEEK_END);
	this->fSize = ftell(file);
	rewind(file);
}

void BinaryReader::Close()
{
	fclose(this->file);
}

int BinaryReader::Read7BitEncodedInt(unsigned char b)
{
	int ret = 0;
	int shift = 0;

	do {
		ret = ret | ((b & 0x7f) << shift);
		shift += 7;
	} while ((b & 0x80) == 0x80);
	return ret;
}

unsigned char BinaryReader::ReadByte()
{
	fseek(this->file, 0, pos);
	char buf[1];
	pos += 1;
	fread(buf, 1, 1, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	int val = buf[0];
	return static_cast<unsigned char>(val);
}

bool BinaryReader::ReadBool()
{
	return (ReadByte() != 0);
}

char BinaryReader::ReadChar()
{
	fseek(this->file, 0, pos);
	pos += 1;
	char buf[1];
	fread(buf, 1, 1, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	return buf[0];
}

int BinaryReader::ReadShort()
{
	fseek(this->file, 0, pos);
	pos += 2;
	char buf[2];
	fread(buf, 1, 2, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	short ret = *(reinterpret_cast<short*>(buf));
	return ret;
}

int BinaryReader::ReadInt()
{
	fseek(this->file, 0, pos);
	pos += 4;
	char buf[4];
	fread(buf, 1, 4, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	// this cast is from Matt Davis at http://stackoverflow.com/questions/544928/reading-integer-size-bytes-from-a-char-array
	int ret = *(reinterpret_cast<int*>(buf));
	return ret;
}

float BinaryReader::ReadFloat()
{
	fseek(this->file, 0, pos);
	pos += 4;
	char buf[4];
	fread(buf, 1, 4, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	return *(reinterpret_cast<float*>(buf));
}

string BinaryReader::ReadChars(int chars)
{
	if(chars < 1) return "";
	fseek(this->file, 0, pos);
	pos += chars;
	char buf[chars];
	fread(buf, 1, chars, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	string s = buf;
	s = s.substr(0, chars);
	return s;
}

string BinaryReader::ReadString()
{
	fseek(this->file, 0, pos);
	pos += 1;
	char buf[1];
	fread(buf, 1, 1, this->file);
	int len = this->Read7BitEncodedInt(buf[0]);
	return this->ReadChars(len);
	/*fseek(this->file, 0, pos);
	pos += len;
	char sbuf[len]; // string buffer
	fread(sbuf, 1, len, this->file);
	if(ferror(this->file))
	{
		perror("Error reading file");
	}
	string s = sbuf;
	s = s.substr(0, len);
	return s;*/
}
