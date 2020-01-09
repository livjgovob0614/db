/* maxicode.c - Handles Maxicode */

/*
    libzint - the open source barcode library
    Copyright (C) 2010-2016 Robin Stuart <rstuart114@gmail.com>

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. Neither the name of the project nor the names of its contributors
       may be used to endorse or promote products derived from this software
       without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
 */

/* Includes corrections thanks to Monica Swanson @ Source Technologies */
#include "common.h"
#pragma hdrstop
//#include "maxicode.h"
//
// Handles Maxicode
//
static const int16 MaxiGrid[] = { // @sobolev int-->int16
    // ISO/IEC 16023 Figure 5 - MaxiCode Module Sequence */ /* 30 x 33 data grid 
    122, 121, 128, 127, 134, 133, 140, 139, 146, 145, 152, 151, 158, 157, 164, 163, 170, 169, 176, 175, 182, 181, 188, 187, 194, 193, 200, 199, 0, 0,
    124, 123, 130, 129, 136, 135, 142, 141, 148, 147, 154, 153, 160, 159, 166, 165, 172, 171, 178, 177, 184, 183, 190, 189, 196, 195, 202, 201, 817, 0,
    126, 125, 132, 131, 138, 137, 144, 143, 150, 149, 156, 155, 162, 161, 168, 167, 174, 173, 180, 179, 186, 185, 192, 191, 198, 197, 204, 203, 819, 818,
    284, 283, 278, 277, 272, 271, 266, 265, 260, 259, 254, 253, 248, 247, 242, 241, 236, 235, 230, 229, 224, 223, 218, 217, 212, 211, 206, 205, 820, 0,
    286, 285, 280, 279, 274, 273, 268, 267, 262, 261, 256, 255, 250, 249, 244, 243, 238, 237, 232, 231, 226, 225, 220, 219, 214, 213, 208, 207, 822, 821,
    288, 287, 282, 281, 276, 275, 270, 269, 264, 263, 258, 257, 252, 251, 246, 245, 240, 239, 234, 233, 228, 227, 222, 221, 216, 215, 210, 209, 823, 0,
    290, 289, 296, 295, 302, 301, 308, 307, 314, 313, 320, 319, 326, 325, 332, 331, 338, 337, 344, 343, 350, 349, 356, 355, 362, 361, 368, 367, 825, 824,
    292, 291, 298, 297, 304, 303, 310, 309, 316, 315, 322, 321, 328, 327, 334, 333, 340, 339, 346, 345, 352, 351, 358, 357, 364, 363, 370, 369, 826, 0,
    294, 293, 300, 299, 306, 305, 312, 311, 318, 317, 324, 323, 330, 329, 336, 335, 342, 341, 348, 347, 354, 353, 360, 359, 366, 365, 372, 371, 828, 827,
    410, 409, 404, 403, 398, 397, 392, 391, 80, 79, 0, 0, 14, 13, 38, 37, 3, 0, 45, 44, 110, 109, 386, 385, 380, 379, 374, 373, 829, 0,
    412, 411, 406, 405, 400, 399, 394, 393, 82, 81, 41, 0, 16, 15, 40, 39, 4, 0, 0, 46, 112, 111, 388, 387, 382, 381, 376, 375, 831, 830,
    414, 413, 408, 407, 402, 401, 396, 395, 84, 83, 42, 0, 0, 0, 0, 0, 6, 5, 48, 47, 114, 113, 390, 389, 384, 383, 378, 377, 832, 0,
    416, 415, 422, 421, 428, 427, 104, 103, 56, 55, 17, 0, 0, 0, 0, 0, 0, 0, 21, 20, 86, 85, 434, 433, 440, 439, 446, 445, 834, 833,
    418, 417, 424, 423, 430, 429, 106, 105, 58, 57, 0, 0, 0, 0, 0, 0, 0, 0, 23, 22, 88, 87, 436, 435, 442, 441, 448, 447, 835, 0,
    420, 419, 426, 425, 432, 431, 108, 107, 60, 59, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 90, 89, 438, 437, 444, 443, 450, 449, 837, 836,
    482, 481, 476, 475, 470, 469, 49, 0, 31, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 54, 53, 464, 463, 458, 457, 452, 451, 838, 0,
    484, 483, 478, 477, 472, 471, 50, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 466, 465, 460, 459, 454, 453, 840, 839,
    486, 485, 480, 479, 474, 473, 52, 51, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 43, 468, 467, 462, 461, 456, 455, 841, 0,
    488, 487, 494, 493, 500, 499, 98, 97, 62, 61, 0, 0, 0, 0, 0, 0, 0, 0, 0, 27, 92, 91, 506, 505, 512, 511, 518, 517, 843, 842,
    490, 489, 496, 495, 502, 501, 100, 99, 64, 63, 0, 0, 0, 0, 0, 0, 0, 0, 29, 28, 94, 93, 508, 507, 514, 513, 520, 519, 844, 0,
    492, 491, 498, 497, 504, 503, 102, 101, 66, 65, 18, 0, 0, 0, 0, 0, 0, 0, 19, 30, 96, 95, 510, 509, 516, 515, 522, 521, 846, 845,
    560, 559, 554, 553, 548, 547, 542, 541, 74, 73, 33, 0, 0, 0, 0, 0, 0, 11, 68, 67, 116, 115, 536, 535, 530, 529, 524, 523, 847, 0,
    562, 561, 556, 555, 550, 549, 544, 543, 76, 75, 0, 0, 8, 7, 36, 35, 12, 0, 70, 69, 118, 117, 538, 537, 532, 531, 526, 525, 849, 848,
    564, 563, 558, 557, 552, 551, 546, 545, 78, 77, 0, 34, 10, 9, 26, 25, 0, 0, 72, 71, 120, 119, 540, 539, 534, 533, 528, 527, 850, 0,
    566, 565, 572, 571, 578, 577, 584, 583, 590, 589, 596, 595, 602, 601, 608, 607, 614, 613, 620, 619, 626, 625, 632, 631, 638, 637, 644, 643, 852, 851,
    568, 567, 574, 573, 580, 579, 586, 585, 592, 591, 598, 597, 604, 603, 610, 609, 616, 615, 622, 621, 628, 627, 634, 633, 640, 639, 646, 645, 853, 0,
    570, 569, 576, 575, 582, 581, 588, 587, 594, 593, 600, 599, 606, 605, 612, 611, 618, 617, 624, 623, 630, 629, 636, 635, 642, 641, 648, 647, 855, 854,
    728, 727, 722, 721, 716, 715, 710, 709, 704, 703, 698, 697, 692, 691, 686, 685, 680, 679, 674, 673, 668, 667, 662, 661, 656, 655, 650, 649, 856, 0,
    730, 729, 724, 723, 718, 717, 712, 711, 706, 705, 700, 699, 694, 693, 688, 687, 682, 681, 676, 675, 670, 669, 664, 663, 658, 657, 652, 651, 858, 857,
    732, 731, 726, 725, 720, 719, 714, 713, 708, 707, 702, 701, 696, 695, 690, 689, 684, 683, 678, 677, 672, 671, 666, 665, 660, 659, 654, 653, 859, 0,
    734, 733, 740, 739, 746, 745, 752, 751, 758, 757, 764, 763, 770, 769, 776, 775, 782, 781, 788, 787, 794, 793, 800, 799, 806, 805, 812, 811, 861, 860,
    736, 735, 742, 741, 748, 747, 754, 753, 760, 759, 766, 765, 772, 771, 778, 777, 784, 783, 790, 789, 796, 795, 802, 801, 808, 807, 814, 813, 862, 0,
    738, 737, 744, 743, 750, 749, 756, 755, 762, 761, 768, 767, 774, 773, 780, 779, 786, 785, 792, 791, 798, 797, 804, 803, 810, 809, 816, 815, 864, 863
};

