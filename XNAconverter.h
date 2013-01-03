#ifndef XNACONVERTER_H
#define XNACONVERTER_H

#include <malloc.h>
#include <string>
#include <iostream>
using namespace std;
#include <png.h>

#include "BinaryReader.h"
#include "BinaryWriter.h"

#define SURFACEFORMAT_NONE 0
#define SURFACEFORMAT_DXT1 4
#define SURFACEFORMAT_DXT3 5
#define SURFACEFORMAT_DXT5 6

class XNAconverter
{
	public:
		static bool XNB2WAV(string xnb, string wav);
		static bool XNB2PNG(string xnb, string png);
};

#endif // XNACONVERTER_H
