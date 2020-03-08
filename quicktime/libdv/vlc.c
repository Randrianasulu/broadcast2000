/* 
 *  vlc.c
 *
 *     Copyright (C) Charles 'Buck' Krasic - April 2000
 *     Copyright (C) Erik Walthinsen - April 2000
 *
 *  This file is part of libdv, a free DV (IEC 61834/SMPTE 314M)
 *  decoder.
 *
 *  libdv is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your
 *  option) any later version.
 *   
 *  libdv is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *   
 *  You should have received a copy of the GNU General Public License
 *  along with GNU Make; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 *  The libdv homepage is http://libdv.sourceforge.net/.  
 */

#include <stdio.h>
#include "vlc.h"

// We will used five vlc tables, divided into what I term vlc
// "classes".  A vlc class is identified by the codeword prefix, and
// its corresponding lookup table indexed by codeword suffix.  The
// classes are organized by ranges of vlc codeword lengths as follows:
//
//   class 0: incomplete vlcs (less than 3 bits available to decode);
//   class 1: 2 <= length <= 5;
//   class 2: 6 <= length <= 9;
//   class 3: 10 <= length <= 12;
//   class 4: length == 13
//   class 5: length == 15
//
// The sizes of the respective lookup tables are: 1, 32, 128, 64, 64, 256 (all 32 bit elements).

// 2 or less bits available (no class selector)
static gint8 dv_vlc_class_broken[1] = { 0 };

// 3 - 6 bits available (no class selector)
static gint8 dv_vlc_class_lookup1[1] = { 1 };

// 7 - 10 bits available class selector is [0:1] 
static gint8 dv_vlc_class_lookup2[4] = 
{
  1, 1, 1, 2,
}; // dv_vlc_class_lookup2

// 11 - 16 bits available, class selector is [0:6]
static gint8 dv_vlc_class_lookup3[128] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 0, 0
}; // dv_vlc_class_lookup3

static gint8 dv_vlc_class_lookup4[128] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 0
}; // dv_vlc_class_lookup4

static gint8 dv_vlc_class_lookup5[128] = 
{
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 4, 5
}; // dv_vlc_class_lookup5

// Indexed by number of bits available
const gint8 *dv_vlc_classes[17] = 
{
	&dv_vlc_class_broken[0],
	&dv_vlc_class_broken[0],
	&dv_vlc_class_broken[0],
	&dv_vlc_class_lookup1[0],
	&dv_vlc_class_lookup1[0],
	&dv_vlc_class_lookup1[0],
	&dv_vlc_class_lookup1[0],
	&dv_vlc_class_lookup2[0],
	&dv_vlc_class_lookup2[0],
	&dv_vlc_class_lookup2[0],
	&dv_vlc_class_lookup2[0],
	&dv_vlc_class_lookup3[0],
	&dv_vlc_class_lookup3[0],
	&dv_vlc_class_lookup4[0],
	&dv_vlc_class_lookup4[0],
	&dv_vlc_class_lookup4[0],
	&dv_vlc_class_lookup5[0]
}; // dv_vlc_classes

// bitmask to extract class index, given x bits of left-aligned input
// the class index is derived from y prefix bits 
const gint dv_vlc_class_index_mask[17] = 
{
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xc000, 0xc000, 0xc000, 0xc000,
	0xfe00, 0xfe00, 0xfe00, 0xfe00, 0xfe00, 0xfe00,
}; // class_index_mask 

const gint dv_vlc_class_index_rshift[17] = 
{
	0, 0, 0, 0, 0, 0, 0, 
	14, 14, 14, 14, 
	9, 9, 9, 9, 9, 9
}; // class_index_mask 

const dv_vlc_tab_t dv_vlc_broken[1] = 
{ 
	{ -1, VLC_NOBITS, -1 }
};

