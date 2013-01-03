/*
This file uses code from MonoGame. The license is as follows:

Microsoft Public License (Ms-PL)
MonoGame - Copyright © 2009 The MonoGame Team

All rights reserved.

This license governs use of the accompanying software. If you use the software, you accept this license. If you do not
accept the license, do not use the software.

1. Definitions
The terms "reproduce," "reproduction," "derivative works," and "distribution" have the same meaning here as under
U.S. copyright law.

A "contribution" is the original software, or any additions or changes to the software.
A "contributor" is any person that distributes its contribution under this license.
"Licensed patents" are a contributor's patent claims that read directly on its contribution.

2. Grant of Rights
(A) Copyright Grant- Subject to the terms of this license, including the license conditions and limitations in section 3,
each contributor grants you a non-exclusive, worldwide, royalty-free copyright license to reproduce its contribution, prepare derivative works of its contribution, and distribute its contribution or any derivative works that you create.
(B) Patent Grant- Subject to the terms of this license, including the license conditions and limitations in section 3,
each contributor grants you a non-exclusive, worldwide, royalty-free license under its licensed patents to make, have made, use, sell, offer for sale, import, and/or otherwise dispose of its contribution in the software or derivative works of the contribution in the software.

3. Conditions and Limitations
(A) No Trademark License- This license does not grant you rights to use any contributors' name, logo, or trademarks.
(B) If you bring a patent claim against any contributor over patents that you claim are infringed by the software,
your patent license from such contributor to the software ends automatically.
(C) If you distribute any portion of the software, you must retain all copyright, patent, trademark, and attribution
notices that are present in the software.
(D) If you distribute any portion of the software in source code form, you may do so only under this license by including
a complete copy of this license with your distribution. If you distribute any portion of the software in compiled or object
code form, you may only do so under a license that complies with this license.
(E) The software is licensed "as-is." You bear the risk of using it. The contributors give no express warranties, guarantees
or conditions. You may have additional consumer rights under your local laws which this license cannot change. To the extent
permitted under your local laws, the contributors exclude the implied warranties of merchantability, fitness for a particular
purpose and non-infringement.
*/

#include <string.h>
#include "LzxDecoder.h"
#include "XNAconverter.h"

