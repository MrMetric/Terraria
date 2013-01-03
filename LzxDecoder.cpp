// This file uses code from LzxDecoder.cs in MonoGame. The following is from the beginning of the file:
//#region HEADER
/* This file was derived from libmspack
 * (C) 2003-2004 Stuart Caie.
 * (C) 2011 Ali Scissons.
 *
 * The LZX method was created by Jonathan Forbes and Tomi Poutanen, adapted
 * by Microsoft Corporation.
 *
 * This source file is Dual licensed; meaning the end-user of this source file
 * may redistribute/modify it under the LGPL 2.1 or MS-PL licenses.
 */
//#region LGPL License
/* GNU LESSER GENERAL PUBLIC LICENSE version 2.1
 * LzxDecoder is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License (LGPL) version 2.1
 */
//#endregion
//#region MS-PL License
/*
 * MICROSOFT PUBLIC LICENSE
 * This source code is subject to the terms of the Microsoft Public License (Ms-PL).
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * is permitted provided that redistributions of the source code retain the above
 * copyright notices and this file header.
 *
 * Additional copyright notices should be appended to the list above.
 *
 * For details, see <http://www.opensource.org/licenses/ms-pl.html>.
 */
//#endregion
/*
 * This derived work is recognized by Stuart Caie and is authorized to adapt
 * any changes made to lzxd.c in his libmspack library and will still retain
 * this dual licensing scheme. Big thanks to Stuart Caie!
 *
 * DETAILS
 * This file is a pure C# port of the lzxd.c file from libmspack, with minor
 * changes towards the decompression of XNB files. The original decompression
 * software of LZX encoded data was written by Suart Caie in his
 * libmspack/cabextract projects, which can be located at
 * http://http://www.cabextract.org.uk/
 */
//#endregion

#include "LzxDecoder.h"

unsigned int LzxDecoder::position_base[51];
unsigned char LzxDecoder::extra_bits[52];

LzxDecoder::LzxDecoder(int window)
{
	unsigned int wndsize = (unsigned int)(1 << window);
	int posn_slots;

	// setup proper exception
	//if(window < 15 || window > 21)
	if(window != 16)
	{
		cerr << "Unsupported window size range: " << window << endl;
		return;
	}

	// let's initialise our state
	this->state_actual_size = 0;
	//this->state_window = new unsigned char[wndsize];
	for(unsigned int i = 0; i < wndsize; i++)
	{
		this->state_window[i] = 0xDC;
	}
	this->state_actual_size = wndsize;
	this->state_window_size = wndsize;
	this->state_window_posn = 0;

	// initialize static tables
	for(int i = 0, j = 0; i <= 50; i += 2)
	{
		LzxDecoder::extra_bits[i] = LzxDecoder::extra_bits[i+1] = (unsigned char)j;
		if ((i != 0) && (j < 17)) j++;
	}
	for(int i = 0, j = 0; i <= 50; i++)
	{
		position_base[i] = (unsigned int)j;
		j += 1 << this->extra_bits[i];
	}

	/* calculate required position slots */
	if(window == 20) posn_slots = 42;
	else if(window == 21) posn_slots = 50;
	else posn_slots = window << 1;

	this->state_R0 = this->state_R1 = this->state_R2 = 1;
	this->state_main_elements = (unsigned short)(NUM_CHARS + (posn_slots << 3));
	this->state_header_read = 0;
	this->state_frames_read = 0;
	this->state_block_remaining = 0;
	this->state_block_type = INVALID;
	this->state_intel_curpos = 0;
	this->state_intel_started = 0;

	/* initialise tables to 0 (because deltas will be applied to them) */
	for(int i = 0; i < MAINTREE_MAXSYMBOLS; i++) this->state_MAINTREE_len[i] = 0;
	for(int i = 0; i < LENGTH_MAXSYMBOLS; i++) this->state_LENGTH_len[i] = 0;
}