static const int8 maxiCodeSet[256] = { // @sobolev int-->int8
    // from Appendix A - ASCII character to Code Set (e.g. 2 = Set B) 
    // set 0 refers to special characters that fit into more than one set (e.g. GS) 
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 0, 0, 0, 5, 0, 2, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 2,
    2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 4, 5, 5, 5, 5, 5, 5, 4, 5, 3, 4, 3, 5, 5, 4, 4, 3, 3, 3,
    4, 3, 5, 4, 4, 3, 3, 4, 3, 3, 3, 4, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
    4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4
};

static const int8 maxiSymbolChar[256] = { // @sobolev int-->int8
    // from Appendix A - ASCII character to symbol value 
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    20, 21, 22, 23, 24, 25, 26, 30, 28, 29, 30, 35, 32, 53, 34, 35, 36, 37, 38, 39,
    40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 37,
    38, 39, 40, 41, 52, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 42, 43, 44, 45, 46, 0, 1, 2, 3,
    4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23,
    24, 25, 26, 32, 54, 34, 35, 36, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 47, 48,
    49, 50, 51, 52, 53, 54, 55, 56, 57, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 36,
    37, 37, 38, 39, 40, 41, 42, 43, 38, 44, 37, 39, 38, 45, 46, 40, 41, 39, 40, 41,
    42, 42, 47, 43, 44, 43, 44, 45, 45, 46, 47, 46, 0, 1, 2, 3, 4, 5, 6, 7,
    8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 32,
    33, 34, 35, 36, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 32, 33, 34, 35, 36
};