const dv_vlc_tab_t dv_vlc_lookup1[32] = 
{ 
  // prefix 00
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  {  0,   1+2,   1}, // 00s
  // prefix 010
  {  0,   1+3,   2}, // 010s
  {  0,   1+3,   2}, // 010s
  {  0,   1+3,   2}, // 010s
  {  0,   1+3,   2}, // 010s
  // prefix 011
  { -1,     4,   0}, // 0110 (EOB)
  { -1,     4,   0}, // 0110 (EOB)
  {  1,   1+4,   1}, // 0111s
  {  1,   1+4,   1}, // 0111s
  // prefix 100
  {  0,   1+4,   3}, // 1000s
  {  0,   1+4,   3}, // 1000s
  {  0,   1+4,   4}, // 1001s
  {  0,   1+4,   4}, // 1001s
  // prefix 101
  {  2,   1+5,   1}, // 10100s
  {  1,   1+5,   2}, // 10101s
  {  0,   1+5,   5}, // 10110s
  {  0,   1+5,   6}, // 10111s
  // prefix 110-111
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
  { -1,   VLC_NOBITS,  -1},
}; // dv_vlc_lookup1

dv_vlc_tab_t dv_vlc_class1_shortcut[128];

const dv_vlc_tab_t dv_vlc_lookup2[128] = 
{ 
  // prefix 110
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  3,   1+6,   1}, // 110000s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  4,   1+6,   1}, // 110001s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   7}, // 110010s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  {  0,   1+6,   8}, // 110011s
  // prefix 1101
  {  5,   1+7,   1}, // 1101000s
  {  5,   1+7,   1}, // 1101000s
  {  5,   1+7,   1}, // 1101000s
  {  5,   1+7,   1}, // 1101000s
  {  6,   1+7,   1}, // 1101001s
  {  6,   1+7,   1}, // 1101001s
  {  6,   1+7,   1}, // 1101001s
  {  6,   1+7,   1}, // 1101001s
  {  2,   1+7,   2}, // 1101010s
  {  2,   1+7,   2}, // 1101010s
  {  2,   1+7,   2}, // 1101010s
  {  2,   1+7,   2}, // 1101010s
  {  1,   1+7,   3}, // 1101011s
  {  1,   1+7,   3}, // 1101011s
  {  1,   1+7,   3}, // 1101011s
  {  1,   1+7,   3}, // 1101011s
  {  1,   1+7,   4}, // 1101100s
  {  1,   1+7,   4}, // 1101100s
  {  1,   1+7,   4}, // 1101100s
  {  1,   1+7,   4}, // 1101100s
  {  0,   1+7,   9}, // 1101101s
  {  0,   1+7,   9}, // 1101101s
  {  0,   1+7,   9}, // 1101101s
  {  0,   1+7,   9}, // 1101101s
  {  0,   1+7,  10}, // 1101110s
  {  0,   1+7,  10}, // 1101110s
  {  0,   1+7,  10}, // 1101110s
  {  0,   1+7,  10}, // 1101110s
  {  0,   1+7,  11}, // 1101111s
  {  0,   1+7,  11}, // 1101111s
  {  0,   1+7,  11}, // 1101111s
  {  0,   1+7,  11}, // 1101111s
  // prefix 1110
  {  7,   1+8,   1}, // 11100000s
  {  7,   1+8,   1}, // 11100000s
  {  8,   1+8,   1}, // 11100001s
  {  8,   1+8,   1}, // 11100001s
  {  9,   1+8,   1}, // 11100010s
  {  9,   1+8,   1}, // 11100010s
  { 10,   1+8,   1}, // 11100011s
  { 10,   1+8,   1}, // 11100011s
  {  3,   1+8,   2}, // 11100100s
  {  3,   1+8,   2}, // 11100100s
  {  4,   1+8,   2}, // 11100101s
  {  4,   1+8,   2}, // 11100101s
  {  2,   1+8,   3}, // 11100110s
  {  2,   1+8,   3}, // 11100110s
  {  1,   1+8,   5}, // 11100111s
  {  1,   1+8,   5}, // 11100111s
  {  1,   1+8,   6}, // 11101000s
  {  1,   1+8,   6}, // 11101000s
  {  1,   1+8,   7}, // 11101001s
  {  1,   1+8,   7}, // 11101001s
  {  0,   1+8,  12}, // 11101010s
  {  0,   1+8,  12}, // 11101010s
  {  0,   1+8,  13}, // 11101011s
  {  0,   1+8,  13}, // 11101011s
  {  0,   1+8,  14}, // 11101100s
  {  0,   1+8,  14}, // 11101100s
  {  0,   1+8,  15}, // 11101101s
  {  0,   1+8,  15}, // 11101101s
  {  0,   1+8,  16}, // 11101110s
  {  0,   1+8,  16}, // 11101110s
  {  0,   1+8,  17}, // 11101111s
  {  0,   1+8,  17}, // 11101111s
  // prefix 1111 0
  { 11,   1+9,   1}, // 1111 0000 0s
  { 12,   1+9,   1}, // 1111 0000 1s
  { 13,   1+9,   1}, // 1111 0001 0s
  { 14,   1+9,   1}, // 1111 0001 1s
  { 5,   1+9,	 2}, // 1111 0010 0s
  { 6,   1+9,	 2}, // 1111 0010 1s
  { 3,   1+9,	 3}, // 1111 0011 0s
  { 4,   1+9,	 3}, // 1111 0011 1s
  { 2,   1+9,	 4}, // 1111 0100 0s
  { 2,   1+9,	 5}, // 1111 0100 1s
  { 1,   1+9,	 8}, // 1111 0101 0s
  { 0,   1+9,	18}, // 1111 0101 1s
  { 0,   1+9,	19}, // 1111 0110 0s
  { 0,   1+9,	20}, // 1111 0110 1s
  { 0,   1+9,	21}, // 1111 0111 0s
  { 0,   1+9,	22}, // 1111 0111 1s
  // prefix 1111 1
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
  { -1,  VLC_NOBITS, -1},
}; // dv_vlc_lookup2