int LzxDecoder::Decompress(BinaryReader *inData, int inLen, BinaryWriter *outData, int outLen)
{
	BitBuffer *bitbuf = new BitBuffer(inData);
	long startpos = inData->pos;
	long endpos = inData->pos + inLen;

	unsigned char window[65536];
	memcpy(this->state_window, window, 65536);

	unsigned int window_posn = this->state_window_posn;
	unsigned int window_size = this->state_window_size;
	unsigned int R0 = this->state_R0;
	unsigned int R1 = this->state_R1;
	unsigned int R2 = this->state_R2;
	unsigned int i, j;

	int togo = outLen, this_run, main_element, match_length, match_offset, length_footer, extra, verbatim_bits;
	int rundest, runsrc, copy_length, aligned_bits;

	bitbuf->InitBitStream();

	// read header if necessary
	if(this->state_header_read == 0)
	{
		unsigned int intel = bitbuf->ReadBits(1);
		if(intel != 0)
		{
			// read the filesize
			i = bitbuf->ReadBits(16);
			j = bitbuf->ReadBits(16);
			this->state_intel_filesize = (int)((i << 16) | j);
		}
		this->state_header_read = 1;
	}
	if(this->state_intel_filesize == -1) this->state_intel_filesize = 0;
	cout << "state_intel_filesize: " << this->state_intel_filesize << endl;

	// main decoding loop
	while(togo > 0)
	{
		// last block finished, new block expected
		if(this->state_block_remaining == 0)
		{
			// TODO may screw something up here
			if(this->state_block_type == UNCOMPRESSED)
			{
				if((this->state_block_length & 1) == 1) inData->ReadByte(); // realign bitstream to word
				bitbuf->InitBitStream();
			}

			this->state_block_type = (BLOCKTYPE)bitbuf->ReadBits(3);
			cout << "state_block_type == " << this->state_block_type << endl;
			i = bitbuf->ReadBits(16);
			j = bitbuf->ReadBits(8);
			this->state_block_remaining = this->state_block_length = (unsigned int)((i << 8) | j);

			switch(this->state_block_type)
			{
				case ALIGNED:
				{
					for(i = 0, j = 0; i < 8; i++) { j = bitbuf->ReadBits(3); this->state_ALIGNED_len[i] = (unsigned char)j; }
					this->MakeDecodeTable(ALIGNED_MAXSYMBOLS, ALIGNED_TABLEBITS, this->state_ALIGNED_len, this->state_ALIGNED_table);
					// rest of aligned header is same as verbatim
				}

				case VERBATIM:
				{
					this->ReadLengths(this->state_MAINTREE_len, 0, 256, bitbuf);
					this->ReadLengths(this->state_MAINTREE_len, 256, this->state_main_elements, bitbuf);
					this->MakeDecodeTable(MAINTREE_MAXSYMBOLS, MAINTREE_TABLEBITS, this->state_MAINTREE_len, this->state_MAINTREE_table);
					if(this->state_MAINTREE_len[0xE8] != 0) this->state_intel_started = 1;

					this->ReadLengths(this->state_LENGTH_len, 0, NUM_SECONDARY_LENGTHS, bitbuf);
					this->MakeDecodeTable(LENGTH_MAXSYMBOLS, LENGTH_TABLEBITS, this->state_LENGTH_len, this->state_LENGTH_table);
					break;
				}

				case UNCOMPRESSED:
				{
					this->state_intel_started = 1; // because we can't assume otherwise
					bitbuf->EnsureBits(16); // get up to 16 pad bits into the buffer
					if(bitbuf->bitsleft > 16) inData->pos -= 2; // and align the bitstream!
					unsigned char hi, mh, ml, lo;
					lo = (unsigned char)inData->ReadByte(); ml = (unsigned char)inData->ReadByte(); mh = (unsigned char)inData->ReadByte(); hi = (unsigned char)inData->ReadByte();
					R0 = (unsigned int)(lo | ml << 8 | mh << 16 | hi << 24);
					lo = (unsigned char)inData->ReadByte(); ml = (unsigned char)inData->ReadByte(); mh = (unsigned char)inData->ReadByte(); hi = (unsigned char)inData->ReadByte();
					R1 = (unsigned int)(lo | ml << 8 | mh << 16 | hi << 24);
					lo = (unsigned char)inData->ReadByte(); ml = (unsigned char)inData->ReadByte(); mh = (unsigned char)inData->ReadByte(); hi = (unsigned char)inData->ReadByte();
					R2 = (unsigned int)(lo | ml << 8 | mh << 16 | hi << 24);
					break;
				}

				default:
				{
					cerr << "Invalid state block type" << endl;
					return -1; // TODO throw proper exception
				}
			}
		}

		/* buffer exhaustion check */
		if(inData->pos > (startpos + inLen))
		{
			/*it's possible to have a file where the next run is less than 16 bits in size. In this case, the READ_HUFFSYM() macro used in building
			the tables will exhaust the buffer, so we should allow for this, but not allow those accidentally read bits to be used
			(so we check that there are at least 16 bits remaining - in this boundary case they aren't really part of the compressed data)*/
			cerr << "WTF: pos > startpos + inLen" << endl;
			if(inData->pos > (startpos+inLen+2) || bitbuf->bitsleft < 16) return -1; //TODO throw proper exception
		}

		while((this_run = (int)this->state_block_remaining) > 0 && togo > 0)
		{
			if(this_run > togo) this_run = togo;
			togo -= this_run;
			this->state_block_remaining -= (unsigned int)this_run;

			// apply 2^x-1 mask
			window_posn &= window_size - 1;
			// runs can't straddle the window wraparound
			if((window_posn + this_run) > window_size)
			{
				cerr << "WTF: Window position + Run > Window Size" << endl;
				return -1; //TODO throw proper exception
			}

			switch(this->state_block_type)
			{
				case VERBATIM:
				{
					while(this_run > 0)
					{
						main_element = (int)this->ReadHuffSym(this->state_MAINTREE_table, this->state_MAINTREE_len, MAINTREE_MAXSYMBOLS, MAINTREE_TABLEBITS, bitbuf);
						if(main_element < NUM_CHARS)
						{
							/* literal: 0 to NUM_CHARS-1 */
							window[window_posn++] = (unsigned char)main_element;
							this_run--;
						}
						else
						{
							/* match: NUM_CHARS + ((slot<<3) | length_header (3 bits)) */
							main_element -= NUM_CHARS;

							match_length = main_element & NUM_PRIMARY_LENGTHS;
							if(match_length == NUM_PRIMARY_LENGTHS)
							{
								length_footer = (int)this->ReadHuffSym(this->state_LENGTH_table, this->state_LENGTH_len, LENGTH_MAXSYMBOLS, LENGTH_TABLEBITS, bitbuf);
								match_length += length_footer;
							}
							match_length += MIN_MATCH;

							match_offset = main_element >> 3;

							if(match_offset > 2)
							{
								// not repeated offset
								if(match_offset != 3)
								{
									extra = extra_bits[match_offset];
									verbatim_bits = (int)bitbuf->ReadBits((unsigned char)extra);
									match_offset = (int)position_base[match_offset] - 2 + verbatim_bits;
								}
								else
								{
									match_offset = 1;
								}

								/* update repeated offset LRU queue */
								R2 = R1; R1 = R0; R0 = (unsigned int)match_offset;
							}
							else if(match_offset == 0)
							{
								match_offset = (int)R0;
							}
							else if(match_offset == 1)
							{
								match_offset = (int)R1;
								R1 = R0; R0 = (unsigned int)match_offset;
							}
							else // match_offset == 2
							{
								match_offset = (int)R2;
								R2 = R0; R0 = (unsigned int)match_offset;
							}

							rundest = (int)window_posn;
							this_run -= match_length;

							/* copy any wrapped around source data */
							if(window_posn >= match_offset)
							{
								// no wrap
								runsrc = rundest - match_offset;
							}
							else
							{
								runsrc = rundest + ((int)window_size - match_offset);
								copy_length = match_offset - (int)window_posn;
								if(copy_length < match_length)
								{
									match_length -= copy_length;
									window_posn += (unsigned int)copy_length;
									while(copy_length-- > 0) window[rundest++] = window[runsrc++];
									runsrc = 0;
								}
							}
							window_posn += (unsigned int)match_length;

							/* copy match data - no worries about destination wraps */
							while(match_length-- > 0) window[rundest++] = window[runsrc++];
						}
					}
					break;
				}

				case ALIGNED:
				{
					while(this_run > 0)
					{
						main_element = (int)this->ReadHuffSym(this->state_MAINTREE_table, this->state_MAINTREE_len, MAINTREE_MAXSYMBOLS, MAINTREE_TABLEBITS, bitbuf);

						if(main_element < NUM_CHARS)
						{
							// literal 0 to NUM_CHARS-1
							window[window_posn++] = (unsigned char)main_element;
							this_run--;
						}
						else
						{
							/* match: NUM_CHARS + ((slot<<3) | length_header (3 bits)) */
							main_element -= NUM_CHARS;

							match_length = main_element & NUM_PRIMARY_LENGTHS;
							if(match_length == NUM_PRIMARY_LENGTHS)
							{
								length_footer = (int)this->ReadHuffSym(this->state_LENGTH_table, this->state_LENGTH_len, LENGTH_MAXSYMBOLS, LENGTH_TABLEBITS, bitbuf);
								match_length += length_footer;
							}
							match_length += MIN_MATCH;

							match_offset = main_element >> 3;

							if(match_offset > 2)
							{
								/* not repeated offset */
								extra = extra_bits[match_offset];
								match_offset = (int)position_base[match_offset] - 2;
								if(extra > 3)
								{
									/* verbatim and aligned bits */
									extra -= 3;
									verbatim_bits = (int)bitbuf->ReadBits((unsigned char)extra);
									match_offset += (verbatim_bits << 3);
									aligned_bits = (int)this->ReadHuffSym(this->state_ALIGNED_table, this->state_ALIGNED_len, ALIGNED_MAXSYMBOLS, ALIGNED_TABLEBITS, bitbuf);
									match_offset += aligned_bits;
								}
								else if(extra == 3)
								{
									/* aligned bits only */
									aligned_bits = (int)this->ReadHuffSym(this->state_ALIGNED_table, this->state_ALIGNED_len, ALIGNED_MAXSYMBOLS, ALIGNED_TABLEBITS, bitbuf);
									match_offset += aligned_bits;
								}
								else if (extra > 0) /* extra==1, extra==2 */
								{
									/* verbatim bits only */
									verbatim_bits = (int)bitbuf->ReadBits((unsigned char)extra);
									match_offset += verbatim_bits;
								}
								else /* extra == 0 */
								{
									/* ??? */
									match_offset = 1;
								}

								/* update repeated offset LRU queue */
								R2 = R1; R1 = R0; R0 = (unsigned int)match_offset;
							}
							else if( match_offset == 0)
							{
								match_offset = (int)R0;
							}
							else if(match_offset == 1)
							{
								match_offset = (int)R1;
								R1 = R0; R0 = (unsigned int)match_offset;
							}
							else /* match_offset == 2 */
							{
								match_offset = (int)R2;
								R2 = R0; R0 = (unsigned int)match_offset;
							}

							rundest = (int)window_posn;
							this_run -= match_length;

							// copy any wrapped around source data
							if(window_posn >= match_offset)
							{
								// no wrap
								runsrc = rundest - match_offset;
							}
							else
							{
								runsrc = rundest + ((int)window_size - match_offset);
								copy_length = match_offset - (int)window_posn;
								if(copy_length < match_length)
								{
									match_length -= copy_length;
									window_posn += (unsigned int)copy_length;
									while(copy_length-- > 0) window[rundest++] = window[runsrc++];
									runsrc = 0;
								}
							}
							window_posn += (unsigned int)match_length;

							// copy match data - no worries about destination wraps
							while(match_length-- > 0) window[rundest++] = window[runsrc++];
						}
					}
					break;
				}

				case UNCOMPRESSED:
				{
					if((inData->pos + this_run) > endpos)
					{
						cerr << "(inData->pos + this_run) > endpos" << endl;
						return -1; //TODO throw proper exception
					}
					/*unsigned char temp_buffer[this_run];
					for(int i = 0; i < this_run; i++)
					{
						temp_buffer[i] = inData->ReadByte();
					}
					temp_buffer.CopyTo(window, window_posn);*/
					inData->pos += this_run;
					window_posn += (unsigned int)this_run;
					break;
				}

				default:
				{
					cerr << "Invalid state block type" << endl;
					return -1; //TODO throw proper exception
				}
			}
		}
	}
	if(togo != 0)
	{
		cerr << "togo != 0" << endl;
		return -1; //TODO throw proper exception
	}
	int start_window_pos = (int)window_posn;
	if(start_window_pos == 0) start_window_pos = (int)window_size;
	start_window_pos -= outLen;
	outData->WriteBytes(window, start_window_pos, outLen);

	this->state_window_posn = window_posn;
	this->state_R0 = R0;
	this->state_R1 = R1;
	this->state_R2 = R2;

	// TODO finish intel E8 decoding
	/* intel E8 decoding */
	if((this->state_frames_read++ < 32768) && this->state_intel_filesize != 0)
	{
		/*if(outLen <= 6 || this->state_intel_started == 0)
		{
			this->state_intel_curpos += outLen;
		}
		else
		{
			int dataend = outLen - 10;
			unsigned int curpos = (unsigned int)this->state_intel_curpos;
			//unsigned int filesize = (unsigned int)this->state_intel_filesize;
			//unsigned int abs_off, rel_off;

			this->state_intel_curpos = (int)curpos + outLen;

			BinaryReader *br = new BinaryReader(outData->fname);
			br->pos = outData->totalBytes;

			while(outData->totalBytes < dataend)
			{
				outData->totalBytes++;
				if(br->ReadByte() != 0xE8)
				{
					curpos++;
					continue;
				}
				//abs_off =
			}
			delete br;
		}*/
		cerr << "Can't decode Intel E8" << endl;
		return -1;
	}
	return 0;
}

