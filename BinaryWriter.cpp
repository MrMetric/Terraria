#include <stdio.h> // file operations
#include <string.h> // strlen
#include <string>
#include <sstream> // stringstream
using namespace std;

#include "BinaryWriter.h"

/**
	BinaryWriter.cpp
	This class is my implementation of Microsoft's C# BinaryWriter
	It writes the following to a file:
	 - the bytes that compose a number (short, int, ect)
	 - single characters or strings of characters
	 - Microsoft-style strings (one byte before the string specifies the length)

	@arg s		The file to write
	@arg bak	If this is true, the pre-existing file (if there is one) with the name
				specified in the previous argument will have .bak appended to its name
*/

BinaryWriter::BinaryWriter(string s, bool bak)
{
	this->IsLoaded = false;
	this->fname = s;
	this->totalBytes = 0;
	if(bak)
	{
		stringstream ss;
		ss << s << ".bak";
		remove(ss.str().c_str());
		rename(s.c_str(), ss.str().c_str());
	}
	this->file = fopen(s.c_str(), "ab");
	if(ferror(this->file))
	{
		perror("Error opening file");
	}
	else this->IsLoaded = true;
}

void BinaryWriter::addBytes(int i)
{
	this->totalBytes += i;
	// I used this for debugging
	//cout << "Wrote " << i << (i==1?" byte":" bytes") << endl;
}

void BinaryWriter::Close()
{
	fclose(this->file);
}

void BinaryWriter::Write7BitEncodedInt(int value)
{
	unsigned int temp = static_cast<unsigned int>(value);
	while(temp >= 128)
	{
		WriteByte(static_cast<unsigned char>(temp | 0x80));
		temp >>= 7;
	}
	WriteByte(static_cast<unsigned char>(temp));
}

bool BinaryWriter::WriteByte(unsigned char value)
{
	if(!this->IsLoaded) return false;
	addBytes(1);
	char buf[1];
	buf[0] = value;
	fwrite(buf, 1, 1, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteBool(bool b)
{
	if(!this->IsLoaded) return false;
	addBytes(1);
	char buf[1];
	buf[0] = (b?1:0);
	fwrite(buf, 1, 1, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteChar(char c)
{
	if(!this->IsLoaded) return false;
	this->totalBytes++;
	char buf[1];
	buf[0] = c;
	fwrite(buf, 1, 1, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteShort(short i)
{
	if(!this->IsLoaded) return false;
	addBytes(2);
	char buf[2];
	buf[0] = i;
	buf[1] = (i >> 8);
	fwrite(buf, 1, 2, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}


bool BinaryWriter::WriteInt(int i)
{
	if(!this->IsLoaded) return false;
	addBytes(4);
	char buf[4];
	buf[0] = i;
	buf[1] = (i >> 8);
	buf[2] = (i >> 16);
	buf[3] = (i >> 24);
	fwrite(buf, 1, 4, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteFloat(float value)
{
	if(!this->IsLoaded) return false;
	addBytes(4);
	unsigned long p = *(unsigned long *)&value;
	char buf[4];
	// from http://cboard.cprogramming.com/c-programming/137150-floats-bytes-back-again.html
	buf[0] = (p & 0x000000FF);
	buf[1] = ((p & 0x0000FF00) >> 8);
	buf[2] = ((p & 0x00FF0000) >> 16);
	buf[3] = ((p & 0xFF000000) >> 24);
	fwrite(buf, 1, 4, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteBytes(unsigned char *c)
{
	if(!this->IsLoaded) return false;
	addBytes(sizeof(c));
	fwrite(c, 1, sizeof(c), this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteChars(char *c)
{
	if(!this->IsLoaded) return false;
	int len = strlen(c);
	addBytes(len);
	fwrite(c, 1, len, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}

bool BinaryWriter::WriteString(string s)
{
	if(!this->IsLoaded) return false;
	int len = s.length();
	addBytes(len);
	Write7BitEncodedInt(len);
	fwrite(s.c_str(), 1, len, this->file);
	if(ferror(this->file))
	{
		perror("Error writing file");
		return false;
	}
	return true;
}
