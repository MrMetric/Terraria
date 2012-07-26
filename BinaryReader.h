#ifndef BINARYREADER_H
#define BINARYREADER_H

class BinaryReader
{
	public:
		BinaryReader(string s);
		void Close();

		int Read7BitEncodedInt(unsigned char b);
		unsigned char ReadByte();
		bool ReadBool();
		char ReadChar();
		int ReadShort();
		int ReadInt();
		float ReadFloat();

		string ReadChars(int chars);
		string ReadString();

		string fname;
		int pos;
		FILE *file;
		long fSize;
};

#endif // BINARYREADER_H