static int maxi_codeword[144]; // @global
//
// Handles error correction of primary message 
//
void maxi_do_primary_check()
{
	uchar data[15];
	uchar results[15];
	int j;
	int datalen = 10;
	int ecclen = 10;

	rs_init_gf(0x43);
	rs_init_code(ecclen, 1);

	for(j = 0; j < datalen; j += 1)
		data[j] = maxi_codeword[j];

	rs_encode(datalen, data, results);

	for(j = 0; j < ecclen; j += 1)
		maxi_codeword[ datalen + j] = results[ecclen - 1 - j];
	rs_free();
}

/* Handles error correction of odd characters in secondary */
void maxi_do_secondary_chk_odd(int ecclen)
{
	uchar data[100];
	uchar results[30];
	int j;
	int datalen = 68;

	rs_init_gf(0x43);
	rs_init_code(ecclen, 1);

	if(ecclen == 20)
		datalen = 84;

	for(j = 0; j < datalen; j += 1)
		if(j & 1) // odd
			data[(j - 1) / 2] = maxi_codeword[j + 20];

	rs_encode(datalen / 2, data, results);

	for(j = 0; j < (ecclen); j += 1)
		maxi_codeword[ datalen + (2 * j) + 1 + 20 ] = results[ecclen - 1 - j];
	rs_free();
}

/* Handles error correction of even characters in secondary */
void maxi_do_secondary_chk_even(int ecclen)
{
	uchar data[100];
	uchar results[30];
	int j;
	int datalen = 68;

	if(ecclen == 20)
		datalen = 84;

	rs_init_gf(0x43);
	rs_init_code(ecclen, 1);

	for(j = 0; j < datalen + 1; j += 1)
		if(!(j & 1)) // even
			data[j / 2] = maxi_codeword[j + 20];

	rs_encode(datalen / 2, data, results);

	for(j = 0; j < (ecclen); j += 1)
		maxi_codeword[ datalen + (2 * j) + 20] = results[ecclen - 1 - j];
	rs_free();
}

/* Moves everything up so that a shift or latch can be inserted */
void maxi_bump(int set[], int character[], int bump_posn)
{
	for(int i = 143; i > bump_posn; i--) {
		set[i] = set[i - 1];
		character[i] = character[i - 1];
	}
}