// READ_LENGTHS(table, first, last)
// if(lzx_read_lens(LENTABLE(table), first, last, bitsleft))
//   return ERROR (ILLEGAL_DATA)
//

// TODO make returns throw exceptions
int LzxDecoder::MakeDecodeTable(unsigned int nsyms, unsigned int nbits, unsigned char length[], unsigned short table[])
{
	unsigned short sym;
	unsigned int leaf;
	unsigned char bit_num = 1;
	unsigned int fill;
	unsigned int pos			= 0; // the current position in the decode table
	unsigned int table_mask	= (unsigned int)(1 << (int)nbits);
	unsigned int bit_mask		= table_mask >> 1; // don't do 0 length codes
	unsigned int next_symbol	= bit_mask; // base of allocation for long codes

	/* fill entries for codes short enough for a direct mapping */
	while(bit_num <= nbits )
	{
		for(sym = 0; sym < nsyms; sym++)
		{
			if(length[sym] == bit_num)
			{
				leaf = pos;

				if((pos += bit_mask) > table_mask) return 1; /* table overrun */

				/* fill all possible lookups of this symbol with the symbol itself */
				fill = bit_mask;
				while(fill-- > 0) table[leaf++] = sym;
			}
		}
		bit_mask >>= 1;
		bit_num++;
	}

	/* if there are any codes longer than nbits */
	if(pos != table_mask)
	{
		/* clear the remainder of the table */
		for(sym = (unsigned short)pos; sym < table_mask; sym++) table[sym] = 0;

		/* give ourselves room for codes to grow by up to 16 more bits */
		pos <<= 16;
		table_mask <<= 16;
		bit_mask = 1 << 15;

		while(bit_num <= 16)
		{
			for(sym = 0; sym < nsyms; sym++)
			{
				if(length[sym] == bit_num)
				{
					leaf = pos >> 16;
					for(fill = 0; fill < bit_num - nbits; fill++)
					{
						// if this path hasn't been taken yet, 'allocate' two entries
						if(table[leaf] == 0)
						{
							table[(next_symbol << 1)] = 0;
							table[(next_symbol << 1) + 1] = 0;
							table[leaf] = (unsigned short)(next_symbol++);
						}
						// follow the path and select either left or right for next bit
						leaf = (unsigned int)(table[leaf] << 1);
						if(((pos >> (int)(15-fill)) & 1) == 1) leaf++;
					}
					table[leaf] = sym;

					if((pos += bit_mask) > table_mask) return 1;
				}
			}
			bit_mask >>= 1;
			bit_num++;
		}
	}

	// full table?
	if(pos == table_mask) return 0;

	// either erroneous table, or all elements are 0 - let's find out.
	for(sym = 0; sym < nsyms; sym++) if(length[sym] != 0) return 1;
	return 0;
}

