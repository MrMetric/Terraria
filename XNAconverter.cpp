#include <string>
#include <iostream>
using namespace std;

#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "XNAconverter.h"

/**
	XNAconverter.cpp
	This class converts XNA-formatted files to a standard format for use with SDL
	Functions:
		XNB2WAV - Converts an XNB sound to WAV
		XNB2PNG - Converts an XNB image to PNG (incomplete)
*/

/**
	XNB2WAV
	Derived from http://www.terrariaonline.com/threads/86509/

	@arg xnb The XNB file to read
	@arg wav The WAV file to create
*/
bool XNAconverter::XNB2WAV(string xnb, string wav)
{
	BinaryReader *br = new BinaryReader(xnb);
	string format = br->ReadChars(3);
	if(format != "XNB")
	{
		cerr << "Invalid format: " << format << "\n";
		return false;
	}
	unsigned char tmpchar = br->ReadByte();
	if(tmpchar != 'w')
	{
		cerr << "Unhandled platform: " << tmpchar << "\n";
		return false;
	}
	tmpchar = br->ReadByte();
	if(tmpchar != 5)
	{
		cerr << "Unhandled version: " << (int)tmpchar << "\n";
		return false;
	}
	tmpchar = br->ReadByte();
	if(tmpchar != 0)
	{
		cerr << "Unhandled profile: " << (int)tmpchar << "\n";
		return false;
	}
	int fileLength = br->ReadInt();
	if(fileLength != br->fSize)
	{
		cerr << "File length mismatch: " << fileLength << " - should be " << br->fSize << "\n";
		return false;
	}
	int typeCount = br->Read7BitEncodedInt(br->ReadByte());
	if(typeCount != 1)
	{
		cerr << "Unhandled type count: " << typeCount << "\n";
		return false;
	}
	string type = br->ReadString();
	if(type != "Microsoft.Xna.Framework.Content.SoundEffectReader")
	{
		cerr << "Unhandled type reader: " << type << "\n";
		return false;
	}
	int typeReaderVersion = br->ReadInt();
	if(typeReaderVersion != 0)
	{
		cerr << "Unhandled type reader version: " << typeReaderVersion << "\n";
		return false;
	}
	unsigned int sharedResourcesCount = br->Read7BitEncodedInt(br->ReadByte());
	if(sharedResourcesCount != 0)
	{
		cerr << "Too many shared resources: " << sharedResourcesCount << "\n";
		return false;
	}
	int unknown = br->Read7BitEncodedInt(br->ReadByte());
	if(unknown != 1)
	{
		cerr << "Unknown byte is not 1 (" << unknown << ")\n";
		return false;
	}
	// WAVE format
	int formatChunkSize = br->ReadInt();
	if(formatChunkSize != 18)
	{
		cerr << "Wrong format chunk size: " << formatChunkSize << "\n";
		return false;
	}
	short wFormatTag;
	if((wFormatTag = br->ReadShort()) != 1)
	{
		cerr << "Unhandled wav codec (must be PCM)\n";
		return false;
	}
	short nChannels = br->ReadShort();
	int nSamplesPerSec = br->ReadInt();
	int nAvgBytesPerSec = br->ReadInt();
	short nBlockAlign = br->ReadShort();
	short wBitsPerSample = br->ReadShort();
	if(nAvgBytesPerSec != (nSamplesPerSec * nChannels * (wBitsPerSample / 8)))
	{
		cerr << "Average bytes per second number incorrect\n";
		return false;
	}
	if(nBlockAlign != (nChannels * (wBitsPerSample / 8)))
	{
		cerr << "Block align number incorrect\n";
		return false;
	}
	br->ReadShort();
	int dataChunkSize = br->ReadInt();

	BinaryWriter *bw = new BinaryWriter(wav, false);
	bw->WriteChars((char*)"RIFF");
	bw->WriteInt(dataChunkSize + 36);
	bw->WriteChars((char*)"WAVEfmt ");
	bw->WriteInt(16);
	bw->WriteShort(wFormatTag);
	bw->WriteShort(nChannels);
	bw->WriteInt(nSamplesPerSec);
	bw->WriteInt(nAvgBytesPerSec);
	bw->WriteShort(nBlockAlign);
	bw->WriteShort(wBitsPerSample);
	bw->WriteChars((char*)"data");
	bw->WriteInt(dataChunkSize);
	//cout << "Header size: " << bw->totalBytes << endl;
	for(int i = 0; i < dataChunkSize; i++)
	{
		bw->WriteChar(br->ReadChar());
	}
	//cout << "Data size: " << dataChunkSize << endl;
	//cout << "Total: " << bw->totalBytes << endl;
	br->Close();
	bw->Close();
	delete(br);
	delete(bw);
	return true;
}

/**
	XNB2PNG

	@note INCOMPLETE; This will not create a PNG file!
	@arg xnb The XNB file to read
	@arg wav The PNG file to create
*/
bool XNAconverter::XNB2PNG(string xnb, string png)
{
	BinaryReader *br = new BinaryReader(xnb);
	string format = br->ReadChars(3);
	if(format != "XNB")
	{
		cerr << "Invalid format: " << format << endl;
		return false;
	}
	unsigned char tmpchar = br->ReadByte();
	if(tmpchar != 'w')
	{
		cerr << "Unhandled platform: " << tmpchar << endl;
		return false;
	}
	tmpchar = br->ReadByte();
	if(tmpchar != 5)
	{
		cerr << "Unhandled version: " << tmpchar << endl;
		return false;
	}
	tmpchar = br->ReadByte();
	if(tmpchar != 128)
	{
		cout << "Unhandled profile: " << (int)tmpchar << "\n";
	}
	int tempint = br->ReadInt();
	if(tempint != br->fSize)
	{
		cerr << "File length mismatch: " << tempint << " - should be " << br->fSize << "\n";
		return false;
	}
	delete(br);
	cerr << "This operation is not yet implemented" << endl;
	return false;
}