/* Format text according to Appendix A */
static int maxi_text_process(int mode, const uchar source[], int length, int eci)
{
	/* This code doesn't make use of [Lock in C], [Lock in D]
	   and [Lock in E] and so is not always the most efficient at
	   compressing data, but should suffice for most applications */

	int set[144], character[144], i, j, done, count, current_set;
	if(length > 138) {
		return ZINT_ERROR_TOO_LONG;
	}
	for(i = 0; i < 144; i++) {
		set[i] = -1;
		character[i] = 0;
	}
	for(i = 0; i < length; i++) {
		// Look up characters in table from Appendix A - this gives value and code set for most characters 
		set[i] = maxiCodeSet[source[i]];
		character[i] = maxiSymbolChar[source[i]];
	}
	// If a character can be represented in more than one code set, pick which version to use 
	if(set[0] == 0) {
		if(character[0] == 13) {
			character[0] = 0;
		}
		set[0] = 1;
	}
	for(i = 1; i < length; i++) {
		if(set[i] == 0) {
			done = 0;
			// Special character 
			if(character[i] == 13) {
				// Carriage Return 
				if(set[i - 1] == 5) {
					character[i] = 13;
					set[i] = 5;
				}
				else {
					if((i != length - 1) && (set[i + 1] == 5)) {
						character[i] = 13;
						set[i] = 5;
					}
					else {
						character[i] = 0;
						set[i] = 1;
					}
				}
				done = 1;
			}

			if((character[i] == 28) && (done == 0)) {
				/* FS */
				if(set[i - 1] == 5) {
					character[i] = 32;
					set[i] = 5;
				}
				else {
					set[i] = set[i - 1];
				}
				done = 1;
			}

			if((character[i] == 29) && (done == 0)) {
				/* GS */
				if(set[i - 1] == 5) {
					character[i] = 33;
					set[i] = 5;
				}
				else {
					set[i] = set[i - 1];
				}
				done = 1;
			}

			if((character[i] == 30) && (done == 0)) {
				/* RS */
				if(set[i - 1] == 5) {
					character[i] = 34;
					set[i] = 5;
				}
				else {
					set[i] = set[i - 1];
				}
				done = 1;
			}

			if((character[i] == 32) && (done == 0)) {
				/* Space */
				if(set[i - 1] == 1) {
					character[i] = 32;
					set[i] = 1;
				}
				if(set[i - 1] == 2) {
					character[i] = 47;
					set[i] = 2;
				}
				if(set[i - 1] >= 3) {
					if(i != length - 1) {
						if(set[i + 1] == 1) {
							character[i] = 32;
							set[i] = 1;
						}
						if(set[i + 1] == 2) {
							character[i] = 47;
							set[i] = 2;
						}
						if(set[i + 1] >= 3) {
							character[i] = 59;
							set[i] = set[i - 1];
						}
					}
					else {
						character[i] = 59;
						set[i] = set[i - 1];
					}
				}
				done = 1;
			}

			if((character[i] == 44) && (done == 0)) {
				/* Comma */
				if(set[i - 1] == 2) {
					character[i] = 48;
					set[i] = 2;
				}
				else {
					if((i != length - 1) && (set[i + 1] == 2)) {
						character[i] = 48;
						set[i] = 2;
					}
					else {
						set[i] = 1;
					}
				}
				done = 1;
			}

			if((character[i] == 46) && (done == 0)) {
				/* Full Stop */
				if(set[i - 1] == 2) {
					character[i] = 49;
					set[i] = 2;
				}
				else {
					if((i != length - 1) && (set[i + 1] == 2)) {
						character[i] = 49;
						set[i] = 2;
					}
					else {
						set[i] = 1;
					}
				}
				done = 1;
			}

			if((character[i] == 47) && (done == 0)) {
				/* Slash */
				if(set[i - 1] == 2) {
					character[i] = 50;
					set[i] = 2;
				}
				else {
					if((i != length - 1) && (set[i + 1] == 2)) {
						character[i] = 50;
						set[i] = 2;
					}
					else {
						set[i] = 1;
					}
				}
				done = 1;
			}

			if((character[i] == 58) && (done == 0)) {
				/* Colon */
				if(set[i - 1] == 2) {
					character[i] = 51;
					set[i] = 2;
				}
				else {
					if((i != length - 1) && (set[i + 1] == 2)) {
						character[i] = 51;
						set[i] = 2;
					}
					else {
						set[i] = 1;
					}
				}
				done = 1;
			}
		}
	}

	for(i = length; i < 144; i++) {
		/* Add the padding */
		if(set[length - 1] == 2) {
			set[i] = 2;
		}
		else {
			set[i] = 1;
		}
		character[i] = 33;
	}

	/* Find candidates for number compression */
	if((mode == 2) || (mode == 3)) {
		j = 0;
	}
	else {
		j = 9;
	}
	/* Number compression not allowed in primary message */
	count = 0;
	for(i = j; i < 143; i++) {
		if((set[i] == 1) && ((character[i] >= 48) && (character[i] <= 57))) {
			/* Character is a number */
			count++;
		}
		else {
			count = 0;
		}
		if(count == 9) {
			/* Nine digits in a row can be compressed */
			set[i] = 6;
			set[i - 1] = 6;
			set[i - 2] = 6;
			set[i - 3] = 6;
			set[i - 4] = 6;
			set[i - 5] = 6;
			set[i - 6] = 6;
			set[i - 7] = 6;
			set[i - 8] = 6;
			count = 0;
		}
	}

	/* Add shift and latch characters */
	current_set = 1;
	i = 0;
	do {
		if((set[i] != current_set) && (set[i] != 6)) {
			switch(set[i]) {
				case 1:
				    if(set[i + 1] == 1) {
					    if(set[i + 2] == 1) {
						    if(set[i + 3] == 1) {
							    /* Latch A */
							    maxi_bump(set, character, i);
							    character[i] = 63;
							    current_set = 1;
							    length++;
						    }
						    else {
							    /* 3 Shift A */
							    maxi_bump(set, character, i);
							    character[i] = 57;
							    length++;
							    i += 2;
						    }
					    }
					    else {
						    /* 2 Shift A */
						    maxi_bump(set, character, i);
						    character[i] = 56;
						    length++;
						    i++;
					    }
				    }
				    else {
					    /* Shift A */
					    maxi_bump(set, character, i);
					    character[i] = 59;
					    length++;
				    }
				    break;
				case 2:
				    if(set[i + 1] == 2) {
					    /* Latch B */
					    maxi_bump(set, character, i);
					    character[i] = 63;
					    current_set = 2;
					    length++;
				    }
				    else {
					    /* Shift B */
					    maxi_bump(set, character, i);
					    character[i] = 59;
					    length++;
				    }
				    break;
				case 3:
				    /* Shift C */
				    maxi_bump(set, character, i);
				    character[i] = 60;
				    length++;
				    break;
				case 4:
				    /* Shift D */
				    maxi_bump(set, character, i);
				    character[i] = 61;
				    length++;
				    break;
				case 5:
				    /* Shift E */
				    maxi_bump(set, character, i);
				    character[i] = 62;
				    length++;
				    break;
			}
			i++;
		}
		i++;
	} while(i < 145);

	/* Number compression has not been forgotten! - It's handled below */
	i = 0;
	do {
		if(set[i] == 6) {
			/* Number compression */
			char substring[11];
			int value;

			for(j = 0; j < 9; j++) {
				substring[j] = character[i + j];
			}
			substring[9] = '\0';
			value = atoi(substring);

			character[i] = 31; /* NS */
			character[i + 1] = (value & 0x3f000000) >> 24;
			character[i + 2] = (value & 0xfc0000) >> 18;
			character[i + 3] = (value & 0x3f000) >> 12;
			character[i + 4] = (value & 0xfc0) >> 6;
			character[i + 5] = (value & 0x3f);

			i += 6;
			for(j = i; j < 140; j++) {
				set[j] = set[j + 3];
				character[j] = character[j + 3];
			}
			length -= 3;
		}
		else {
			i++;
		}
	} while(i <= 143);

	/* Insert ECI at the beginning of message if needed */
	if(eci != 3) {
		maxi_bump(set, character, 0);
		character[0] = 27; // ECI
		maxi_bump(set, character, 1);
		character[1] = eci;
		length += 2;
	}

	if(((mode == 2) || (mode == 3)) && (length > 84)) {
		return ZINT_ERROR_TOO_LONG;
	}

	if(((mode == 4) || (mode == 6)) && (length > 93)) {
		return ZINT_ERROR_TOO_LONG;
	}

	if((mode == 5) && (length > 77)) {
		return ZINT_ERROR_TOO_LONG;
	}

	/* Copy the encoded text into the codeword array */
	if((mode == 2) || (mode == 3)) {
		for(i = 0; i < 84; i++) { /* secondary only */
			maxi_codeword[i + 20] = character[i];
		}
	}

	if((mode == 4) || (mode == 6)) {
		for(i = 0; i < 9; i++) { /* primary */
			maxi_codeword[i + 1] = character[i];
		}
		for(i = 0; i < 84; i++) { /* secondary */
			maxi_codeword[i + 20] = character[i + 9];
		}
	}

	if(mode == 5) {
		for(i = 0; i < 9; i++) { /* primary */
			maxi_codeword[i + 1] = character[i];
		}
		for(i = 0; i < 68; i++) { /* secondary */
			maxi_codeword[i + 20] = character[i + 9];
		}
	}

	return 0;
}

