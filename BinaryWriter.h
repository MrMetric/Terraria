#ifndef BINARYWRITER_H
#define BINARYWRITER_H

#include <stdio.h> // file operations
#include <string.h> // strlen
#include <string>
#include <sstream> // stringstream
using namespace std;
#include "Util.h"

class BinaryWriter
{
	public:
		BinaryWriter(string s, bool bak);
		void addBytes(int i);
		void Close();

		void Write7BitEncodedInt(int value);
		bool WriteByte(unsigned char value);
		bool WriteBool(bool b);
		bool WriteChar(char c);
		bool WriteShort(short i);
		bool WriteInt(int i);
		bool WriteLong(long i);
		bool WriteFloat(float value);

		bool WriteBytes(unsigned char *c, int len);
		bool WriteBytes(unsigned char *c, int startpos, int len);
		bool WriteChars(char *c, int len);
		bool WriteString(string s);

		bool isLoaded;
		string fname;
		FILE *file;
		int totalBytes;
};

#endif // BINARYWRITER_H