const dv_vlc_tab_t dv_vlc_lookup3[64] = 
{ 
  // len 10+1/prefix 1111 1000
  {  5,  1+10,    3}, // 1111 1000 00s
  {  5,  1+10,    3}, // 1111 1000 00s
  {  5,  1+10,    3}, // 1111 1000 00s
  {  5,  1+10,    3}, // 1111 1000 00s
  {  3,  1+10,    4}, // 1111 1000 01s
  {  3,  1+10,    4}, // 1111 1000 01s
  {  3,  1+10,    4}, // 1111 1000 01s
  {  3,  1+10,    4}, // 1111 1000 01s
  {  3,  1+10,    5}, // 1111 1000 10s
  {  3,  1+10,    5}, // 1111 1000 10s
  {  3,  1+10,    5}, // 1111 1000 10s
  {  3,  1+10,    5}, // 1111 1000 10s
  {  2,  1+10,    6}, // 1111 1000 11s
  {  2,  1+10,    6}, // 1111 1000 11s
  {  2,  1+10,    6}, // 1111 1000 11s
  {  2,  1+10,    6}, // 1111 1000 11s
  // len 10+1/prefix 1111 1001 0
  {  1,  1+10,    9}, // 1111 1001 00s
  {  1,  1+10,    9}, // 1111 1001 00s
  {  1,  1+10,    9}, // 1111 1001 00s
  {  1,  1+10,    9}, // 1111 1001 00s
  {  1,  1+10,   10}, // 1111 1001 01s
  {  1,  1+10,   10}, // 1111 1001 01s
  {  1,  1+10,   10}, // 1111 1001 01s
  {  1,  1+10,   10}, // 1111 1001 01s
  // len 10+1/prefix 1111 1001 10
  {  1,  1+10,   11}, // 1111 1001 10s
  {  1,  1+10,   11}, // 1111 1001 10s
  {  1,  1+10,   11}, // 1111 1001 10s
  {  1,  1+10,   11}, // 1111 1001 10s
  // len 11/prefix 1111 1001 11
  {  0,  11,	0}, // 1111 1001 110s
  {  0,  11,	0}, // 1111 1001 110s
  {  1,  11,	0}, // 1111 1001 111s
  {  1,  11,	0}, // 1111 1001 111s
  // len 11+1/prefix 1111 1010 0
  {  6,  1+11,    3}, // 1111 1010 000s
  {  6,  1+11,    3}, // 1111 1010 000s
  {  4,  1+11,    4}, // 1111 1010 001s
  {  4,  1+11,    4}, // 1111 1010 001s
  {  3,  1+11,    6}, // 1111 1010 010s
  {  3,  1+11,    6}, // 1111 1010 010s
  {  1,  1+11,   12}, // 1111 1010 011s
  {  1,  1+11,   12}, // 1111 1010 011s
  // len 11+1/prefix 1111 1010 10
  {  1,  1+11,  13}, // 1111 1010 100s
  {  1,  1+11,  13}, // 1111 1010 100s
  {  1,  1+11,  14}, // 1111 1010 101s
  {  1,  1+11,  14}, // 1111 1010 101s
  // len 12/prefix 1111 1010 11
  {  2,  12,   0}, // 1111 1010 1100
  {  3,  12,   0}, // 1111 1010 1101
  {  4,  12,   0}, // 1111 1010 1110
  {  5,  12,   0}, // 1111 1010 1111
  // len 12+1/prefix 1111 1011
  { 7,  1+12,	2}, // 1111 1011 0000s
  { 8,  1+12,	2}, // 1111 1011 0001s
  { 9,  1+12,	2}, // 1111 1011 0010s
  { 10, 1+12,	2}, // 1111 1011 0011s
  { 7,  1+12,	3}, // 1111 1011 0100s
  { 8,  1+12,	3}, // 1111 1011 0101s
  { 4,  1+12,	5}, // 1111 1011 0110s
  { 3,  1+12,	7}, // 1111 1011 0111s
  { 2,  1+12,	7}, // 1111 1011 1000s
  { 2,  1+12,	8}, // 1111 1011 1001s
  { 2,  1+12,	9}, // 1111 1011 1010s
  { 2,  1+12,  10}, // 1111 1011 1011s
  { 2,  1+12,  11}, // 1111 1011 1100s
  { 1,  1+12,  15}, // 1111 1011 1101s
  { 1,  1+12,  16}, // 1111 1011 1110s
  { 1,  1+12,  17}, // 1111 1011 1111s
}; // dv_vlc_lookup3

