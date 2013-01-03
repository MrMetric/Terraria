#ifndef BINARYREADER_H
#define BINARYREADER_H

#include <stdio.h> // file operations
#include <string>
using namespace std;

class BinaryReader
{
	public:
		BinaryReader(string s);
		void ChangeFile(string s);
		void Close();

		int Read7BitEncodedInt(unsigned char b);
		unsigned char ReadByte();
		bool ReadBool();
		char ReadChar();
		int ReadShort();
		int ReadInt();
		long ReadLong();
		float ReadFloat();

		string ReadChars(int chars);
		string ReadString();

		bool isLoaded;
		string fname;
		int pos;
		FILE *file;
		long fSize;
};

#endif // BINARYREADER_H