/* Format structured primary for Mode 2 */
void maxi_do_primary_2(char postcode[], int country, int service)
{
	int postcode_length, postcode_num, i;

	for(i = 0; i < 10; i++) {
		if((postcode[i] < '0') || (postcode[i] > '9')) {
			postcode[i] = '\0';
		}
	}

	postcode_length = strlen(postcode);
	postcode_num = atoi(postcode);

	maxi_codeword[0] = ((postcode_num & 0x03) << 4) | 2;
	maxi_codeword[1] = ((postcode_num & 0xfc) >> 2);
	maxi_codeword[2] = ((postcode_num & 0x3f00) >> 8);
	maxi_codeword[3] = ((postcode_num & 0xfc000) >> 14);
	maxi_codeword[4] = ((postcode_num & 0x3f00000) >> 20);
	maxi_codeword[5] = ((postcode_num & 0x3c000000) >> 26) | ((postcode_length & 0x3) << 4);
	maxi_codeword[6] = ((postcode_length & 0x3c) >> 2) | ((country & 0x3) << 4);
	maxi_codeword[7] = (country & 0xfc) >> 2;
	maxi_codeword[8] = ((country & 0x300) >> 8) | ((service & 0xf) << 2);
	maxi_codeword[9] = ((service & 0x3f0) >> 4);
}