dv_vlc_tab_t dv_vlc_lookup4[64] = 
{  // len 13/prefix 1111 110
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
}; // dv_vlc_lookup4

dv_vlc_tab_t dv_vlc_lookup5[256] = 
{ // len 15+1/prefix 1111 111
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
  { 0,  VLC_ERROR,  -1},
}; // dv_vlc_lookup6

const dv_vlc_tab_t *dv_vlc_lookups[6] = 
{
  dv_vlc_broken,
  dv_vlc_lookup1,
  dv_vlc_lookup2,
  dv_vlc_lookup3,
  dv_vlc_lookup4,
  dv_vlc_lookup5,
}; // dv_vlc_lookups

const gint dv_vlc_index_mask[6] = 
{
  0x0000,  // no choice
  0xf800,  // 5 bit selector [0:4]
  0x3f80,  // 7 bit selector [2:8]
  0x03f0,  // 6 bit selector [6:11] 	
  0x01f8,  // 6 bit selector [7:12] 	
  0x01fe,  // 8 bit selector [7:14]
}; // dv_vlc_index_mask

const gint dv_vlc_index_rshift[6] = 
{
  0,
  11,
  7,
  4,
  3,
  1,
}; // 

const gint sign_mask[17] = 
{ 
  0,
  0x1 << 15,
  0x1 << 14,
  0x1 << 13,
  0x1 << 12,
  0x1 << 11,
  0x1 << 10,
  0x1 << 9,
  0x1 << 8,
  0x1 << 7,
  0x1 << 6,
  0x1 << 5,
  0x1 << 4,
  0x1 << 3,
  0x1 << 2,
  0x1 << 1,
  0x1,
};