// TODO throw exceptions instead of returns
void LzxDecoder::ReadLengths(unsigned char lens[], unsigned int first, unsigned int last, BitBuffer *bitbuf)
{
	unsigned int x, y;
	int z;

	// hufftbl pointer here?

	for(x = 0; x < 20; x++)
	{
			y = bitbuf->ReadBits(4);
			this->state_PRETREE_len[x] = (unsigned char)y;
	}
	MakeDecodeTable(PRETREE_MAXSYMBOLS, PRETREE_TABLEBITS, this->state_PRETREE_len, this->state_PRETREE_table);

	for(x = first; x < last;)
	{
			z = (int)ReadHuffSym(this->state_PRETREE_table, this->state_PRETREE_len, PRETREE_MAXSYMBOLS, PRETREE_TABLEBITS, bitbuf);
			if(z == 17)
			{
					y = bitbuf->ReadBits(4); y += 4;
					while(y-- != 0) lens[x++] = 0;
			}
			else if(z == 18)
			{
					y = bitbuf->ReadBits(5); y += 20;
					while(y-- != 0) lens[x++] = 0;
			}
			else if(z == 19)
			{
					y = bitbuf->ReadBits(1); y += 4;
					z = (int)ReadHuffSym(this->state_PRETREE_table, this->state_PRETREE_len, PRETREE_MAXSYMBOLS, PRETREE_TABLEBITS, bitbuf);
					z = lens[x] - z; if(z < 0) z += 17;
					while(y-- != 0) lens[x++] = (unsigned char)z;
			}
			else
			{
					z = lens[x] - z; if(z < 0) z += 17;
					lens[x++] = (unsigned char)z;
			}
	}
}

unsigned int LzxDecoder::ReadHuffSym(unsigned short table[], unsigned char lengths[], unsigned int nsyms, unsigned int nbits, BitBuffer *bitbuf)
{
	unsigned int i, j;
	bitbuf->EnsureBits(16);
	if((i = table[bitbuf->PeekBits((unsigned char)nbits)]) >= nsyms)
	{
			j = (unsigned int)(1 << (int)((sizeof(unsigned int)*8) - nbits));
			do
			{
					j >>= 1; i <<= 1; i |= (bitbuf->buffer & j) != 0 ? (unsigned int)1 : 0;
					if(j == 0) return 0; // TODO throw proper exception
			} while((i = table[i]) >= nsyms);
	}
	j = lengths[i];
	bitbuf->RemoveBits((unsigned char)j);

	return i;
}