/* Format structured primary for Mode 3 */
void maxi_do_primary_3(char postcode[], int country, int service)
{
	int i, h;

	h = strlen(postcode);
	to_upper((uchar *)postcode);
	for(i = 0; i < h; i++) {
		if((postcode[i] >= 'A') && (postcode[i] <= 'Z')) {
			/* (Capital) letters shifted to Code Set A values */
			postcode[i] -= 64;
		}
		if(((postcode[i] == 27) || (postcode[i] == 31)) || ((postcode[i] == 33) || (postcode[i] >= 59))) {
			/* Not a valid postcode character */
			postcode[i] = ' ';
		}
		/* Input characters lower than 27 (NUL - SUB) in postcode are
		   interpreted as capital letters in Code Set A (e.g. LF becomes 'J') */
	}

	maxi_codeword[0] = ((postcode[5] & 0x03) << 4) | 3;
	maxi_codeword[1] = ((postcode[4] & 0x03) << 4) | ((postcode[5] & 0x3c) >> 2);
	maxi_codeword[2] = ((postcode[3] & 0x03) << 4) | ((postcode[4] & 0x3c) >> 2);
	maxi_codeword[3] = ((postcode[2] & 0x03) << 4) | ((postcode[3] & 0x3c) >> 2);
	maxi_codeword[4] = ((postcode[1] & 0x03) << 4) | ((postcode[2] & 0x3c) >> 2);
	maxi_codeword[5] = ((postcode[0] & 0x03) << 4) | ((postcode[1] & 0x3c) >> 2);
	maxi_codeword[6] = ((postcode[0] & 0x3c) >> 2) | ((country & 0x3) << 4);
	maxi_codeword[7] = (country & 0xfc) >> 2;
	maxi_codeword[8] = ((country & 0x300) >> 8) | ((service & 0xf) << 2);
	maxi_codeword[9] = ((service & 0x3f0) >> 4);
}

