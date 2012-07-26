#ifndef BINARYWRITER_H
#define BINARYWRITER_H

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
		bool WriteFloat(float value);

		bool WriteBytes(unsigned char *c);
		bool WriteChars(char *c);
		bool WriteString(string s);

		bool IsLoaded;
		string fname;
		FILE *file;
		int totalBytes;
};

#endif // BINARYWRITER_H
