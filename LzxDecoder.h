#ifndef LZXDECODER_H
#define LZXDECODER_H

#include <string.h> // for memcpy
#include <iostream>
using namespace std;
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "BitBuffer.h"

#define MIN_MATCH					2
#define MAX_MATCH					257
#define NUM_CHARS					256
#define PRETREE_NUM_ELEMENTS		20
#define ALIGNED_NUM_ELEMENTS		8
#define NUM_PRIMARY_LENGTHS			7
#define NUM_SECONDARY_LENGTHS		249

#define PRETREE_MAXSYMBOLS			PRETREE_NUM_ELEMENTS
#define PRETREE_TABLEBITS			6
#define MAINTREE_MAXSYMBOLS			(NUM_CHARS+50*8)
#define MAINTREE_TABLEBITS			12
#define LENGTH_MAXSYMBOLS			(NUM_SECONDARY_LENGTHS+1)
#define LENGTH_TABLEBITS			12
#define ALIGNED_MAXSYMBOLS			ALIGNED_NUM_ELEMENTS
#define ALIGNED_TABLEBITS			7

#define LENTABLE_SAFETY				64

class LzxDecoder
{
	public:
		LzxDecoder(int window);
		int Decompress(BinaryReader *inData, int inLen, BinaryWriter *outData, int outLen);

		static unsigned int position_base[51];
		static unsigned char extra_bits[52];

	private:
		int MakeDecodeTable(unsigned int nsyms, unsigned int nbits, unsigned char length[], unsigned short table[]);
		void ReadLengths(unsigned char lens[], unsigned int first, unsigned int last, BitBuffer *bitbuf);
		unsigned int ReadHuffSym(unsigned short table[], unsigned char lengths[], unsigned int nsyms, unsigned int nbits, BitBuffer *bitbuf);

		enum BLOCKTYPE
		{
				INVALID = 0,
				VERBATIM = 1,
				ALIGNED = 2,
				UNCOMPRESSED = 3
		};
		unsigned int				state_R0, state_R1, state_R2; 	// for the LRU offset system
		unsigned short			state_main_elements;			// number of main tree elements
		int							state_header_read;				// have we started decoding at all yet?
		BLOCKTYPE					state_block_type;				// type of this block
		unsigned int				state_block_length;				// uncompressed length of this block
		unsigned int				state_block_remaining;			// uncompressed bytes still left to decode
		unsigned int				state_frames_read;				// the number of CFDATA blocks processed
		int							state_intel_filesize;			// magic header value used for transform
		int							state_intel_curpos;				// current offset in transform space
		int							state_intel_started;			// have we seen any translateable data yet?

		// yo dawg i herd u liek arrays so we put arrays in ur arrays so u can array while u array
		unsigned short			state_PRETREE_table[(1 << PRETREE_TABLEBITS) + (PRETREE_MAXSYMBOLS << 1)];
		unsigned char				state_PRETREE_len[PRETREE_MAXSYMBOLS + LENTABLE_SAFETY];
		unsigned short			state_MAINTREE_table[(1 << MAINTREE_TABLEBITS) + (MAINTREE_MAXSYMBOLS << 1)];
		unsigned char				state_MAINTREE_len[MAINTREE_MAXSYMBOLS + LENTABLE_SAFETY];
		unsigned short			state_LENGTH_table[(1 << LENGTH_TABLEBITS) + (LENGTH_MAXSYMBOLS << 1)];
		unsigned char				state_LENGTH_len[LENGTH_MAXSYMBOLS + LENTABLE_SAFETY];
		unsigned short			state_ALIGNED_table[(1 << ALIGNED_TABLEBITS) + (ALIGNED_MAXSYMBOLS << 1)];
		unsigned char				state_ALIGNED_len[ALIGNED_MAXSYMBOLS + LENTABLE_SAFETY];

		// NEEDED MEMBERS
		// CAB actualsize
		// CAB window
		// CAB window_size
		// CAB window_posn
		unsigned int				state_actual_size;
		unsigned char				state_window[65536];
		unsigned int				state_window_size;
		unsigned int				state_window_posn;
};

#endif