int maxicode(struct ZintSymbol * symbol, uchar local_source[], int length)
{
	int i, j, block, bit, countrycode = 0, service = 0, lp = 0;
	int bit_pattern[7], internal_error = 0, eclen;
	char postcode[12], countrystr[4], servicestr[4];
	int    mode = symbol->option_1;
	sstrcpy(postcode, "");
	sstrcpy(countrystr, "");
	sstrcpy(servicestr, "");
	memzero(maxi_codeword, sizeof(maxi_codeword));
	if(mode == -1) { /* If mode is unspecified */
		lp = strlen(symbol->primary);
		if(lp == 0) {
			mode = 4;
		}
		else {
			mode = 2;
			for(i = 0; i < 10 && i < lp; i++) {
				if((symbol->primary[i] < 48) || (symbol->primary[i] > 57)) {
					mode = 3;
					break;
				}
			}
		}
	}

	if((mode < 2) || (mode > 6)) { /* Only codes 2 to 6 supported */
		sstrcpy(symbol->errtxt, "Invalid Maxicode Mode (E50)");
		return ZINT_ERROR_INVALID_OPTION;
	}

	if((mode == 2) || (mode == 3)) { /* Modes 2 and 3 need data in symbol->primary */
		if(lp == 0) { /* Mode set manually means lp doesn't get set */
			lp = strlen(symbol->primary);
		}
		if(lp != 15) {
			sstrcpy(symbol->errtxt, "Invalid Primary String (E51)");
			return ZINT_ERROR_INVALID_DATA;
		}

		for(i = 9; i < 15; i++) { /* check that country code and service are numeric */
			if((symbol->primary[i] < '0') || (symbol->primary[i] > '9')) {
				sstrcpy(symbol->errtxt, "Invalid Primary String (E52)");
				return ZINT_ERROR_INVALID_DATA;
			}
		}

		memcpy(postcode, symbol->primary, 9);
		postcode[9] = '\0';

		if(mode == 2) {
			for(i = 0; i < 10; i++) {
				if(postcode[i] == ' ') {
					postcode[i] = '\0';
				}
			}
		}
		else if(mode == 3) {
			postcode[6] = '\0';
		}

		countrystr[0] = symbol->primary[9];
		countrystr[1] = symbol->primary[10];
		countrystr[2] = symbol->primary[11];
		countrystr[3] = '\0';

		servicestr[0] = symbol->primary[12];
		servicestr[1] = symbol->primary[13];
		servicestr[2] = symbol->primary[14];
		servicestr[3] = '\0';

		countrycode = atoi(countrystr);
		service = atoi(servicestr);

		if(mode == 2) {
			maxi_do_primary_2(postcode, countrycode, service);
		}
		if(mode == 3) {
			maxi_do_primary_3(postcode, countrycode, service);
		}
	}
	else {
		maxi_codeword[0] = mode;
	}

	i = maxi_text_process(mode, local_source, length, symbol->eci);
	if(i == ZINT_ERROR_TOO_LONG) {
		sstrcpy(symbol->errtxt, "Input data too long (E53)");
		return i;
	}
	/* All the data is sorted - now do error correction */
	maxi_do_primary_check(); /* always EEC */
	if(mode == 5)
		eclen = 56;  // 68 data codewords , 56 error corrections
	else
		eclen = 40;  // 84 data codewords,  40 error corrections
	maxi_do_secondary_chk_even(eclen / 2); // do error correction of even
	maxi_do_secondary_chk_odd(eclen / 2); // do error correction of odd
	// Copy data into symbol grid 
	for(i = 0; i < 33; i++) {
		for(j = 0; j < 30; j++) {
			block = (MaxiGrid[(i * 30) + j] + 5) / 6;
			bit = (MaxiGrid[(i * 30) + j] + 5) % 6;
			if(block != 0) {
				bit_pattern[0] = (maxi_codeword[block - 1] & 0x20) >> 5;
				bit_pattern[1] = (maxi_codeword[block - 1] & 0x10) >> 4;
				bit_pattern[2] = (maxi_codeword[block - 1] & 0x8) >> 3;
				bit_pattern[3] = (maxi_codeword[block - 1] & 0x4) >> 2;
				bit_pattern[4] = (maxi_codeword[block - 1] & 0x2) >> 1;
				bit_pattern[5] = (maxi_codeword[block - 1] & 0x1);
				if(bit_pattern[bit] != 0) {
					set_module(symbol, i, j);
				}
			}
		}
	}
	/* Add orientation markings */
	set_module(symbol, 0, 28); // Top right filler
	set_module(symbol, 0, 29);
	set_module(symbol, 9, 10); // Top left marker
	set_module(symbol, 9, 11);
	set_module(symbol, 10, 11);
	set_module(symbol, 15, 7); // Left hand marker
	set_module(symbol, 16, 8);
	set_module(symbol, 16, 20); // Right hand marker
	set_module(symbol, 17, 20);
	set_module(symbol, 22, 10); // Bottom left marker
	set_module(symbol, 23, 10);
	set_module(symbol, 22, 17); // Bottom right marker
	set_module(symbol, 23, 17);

	symbol->width = 30;
	symbol->rows = 33;

	return internal_error;
}