/**
	XNAconverter.cpp
	This class converts XNA-formatted files to a standard format
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
	unsigned char flags = br->ReadByte();
	bool compressed = (flags & 0x80) != 0;
	if(compressed)
	{
		cerr << "Reading compressed XNB sounds is not yet implemented\n";
		return false;
	}
	int fileLength = br->ReadInt();
	if(fileLength != br->fSize)
	{
		cerr << "File length mismatch: " << fileLength << " should be " << br->fSize << "\n";
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
	bw->WriteChars((char*)"RIFF", 4);
	bw->WriteInt(dataChunkSize + 36);
	bw->WriteChars((char*)"WAVEfmt ", 8);
	bw->WriteInt(16);
	bw->WriteShort(wFormatTag);
	bw->WriteShort(nChannels);
	bw->WriteInt(nSamplesPerSec);
	bw->WriteInt(nAvgBytesPerSec);
	bw->WriteShort(nBlockAlign);
	bw->WriteShort(wBitsPerSample);
	bw->WriteChars((char*)"data", 4);
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
	unsigned char flags = br->ReadByte();
	bool compressed = (flags & 0x80) != 0;
	int fileLength = br->ReadInt();
	if(fileLength != br->fSize)
	{
		cerr << "File length mismatch: " << fileLength << " should be " << br->fSize << "\n";
		return false;
	}
	string tmpfile = png + ".tmp";
	BinaryWriter *bw = new BinaryWriter(tmpfile, true);
	if(compressed)
	{
		int compressedSize = fileLength - 14;
		int decompressedSize = br->ReadInt();
		//int newFileSize = decompressedSize + 10;
		//cout << "Compressed size: " << compressedSize << "\nDecompressed size: " << decompressedSize << "\nNew file size: " << newFileSize << endl;
		// window = 16 bits
		// window size = 64k
		LzxDecoder *dec = new LzxDecoder(16);
		int decodedBytes = 0;
		int pos = 0;
		while(pos < compressedSize)
		{
			// let's seek to the correct position
			// The stream should already be in the correct position, and seeking can be slow
			br->pos = pos + 14;
			int hi = br->ReadByte();
			int lo = br->ReadByte();
			int block_size = (hi << 8) | lo;
			int frame_size = 0x8000;
			if(hi == 0xFF)
			{
				hi = lo;
				lo = br->ReadByte();
				frame_size = (hi << 8) | lo;
				hi = br->ReadByte();
				lo = br->ReadByte();
				block_size = (hi << 8) | lo;
				pos += 5;
			}
			else pos += 2;

			if(block_size == 0 || frame_size == 0) break;

			if(dec->Decompress(br, block_size, bw, frame_size) == -1)
			{
				cerr << "Error while decompressing data" << endl;
				//bw->Close();
				//delete bw;
				//remove(tmpfile.c_str());
				//return false;
			}
			pos += block_size;
			decodedBytes += frame_size;
		}
		if(bw->totalBytes != decompressedSize)
		{
			cerr << "Decompression of " << xnb << " failed: " << bw->totalBytes << " should be " << decompressedSize << "\n";
			bw->Close();
			delete bw;
			remove(tmpfile.c_str());
			return false;
		}
	}
	else
	{
		//cout << "XNB image is not compressed\n";
		for(int i = 0; i < fileLength; i++)
		{
			bw->WriteByte(br->ReadByte());
		}
	}
	bw->Close();
	delete bw;
	br->ChangeFile(tmpfile);
	int typeCount = br->Read7BitEncodedInt(br->ReadByte());
	if(typeCount != 1)
	{
		cerr << "Unhandled type count: " << typeCount << "\n";
		return false;
	}
	cout << "Type count: " << typeCount << endl;
	int typeReaderLen = br->ReadByte();
	br->pos = 3 + typeReaderLen + 6;
	int surfaceFormat = br->ReadInt();
	int width = br->ReadInt();
	int height = br->ReadInt();
	int levelCount = br->ReadInt();
	// from Microsoft's code
	/*for(int i = 0; i < num; i++)
	{
		int num2 = input.ReadInt32();
		byte[] data = input.ReadByteBuffer(num2);
		texture2D.SetData<byte>(i, null, data, 0, num2);
	}*/
	int imageLength = br->ReadInt();
	cout << "Surface format: " << surfaceFormat << "\nSize: " << width << "x" << height << "\nLevel count: " << levelCount << "\nImage length: " << imageLength << "\n";
	char imageData[imageLength];
	for(int i = 0; i < imageLength; i++)
	{
		imageData[i] = br->ReadByte();
	}
	switch(surfaceFormat)
	{
		case SURFACEFORMAT_NONE:
		{
			break;
		}
		case SURFACEFORMAT_DXT1:
		{
			cerr << "Reading DXT1-formatted XNB images is not yet implemented" << endl;
			return false;
		}
		case SURFACEFORMAT_DXT3:
		{
			cerr << "Reading DXT3-formatted XNB images is not yet implemented" << endl;
			return false;
		}
		case SURFACEFORMAT_DXT5:
		{
			cerr << "Reading DXT5-formatted XNB images is not yet implemented" << endl;
			return false;
		}
		default:
		{
			cerr << "Unknown XNB format: " << surfaceFormat << endl;
			return false;
		}
	}
	br->Close();
	delete br;
	//remove(tmpfile.c_str());
	// http://www.labbookpages.co.uk/software/imgProc/libPNG.html
	FILE *fp = fopen(png.c_str(), "wb");
	if(!fp)
	{
		cerr << "Error opening " << png <<" for writing\n";
		return false;
	}
	png_structp pngptr;
	png_infop pnginfo;
	pngptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!pngptr)
	{
		cerr << "Error creating PNG Write Struct\n";
		fclose(fp);
		return false;
	}
	pnginfo = png_create_info_struct(pngptr);
	if(!pnginfo)
	{
		cerr << "Error creating PNG Info Struct\n";
		fclose(fp);
		png_free_data(pngptr, pnginfo, PNG_FREE_ALL, -1);
		png_destroy_write_struct(&pngptr, (png_infopp)NULL);
		return false;
	}
	png_init_io(pngptr, fp);
	png_set_IHDR(pngptr, pnginfo, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	png_write_info(pngptr, pnginfo);
	int rowsize = 4*width*sizeof(png_byte);
	png_bytep row = (png_bytep)malloc(rowsize);
	int x, y, offset = 0;
	for(y = 0; y < height; y++)
	{
		for(x = 0; x < width; x++)
		{
			png_byte *ptr = &row[x*4];
			for(int o = 0; o < 4; o++)
			{
				ptr[o] = imageData[offset];
				string val = (o == 0?"R":(o == 1?"G":(o == 2?"B":(o == 3?"A":"?"))));
				offset++;
			}
		}
		png_write_row(pngptr, row);
	}
	png_write_end(pngptr, NULL);
	fclose(fp);
	png_free_data(pngptr, pnginfo, PNG_FREE_ALL, -1);
	png_destroy_write_struct(&pngptr, (png_infopp)NULL);
	free(row);
	return true;
}