const gint sign_rshift[17] = 
{ 
  0,
  15,
  14,
  13,
  12,
  11,
  10,
  9,
  8,
  7,
  6,
  5,
  4,
  3,
  2,
  1,
  0,
};

void dv_construct_vlc_table() {
  int i;
      
  for(i=6; i<62; i++) {
    dv_vlc_lookup4[i].run = i;
    dv_vlc_lookup4[i].amp = 0;
    dv_vlc_lookup4[i].len = 13;
  } // for
  for(i=23; i<256; i++) {
    dv_vlc_lookup5[i].run = 0;
    dv_vlc_lookup5[i].amp = i;
    dv_vlc_lookup5[i].len = 1 + 15;
  } // for

  /* Build dv_vlc_class1_shortcut[] by attempting to match class 1 and
     class 2 vlcs. */
  for (i = 0; i < 128; i++) {
    guint bits = i << 9;
    guint ms7 = ((bits & 0xfe00) >> 9);
    const dv_vlc_t *result;

    if (ms7 <= 0x5f) {
      /* class 1 */
      result = &dv_vlc_lookup1[i >> 2];
    } else if (ms7 <= 0x7b) {
      /* class 2 */
      result = &dv_vlc_lookup2[(bits & dv_vlc_index_mask[2]) >> dv_vlc_index_rshift[2]];
      if (result->len > 7)
	result = NULL;
    } else {
      result = NULL;
    }

    /* result is non-NULL if a vlc matched. */
    if (result) {
      dv_vlc_class1_shortcut[i] = *result;
      if ((result->amp > 0) && ((bits >> sign_rshift[result->len]) & 1))
	dv_vlc_class1_shortcut[i].amp *= -1;
    } else {
      dv_vlc_class1_shortcut[i] = dv_vlc_lookup1[0x1f];
    }
  }
} // dv_construct_vlc_table

/* Note we assume bits is right (lsb) aligned, and that (0 < maxbits <
 * 17).  This may look crazy, but there are no branches here. */

void dv_decode_vlc(gint bits,gint maxbits, dv_vlc_t *result) 
{
  static dv_vlc_t vlc_broken = { -1, VLC_NOBITS, -1};
  dv_vlc_t *results[2] = { &vlc_broken, result };
  gint class, has_sign, amps[2];
  
  // note that BITS is left aligned
  class = dv_vlc_classes[maxbits][(bits & (dv_vlc_class_index_mask[maxbits])) >> (dv_vlc_class_index_rshift[maxbits])];
  *result = dv_vlc_lookups[class][(bits & (dv_vlc_index_mask[class])) >> (dv_vlc_index_rshift[class])];
  amps[1] = -(amps[0] = result->amp);
  has_sign = amps[0] > 0;
  result->amp = amps[has_sign &  // or vlc not valid
		       (bits >> sign_rshift[result->len])];
  *result = *results[maxbits >= result->len];
} // dv_decode_vlc

/* Fastpath version of previous function; assumes full 16bits are
   available, which eleminates left align of input, check for enough
   bits at end, and hardcodes lookups on maxbits. */

void __dv_decode_vlc(gint bits, dv_vlc_t *result) {
  gint class, has_sign, amps[2];
  
  class = dv_vlc_classes[16][(bits & (dv_vlc_class_index_mask[16])) >> (dv_vlc_class_index_rshift[16])];
  *result = dv_vlc_lookups[class][(bits & (dv_vlc_index_mask[class])) >> (dv_vlc_index_rshift[class])];
  amps[1] = -(amps[0] = result->amp);
  has_sign = amps[0] > 0;
  result->amp = amps[has_sign &  // or vlc not valid
		    (bits >> sign_rshift[result->len])];
} // __dv_decode_vlc
