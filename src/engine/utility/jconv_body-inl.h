// -*- C++ -*-

/*
 * jconv_body-inl.h - japanese character code conversion library.
 *
 *   Copyright (c) 2006,2008 Tomotaka SUWA, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *   Copyright (c) 2000-2003 Shiro Kawai, All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 * 
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the authors nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// ======================================================================
// Shift-JIS -> EUC_JP
// ======================================================================

/* Shift_JISX0213 -> EUC-JP
 * 
 * Mapping anormalities
 *
 *   0x5c, 0x7e : Shift_JISX0213 mapping table maps 0x5c to U+00A5
 *       (YEN SIGN) and 0x7e to U+203E (OVERLINE).  But mapping so
 *       breaks the program code written in Shift JIS.   I map them
 *       to the corresponding ASCII chars.
 *   0xfd, 0xfe, 0xff : These are reserved bytes.  Apple uses these
 *       bytes for vendor extension:
 *        0xfd - U+00A9 COPYRIGHT SIGN     |EUC A9A6  |JISX0213
 *        0xfe - U+2122 TRADE MARK SIGN    |EUC 8FA2EF|JISX0212
 *        0xff - U+2026 HORIZONTAL ELLIPSIS|EUC A1C4  |JISX0208
 *       This is a one-direction mapping.
 *   0x80, 0xa0 : These are reserved bytes.  Replaced to the
 *       one-byte substitution character of destination encoding.
 *
 * Conversion scheme
 *   0x00-0x7f : corresponding ASCII range.
 *   0x80      : substitution character
 *   0x81 -- 0x9f : first byte (s1) of double byte range for JIS X 0213 m=1
 *   0xa0      : substitution character
 *   0xa1 -- 0xdf : JISX 0201 kana = s1-0x80
 *   0xe0 -- 0xef : first byte (s1) of double byte range for JIS X 0213 m=1
 *   0xf0 -- 0xfc : first byte (s1) of double byte range for JIS X 0213 m=2
 *   0xfd : U+00A9, EUC A9A6, JISX0213 (1, 0x09, 0x06)
 *   0xfe : U+2122, EUC 8FA2EF, JISX0212
 *   0xff : U+2026, EUC A1C4, JISX0208 (1, 0x01, 0x24)
 *
 *   For double-byte character, second byte s2 must be in the range of
 *   0x40 <= s2 <= 0x7e or 0x80 <= s2 <= 0xfc.  Otherwise, double-byte
 *   substitution character is used.
 *
 *     two bytes (s1, s2) maps to JIS X 0213 (m, k, t) by
 *        m = 1 if s1 <= 0xef, 2 otherwise
 *        k = (s1-0x80)*2 - ((s2 < 0x9f)? 1 : 0)  if s1 <= 0x9f
 *            (s1-0xc0)*2 - ((s2 < 0x9f)? 1 : 0)  if 0xe0 <= s1 <= 0xef
 *            (s1-0x9e)*2 - ((s2 < 0x89)? 1 : 0)  if s1 >= 0xf5
 *            otherwise, use the following table
 *               s1   k (s2>=0x80, s2<0x80)
 *              0xf0   (0x01, 0x08)
 *              0xf1   (0x03, 0x04)
 *              0xf2   (0x05, 0x0c)
 *              0xf3   (0x0e, 0x0d)
 *              0xf4   (0x0f, 0x4e)
 *        t = s2-0x3f if s2 < 0x7f
 *            s2-0x40 if s2 < 0x9f
 *            s2-0x9e otherwise
 *
 *     JIS X 0213 to EUC-JP is a straightfoward conversion.
 */

void sjis_to_eucj::neutral(unsigned char c) {
    // JISX0201 Latin
    if(c <= 0x7f) {
	output_ += c;
	return;
    }

    // JISX0201 Kana
    if(0xa1 <= c && c <= 0xdf) {
	output_ += 0x8e;
	output_ += c;
	return;
    }

    // JISX0213
    if((0x81 <= c && c <= 0x9f) || (0xe0 <= c && c <= 0xfc)) {
	state_ = &sjis_to_eucj::jisx0213;
	input_.clear();
	input_.push_back(c);
	return;
    }

    // copyright mark
    if(c == 0xfd) {
	output_ += 0xa9;
	output_ += 0xa6;
	return;
    }

    // trademark sign.  this is not in JISX0213, but in JISX0212.
    if(c == 0xfe) {
	output_ += 0x8f;
	output_ += 0xa2;
	output_ += 0xef;
	return;
    }

    // horizontal ellipsis.
    if(c == 0xff) {
	output_ += 0xa1;
	output_ += 0xc4;
	return;
    }

    // s1 == 0x80 or 0xa0
    output_ += subst_char;
}

void sjis_to_eucj::jisx0213(unsigned char c) {
    input_.push_back(c);

    reset();

    if(input_[1] < 0x40 || input_[1] == 0x7f || 0xfc < input_[1]) {
	subst_();
	return;
    }

    unsigned char e1, e2;

    if(input_[0] <= 0x9f) {
	e1 = (input_[0] - 0x80) * 2 - ((input_[1] < 0x9f) ? 1 : 0) + 0xa0;
    } else if(input_[0] <= 0xef) {
	e1 = (input_[0] - 0xc0) * 2 - ((input_[1] < 0x9f) ? 1 : 0) + 0xa0;
    } else if(0xf5 <= input_[0]) {
	e1 = (input_[0] - 0xf5) * 2 + 0x50 - ((input_[1] < 0x9f) ? 1 : 0) + 0xa0;
    } else {
	static unsigned char cvt[] = { 0x01, 0x08, 0x03, 0x04, 0x05, 0x0c, 0x0e, 0x0d, 0x0f, 0x4e };
	e1 = cvt[(input_[0] - 0xf0) * 2 + ((input_[1] < 0x9f) ? 0 : 1)] + 0xa0;
    }
        
    if(input_[1] < 0x7f) {
	e2 = input_[1] - 0x3f + 0xa0;
    } else if(input_[1] < 0x9f) {
	e2 = input_[1] - 0x40 + 0xa0;
    } else {
	e2 = input_[1] - 0x9e + 0xa0;
    }

    if(0xef < input_[0]) {
	output_ += 0x8f;
    }

    output_ += e1;
    output_ += e2;
}

// ======================================================================
// EUC_JP -> Shift-JIS
// ======================================================================

/* EUC_JISX0213 -> Shift_JIS
 * 
 * Mapping anormalities
 *
 *   0x80--0xa0 except 0x8e and 0x8f : C1 region.
 *          Doesn't have corresponding SJIS bytes,
 *          so mapped to substitution char.
 *   0xff : reserved byte.  mapped to substitution char.
 *
 * Conversion scheme
 *   0x00-0x7f : corresponding ASCII range.
 *   0x80--0x8d : substitution char.
 *   0x8e : leading byte of JISX 0201 kana
 *   0x8f : leading byte of JISX 0212 or JISX 0213 plane 2
 *   0x90--0xa0 : substitution char.
 *   0xa1--0xfe : first byte (e1) of JISX 0213 plane 1
 *   0xff : substitution char
 *
 *   For double or trible-byte character, subsequent byte has to be in
 *   the range between 0xa1 and 0xfe inclusive.  If not, it is replaced
 *   for the substitution character.
 *   
 *   If the first byte is in the range of 0xa1--0xfe, two bytes (e1, e2)
 *   is mapped to SJIS (s1, s2) by:
 *
 *     s1 = (e1 - 0xa0 + 0x101)/2 if 0xa1 <= e1 <= 0xde
 *          (e1 - 0xa0 + 0x181)/2 if 0xdf <= e1 <= 0xfe
 *     s2 = (e2 - 0xa0 + 0x3f) if odd?(e1) && 0xa1 <= e2 <= 0xdf
 *          (e2 - 0xa0 + 0x40) if odd?(e1) && 0xe0 <= e2 <= 0xfe
 *          (e2 - 0xa0 + 0x9e) if even?(e1)
 *
 *   If the first byte is 0x8f, the second byte (e1) and the third byte
 *   (e2) is mapped to SJIS (s1, s2) by:
 *     if (0xee <= e1 <= 0xfe)  s1 = (e1 - 0xa0 + 0x19b)/2
 *     otherwise, follow the table:
 *       e1 == 0xa1 or 0xa8  => s1 = 0xf0
 *       e1 == 0xa3 or 0xa4  => s1 = 0xf1
 *       e1 == 0xa5 or 0xac  => s1 = 0xf2
 *       e1 == 0xae or 0xad  => s1 = 0xf3
 *       e1 == 0xaf          => s1 = 0xf4
 *     If e1 is other value, it is JISX0212; we use substitution char.
 *     s2 is mapped with the same rule above.
 */

void eucj_to_sjis::neutral(unsigned char c) {
    if(c <= 0x7f) {
	output_ += c;
	return;
    }

    if(c == 0x8e) {
	state_ = &eucj_to_sjis::jisx0201_kana;
	return;
    }

    if(c == 0x8f) {
	state_ = &eucj_to_sjis::jisx0213_plane2;
	input_.clear();
	return;
    }

    if(0xa1 <= c && c <= 0xfe) {
	state_ = &eucj_to_sjis::jisx0213_plane1;
	input_.clear();
	input_.push_back(c);
	return;
    }

    output_ += subst_char;
}

void eucj_to_sjis::jisx0201_kana(unsigned char c) {
    reset(); 

    if(c < 0xa1 || c == 0xff) {
	output_ += subst_char;
    } else {
	output_ += c;
    }
}

void eucj_to_sjis::jisx0213_plane1(unsigned char c) {
    input_.push_back(c);

    reset();

    if(input_[1] < 0xa1 || input_[1] == 0xff) {
	subst_();
	return;
    }

    unsigned char s1, s2;

    if(input_[0] <= 0xde) {
	s1 = (input_[0] - 0xa0 + 0x101) / 2;
    } else {
	s1 = (input_[0] - 0xa0 + 0x181) / 2;
    }

    if(input_[0] % 2 == 0) {
	s2 = input_[1] - 0xa0 + 0x9e;
    } else {
	if(input_[1] <= 0xdf) {
	    s2 = input_[1] - 0xa0 + 0x3f;
	} else {
	    s2 = input_[1] - 0xa0 + 0x40;
	}
    }

    output_ += s1;
    output_ += s2;
}

void eucj_to_sjis::jisx0213_plane2(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 2) {
	return;
    }

    reset();

    if(input_[0] < 0xa1 || input_[0] == 0xff ||
       input_[1] < 0xa1 || input_[1] == 0xff) {
	subst_();
	return;
    }

    unsigned char s1, s2;

    if(0xee <= input_[0]) {
	s1 = (input_[0] - 0xa0 + 0x19b) / 2;
    } else if(0xb0 <= input_[0]) {
	subst_();
	return;
    } else {
	static unsigned char cvt[] = {
	    0xf0, 0x00, 0xf1, 0xf1, 0xf2,
	    0x00, 0x00, 0xf0, 0x00, 0x00,
	    0x00, 0xf2, 0xf3, 0xf3, 0xf4
	};

	s1 = cvt[input_[0] - 0xa1];
	if(s1 == 0) {
	    subst_();
	    return;
	}
    }

    if(input_[0] % 2 == 0) {
	s2 = input_[1] - 0xa0 + 0x9e;
    } else {
	if(c < 0xdf) {
	    s2 = input_[1] - 0xa0 + 0x3f;
	} else {
	    s2 = input_[1] - 0xa0 + 0x40;
	}
    }

    output_ += s1;
    output_ += s2;
}

// ======================================================================
// UTF8 -> EUC_JP
// ======================================================================

/*=================================================================
 * UTF8
 */

/* Conversion between UTF8 and EUC_JP is based on the table found at
 * http://isweb11.infoseek.co.jp/computer/wakaba/table/jis-note.ja.html
 *
 * There are some characters in JISX0213 that can't be represented
 * in a single Unicode character, but can be with a combining character.
 * In such case, EUC_JP to UTF8 conversion uses combining character,
 * but UTF8 to EUC_JP conversion translates the combining character into
 * another character.  For example, a single JISX0213 katakana 'nga'
 * (hiragana "ka" with han-dakuon mark) will translates to Unicode
 * U+304B+309A (HIRAGANA LETTER KA + COMBINING KATAKANA-HIRAGANA SEMI-VOICED
 * SOUND MARK).  When this sequence is converted to EUC_JP again, it
 * becomes EUCJ 0xA4AB + 0xA1AC.  This is an implementation limitation,
 * and should be removed in later release.
 */

/* [UTF8 -> EUC_JP conversion]
 *
 * EUC-JP has the corresponding characters to the wide range of
 * UCS characters.
 *
 *   UCS4 character   # of EUC_JP characters
 *   ---------------------------------------
 *     U+0000+0xxx    564
 *     U+0000+1xxx      6
 *     U+0000+2xxx    321
 *     U+0000+3xxx    422
 *     U+0000+4xxx    347
 *     U+0000+5xxx   1951
 *     U+0000+6xxx   2047
 *     U+0000+7xxx   1868
 *     U+0000+8xxx   1769
 *     U+0000+9xxx   1583
 *     U+0000+fxxx    241
 *     U+0002+xxxx    302
 *
 * It is so wide and so sparse that naive lookup table implementation from
 * UCS to EUC can be space-wasting.  I use hierarchical table with some
 * ad-hoc heuristics.   Since the hierarchical table is used, I directly
 * translates UTF8 to EUC_JP, without converting it to UCS4.
 *
 * Strategy outline: say input consists of bytes named u0, u1, ....
 *
 *  u0 <= 0x7f  : ASCII range
 *  u0 in [0xc2-0xd1] : UTF8 uses 2 bytes.  Some mappings within this range
 *         is either very regular or very small, and they are
 *         hardcoded.   Other mappings uses table lookup.
 *  u0 == 0xe1  : UTF8 uses 3 bytes.  There are only 6 characters in this
 *         range, and it is hardcoded.
 *  u0 in [0xe2-0xe9, 0xef] : Large number of characters are in this range.
 *         Two-level table of 64 entries each is used to dispatch the
 *         characters.
 *  u0 == 0xf0  : UTF8 uses 4 bytes.  u1 is in [0xa0-0xaa].  u2 and u3 is
 *         used for dispatch table of 64 entries each.
 *
 * The final table entry is unsigned short.  0x0000 means no corresponding
 * character is defined in EUC_JP.  >=0x8000 is the EUC_JP character itself.
 * < 0x8000 means the character is in G3 plane; 0x8f should be preceded,
 * and 0x8000 must be added to the value.
 */

void utf8_to_eucj::emit(unsigned short euc) {
    if(euc == 0) {
	subst_();
	return;
    }

    if(euc < 0x8000) {
	output_ += 0x8f;
	output_ += ((euc >> 8) + 0x80);
	output_ += (euc & 0xff);
	return;
    }

    output_ += (euc >> 8);
    output_ += (euc & 0xff);
}

void utf8_to_eucj::neutral(unsigned char c) {
    if(c <= 0x7f) {
	output_ += c;
	return;
    }

    if(c <= 0xbf) {
	return dead_end("utf8_to_eucj::neutral(): illegal UTF8 sequence");
    }

    do {
	if(c <= 0xdf) {
	    state_ = &utf8_to_eucj::sequence_of_2_bytes;
	    break;
	}

	if(c <= 0xef) {
	    state_ = &utf8_to_eucj::sequence_of_3_bytes;
	    break;
	}

	if(c <= 0xf7) {
	    state_ = &utf8_to_eucj::sequence_of_4_bytes;
	    break;
	}

	if(c <= 0xfb) {
	    state_ = &utf8_to_eucj::sequence_of_5_bytes;
	    break;
	}

	if(c <= 0xfd) {
	    state_ = &utf8_to_eucj::sequence_of_6_bytes;
	    break;
	}

	return dead_end("utf8_to_eucj::neutral(): illegal UTF8 sequence");
    } while(0);

    // save first byte
    input_.clear();
    input_.push_back(c);
}

void utf8_to_eucj::sequence_of_2_bytes(unsigned char c) {
    input_.push_back(c);

    reset();

    if(input_[1] < 0x80 || 0xc0 <= input_[1]) {
	return dead_end("utf8_to_eucj::sequence_of_2_bytes(): illegal UTF8 sequence");
    }

    unsigned short* etab = NULL;

    switch(input_[0]) {
    case 0xc2: etab = utf2euc_c2; break;
    case 0xc3: etab = utf2euc_c3; break;
    case 0xc4: etab = utf2euc_c4; break;
    case 0xc5: etab = utf2euc_c5; break;
    case 0xc6:
	if(input_[1] == 0x93) {				// U+0193 -> euc ABA9
	    emit(0xaba9);
	    return;
	} else break;
    case 0xc7: etab = utf2euc_c7; break;
    case 0xc9: etab = utf2euc_c9; break;
    case 0xca: etab = utf2euc_ca; break;
    case 0xcb: etab = utf2euc_cb; break;
    case 0xcc: etab = utf2euc_cc; break;
    case 0xcd:
	if(input_[1] == 0xa1) {				// U+0361 -> euc ABD2
	    emit(0xabd2);
	    return;
	} else break;
    case 0xce: etab = utf2euc_ce; break;
    case 0xcf: etab = utf2euc_cf; break;
    case 0xd0: etab = utf2euc_d0; break;
    case 0xd1: etab = utf2euc_d1; break;
    }

    if(etab != NULL) {
	emit(etab[input_[1] - 0x80]);		// table lookup
    } else {
	subst_();
    }
}

void utf8_to_eucj::sequence_of_3_bytes(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 3) {
	return;
    }

    reset();

    unsigned char* tab1 = NULL;
    unsigned short (*tab2)[64] = NULL;

    switch(input_[0]) {
    case 0xe1:					// special case : there's only 6 chars
    {
	unsigned short euc = 0;
	switch(input_[1]) {
	case 0xb8:
	    switch(input_[2]) {
	    case 0xbe: euc = 0xa8f2; break;
	    case 0xbf: euc = 0xa8f3; break;
	    }
	    break;
	case 0xbd:
	    switch(input_[2]) {
	    case 0xb0: euc = 0xabc6; break;
	    case 0xb1: euc = 0xabc7; break;
	    case 0xb2: euc = 0xabd0; break;
	    case 0xb3: euc = 0xabd1; break;
	    }
	    break;
	}
	emit(euc);
	return;
    }
    case 0xe2: tab1 = utf2euc_e2; tab2 = utf2euc_e2_xx; break;
    case 0xe3: tab1 = utf2euc_e3; tab2 = utf2euc_e3_xx; break;
    case 0xe4: tab1 = utf2euc_e4; tab2 = utf2euc_e4_xx; break;
    case 0xe5: tab1 = utf2euc_e5; tab2 = utf2euc_e5_xx; break;
    case 0xe6: tab1 = utf2euc_e6; tab2 = utf2euc_e6_xx; break;
    case 0xe7: tab1 = utf2euc_e7; tab2 = utf2euc_e7_xx; break;
    case 0xe8: tab1 = utf2euc_e8; tab2 = utf2euc_e8_xx; break;
    case 0xe9: tab1 = utf2euc_e9; tab2 = utf2euc_e9_xx; break;
    case 0xef: tab1 = utf2euc_ef; tab2 = utf2euc_ef_xx; break;
    }

    if(tab1 != NULL) {
	unsigned char ind = tab1[input_[1] - 0x80];
	if(ind != 0) {
	    emit(tab2[ind - 1][input_[2] - 0x80]);
	    return;
	}
    }

    subst_();
}

void utf8_to_eucj::sequence_of_4_bytes(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 4) {
	return;
    }

    reset();

    unsigned short* tab = NULL;

    if(input_[0] != 0xf0) {
	subst_();
	return;
    }

    switch(input_[1]) {
    case 0xa0: tab = utf2euc_f0_a0; break;
    case 0xa1: tab = utf2euc_f0_a1; break;
    case 0xa2: tab = utf2euc_f0_a2; break;
    case 0xa3: tab = utf2euc_f0_a3; break;
    case 0xa4: tab = utf2euc_f0_a4; break;
    case 0xa5: tab = utf2euc_f0_a5; break;
    case 0xa6: tab = utf2euc_f0_a6; break;
    case 0xa7: tab = utf2euc_f0_a7; break;
    case 0xa8: tab = utf2euc_f0_a8; break;
    case 0xa9: tab = utf2euc_f0_a9; break;
    case 0xaa: tab = utf2euc_f0_aa; break;
    }

    if(tab != NULL) {
	unsigned short entry = input_[2] * 256 + input_[3];
	for(int i = 0; tab[i]; i += 2) {
	    if(tab[i] == entry) {
		emit(tab[i + 1]);
		return;
	    }
	}
    }

    subst_();
}

void utf8_to_eucj::sequence_of_5_bytes(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 5) {
	return;
    }

    reset();
    subst_();
}

void utf8_to_eucj::sequence_of_6_bytes(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 6) {
	return;
    }

    reset();
    subst_();
}

// ======================================================================
// EUC_JP -> UTF8
// ======================================================================

/* [EUC_JP -> UTF8 conversion]
 *
 * Conversion strategy:
 *   If euc0 is in ASCII range, or C1 range except 0x8e or 0x8f, map it as is.
 *   If euc0 is 0x8e, use JISX0201-KANA table.
 *   If euc0 is 0x8f, use JISX0213 plane 2 table.
 *   If euc0 is in [0xa1-0xfe], use JISX0213 plane1 table.
 *   If euc0 is 0xa0 or 0xff, return ILLEGAL_SEQUENCE.
 *
 * JISX0213 plane2 table is consisted by a 2-level tree.  The first-level
 * returns an index to the second-level table by (euc1 - 0xa1).  Only the
 * range of JISX0213 defined region is converted; JISX0212 region will be
 * mapped to the substitution char.
 */

int examine_utf8_nbytes(unsigned int ucs) {
    if(ucs <      0x80) return 1;
    if(ucs <     0x800) return 2;
    if(ucs <   0x10000) return 3;
    if(ucs <  0x200000) return 4;
    if(ucs < 0x4000000) return 5;

    return 6;
}

void ucs4_to_utf8(unsigned int ucs, std::string& dest) {
    switch(examine_utf8_nbytes(ucs)) {
    case 1:
	dest += ucs;
	break;
    case 2:
	dest += (((ucs >> 6)	& 0x1f) | 0xc0);
	dest += (((ucs)			& 0x3f) | 0x80);
	break;
    case 3:
	dest += (((ucs >> 12)	& 0x0f) | 0xe0);
	dest += (((ucs >> 6)	& 0x3f) | 0x80);
	dest += (((ucs)			& 0x3f) | 0x80);
	break;
    case 4:
	dest += (((ucs >> 18)	& 0x07) | 0xf0);
	dest += (((ucs >> 12)	& 0x3f) | 0x80);
	dest += (((ucs >> 6)	& 0x3f) | 0x80);
	dest += (((ucs)			& 0x3f) | 0x80);
	break;
    case 5:
	dest += (((ucs >> 24)	& 0x03) | 0xf8);
	dest += (((ucs >> 18)	& 0x3f) | 0x80);
	dest += (((ucs >> 12)	& 0x3f) | 0x80);
	dest += (((ucs >> 6)	& 0x3f) | 0x80);
	dest += (((ucs)			& 0x3f) | 0x80);
	break;
    case 6:
	dest += (((ucs >> 30)	& 0x01) | 0xfc);
	dest += (((ucs >> 24)	& 0x3f) | 0x80);
	dest += (((ucs >> 18)	& 0x3f) | 0x80);
	dest += (((ucs >> 12)	& 0x3f) | 0x80);
	dest += (((ucs >> 6)	& 0x3f) | 0x80);
	dest += (((ucs)			& 0x3f) | 0x80);
	break;
    }
}

/* Given 'encoded' ucs, emit utf8.  'Encoded' ucs is the entry of the
   conversion table.  If ucs >= 0x100000, it is composed by two UCS2
   character.  Otherwise, it is one UCS4 character. */

void eucj_to_utf8::emit(unsigned int ucs) {
    if(ucs == 0) {
	subst_();
	return;
    }

    if(ucs < 0x100000) {
	ucs4_to_utf8(ucs, output_);
	return;
    }

    // we need two UCS characters
    ucs4_to_utf8((ucs >> 16) & 0xffff, output_);
    ucs4_to_utf8(ucs & 0xffff, output_);
}

void eucj_to_utf8::neutral(unsigned char c) {
    if(c == 0x8e) {
	state_ = &eucj_to_utf8::jisx0201_kana;
	return;
    }

    if(c == 0x8f) {
	state_ = &eucj_to_utf8::jisx0213_plane2;
	input_.clear();
	return;
    }

    // ASCII or C1 region
    if(c < 0xa0) {
	output_ += c;
	return;
    }

    if(0xa0 < c && c < 0xff) {
	state_ = &eucj_to_utf8::jisx0213_plane1;
	input_.clear();
	input_.push_back(c);
	return;
    }

    dead_end("eucj_to_utf8::neutral(): illegal EUC_JP sequence");
}

void eucj_to_utf8::jisx0201_kana(unsigned char c) {
    reset();

    // illegal sequence
    if(c < 0xa1 || 0xdf < c) {
	return dead_end("eucj_to_utf8::jisx0201_kana(): illegal EUC_JP sequence");
    }

    emit(0xff61 + (c - 0xa1));
}

void eucj_to_utf8::jisx0213_plane1(unsigned char c) {
    input_.push_back(c);

    reset();

    // illegal sequence
    if(input_[1] < 0xa1 || 0xfe < input_[1]) {
	return dead_end("eucj_to_utf8::jisx0213_plane1(): illegal EUC_JP sequence");
    }

    unsigned int ucs = euc_jisx0213_1_to_ucs2[input_[0] - 0xa1][input_[1] - 0xa1];
    emit(ucs);
}

void eucj_to_utf8::jisx0213_plane2(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 2) {
	return;
    }

    reset();

    // illegal sequence
    if(input_[0] < 0xa1 || 0xfe < input_[0] ||
       input_[1] < 0xa1 || 0xfe < input_[1]) {
	return dead_end("eucj_to_utf8::jisx0213_plane2(): illegal EUC_JP sequence");
    }

    int index = euc_jisx0213_2_index[input_[0] - 0xa1];
    if(index < 0) {
	subst_();
	return;
    }

    unsigned int ucs = euc_jisx0213_2_to_ucs2[index][input_[1] - 0xa1];
    emit(ucs);
}

// ======================================================================
// ISO-2022-JP -> EUC_JP
// ======================================================================

/*=================================================================
 * ISO2022-JP
 */

/* ISO2022-JP{-1(,2),3} -> EUC_JP
 * Strategy: accepts as many possibilities as possible.
 * The following escape sequence is recognized:
 * (See Lunde, CJKV information processing, O'Reilly, pp.155--158)
 *
 *  <ESC> ( B     ASCII
 *  <ESC> ( J     JIS-Roman
 *  <ESC> ( H     JIS-Roman (for compatibility)
 *  <ESC> ( I     Half-width katakana (JIS X 0201 kana)
 *  <ESC> $ @     JIS C 6226-1978 (78JIS)
 *  <ESC> $ B     JIS X 0208-1983 (83JIS)
 *  <ESC> $ ( D   JIS X 0212-1990
 *  <ESC> $ ( O   JIS X 0213:2000 plane 1
 *  <ESC> $ ( P   JIS X 0213:2000 plane 2
 *  <ESC> & @ <ESC> $ B   JIS X 0208-1990, JIS X 0208:1997
 *  0x0e          JIS7 half-width katakana shift-out
 *  0x0f          JIS7 half-width katakana shift-in
 *
 * The following escape sequences defined in ISO2022-JP-2 are recognized,
 * but all the characters within the sequence will be replaced by '?'.
 *
 *  <ESC> $ A     (GB2312-80) unsupported
 *  <ESC> $ ( C   (KS X 1001:1992) unsupported
 *  <ESC> . A     (ISO8859-1:1998) unsupported
 *  <ESC> . F     (ISO8859-7:1998) unsupported
 * 
 * JIS8 kana is allowed.
 */

void iso2022jp_to_eucj::neutral(unsigned char c) {
    if(c == 0x1b) {
	state_ = &iso2022jp_to_eucj::escape_sequence;
	input_.clear();
	return;
    }

    if(c < 0x20 || c == '\n' || c == '\r') {
	output_ += c;
	return;
    }

    // JIS8 kana
    if(0xa1 <= c && c <= 0xdf) {
	output_ += 0x8e;
	output_ += c;
	return;
    }

    // invoke sub state
    (this->*sub_state_)(c);
}

void iso2022jp_to_eucj::escape_sequence(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 2) {
	return;
    }

    switch(input_[0]) {
    case '(':
	switch(input_[1]) {
	case 'B': sub_state_ = &iso2022jp_to_eucj::us_ascii; break;
	case 'J': sub_state_ = &iso2022jp_to_eucj::jisx0201_roman; break;
	case 'H': sub_state_ = &iso2022jp_to_eucj::jisx0201_roman; break;
	case 'I': sub_state_ = &iso2022jp_to_eucj::jisx0201_kana; break;
	default:							// illegal
	    return dead_end("iso2022jp_to_eucj::escape(): invalid escape sequence");
	}
	break;
    case '$':
	switch(input_[1]) {
	case '@': sub_state_ = &iso2022jp_to_eucj::jisx0208_1978; break;
	case 'B': sub_state_ = &iso2022jp_to_eucj::jisx0213_plane1; break;
	case 'A': sub_state_ = &iso2022jp_to_eucj::unknown; break;
	case '(':
	    if(input_.size() < 3) return;

	    switch(input_[2]) {
	    case 'D': sub_state_ = &iso2022jp_to_eucj::jisx0212; break;
	    case 'O': sub_state_ = &iso2022jp_to_eucj::jisx0213_plane1; break;
	    case 'P': sub_state_ = &iso2022jp_to_eucj::jisx0213_plane2; break;
	    case 'C': sub_state_ = &iso2022jp_to_eucj::unknown; break;
	    default:						// illegal
		return dead_end("iso2022jp_to_eucj::escape(): invalid escape sequence");
	    }
	}
	break;
    case '&':
	if(input_.size() < 5) return;

	if(input_[1] == '@' && input_[2] == 0x1b && input_[3] == '$' && input_[4] == 'B') {
	    sub_state_ = &iso2022jp_to_eucj::jisx0213_plane1;
	} else {
	    return dead_end("iso2022jp_to_eucj::escape(): invalid escape sequence");
	}
	break;
    case '.':
	switch(input_[1]) {
	case 'A':							// fallthrough
	case 'F': sub_state_ = &iso2022jp_to_eucj::unknown; break;
	default:							// illegal
	    return dead_end("iso2022jp_to_eucj::escape(): invalid escape sequence");
	}
	break;
    default:								// illegal
	return dead_end("iso2022jp_to_eucj::escape(): invalid escape sequence");
    }

    input_.clear();

    reset();
}

void iso2022jp_to_eucj::us_ascii(unsigned char c) {
    output_ += c;
}

void iso2022jp_to_eucj::jisx0201_roman(unsigned char c) {
    // jis-roman and ascii differs on 0x5c and 0x7e -- for now, I
    // ignore the difference.
    us_ascii(c);
}

void iso2022jp_to_eucj::jisx0201_kana(unsigned char c) {
    output_ += 0x8e;
    output_ += (c + 0x80);
}

void iso2022jp_to_eucj::jisx0208_1978(unsigned char c) {
    // for now, I ignore the difference between JIS78 and JIS83
    jisx0213_plane1(c);
}

void iso2022jp_to_eucj::jisx0212(unsigned char c) {
    // jis x 0212 and jis x 0213 plane 2 are different character sets,
    // but uses the same conversion scheme.
    jisx0213_plane2(c);
}

void iso2022jp_to_eucj::jisx0213_plane1(unsigned char c) {
    input_.push_back(c);

    if(input_.size() == 2) {
	output_ += (input_[0] + 0x80);
	output_ += (input_[1] + 0x80);
	input_.clear();
    }
}

void iso2022jp_to_eucj::jisx0213_plane2(unsigned char c) {
    input_.push_back(c);

    if(input_.size() == 2) {
	output_ += 0x8f;
	output_ += (input_[0] + 0x80);
	output_ += (input_[1] + 0x80);
	input_.clear();
    }
}

void iso2022jp_to_eucj::unknown(unsigned char /* c */) {
    output_ += subst_char;
}

// ======================================================================
// EUC_JP -> ISO-2022-JP
// ======================================================================

/* EUC_JP -> ISO2022JP(-3)
 *
 * For now, I follow the strategy of iso2022jp-3-compatible behavior.
 */

void eucj_to_iso2022jp::neutral(unsigned char c) {
    if(c < 0x80) {
	us_ascii(c);
	return;
    }

    if(c == 0x8e) {
	jisx0201_kana(c);
	return;
    }

    if(c == 0x8f) {
	state_ = &eucj_to_iso2022jp::guess;
	return;
    }

    if(0xa0 < c && c < 0xff) {
	jisx0213_plane1(c);
	return;
    }

    dead_end("eucj_to_iso2022jp::neutral(): invalid sequence");
}

void eucj_to_iso2022jp::guess(unsigned char c) {
    input_.push_back(c);
    if(input_.size() < 2) {
	return;
    }

    if(0xa0 < input_[0] && input_[0] < 0xff &&
       0xa0 < input_[1] && input_[1] < 0xff) {
	handler tmp_state = &eucj_to_iso2022jp::jisx0212;
	switch(input_[0]) {
	case 0xa1:
	case 0xa3:
	case 0xa4:
	case 0xa5:
	case 0xa8:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xaf:
	    tmp_state = &eucj_to_iso2022jp::jisx0213_plane2;
	    break;
	}
	if(0xee <= input_[0]) {
	    tmp_state = &eucj_to_iso2022jp::jisx0213_plane2;
	}

	input_.pop_back();				// oops!

	(this->*tmp_state)(c);
    }

    reset();
}

void eucj_to_iso2022jp::us_ascii(unsigned char c) {
    if(sub_state_ != &eucj_to_iso2022jp::us_ascii) {
	sub_state_ = &eucj_to_iso2022jp::us_ascii;
	output_ += "\033(B";
    }

    output_ += c;
}

void eucj_to_iso2022jp::jisx0201_kana(unsigned char c) {
    if(sub_state_ != &eucj_to_iso2022jp::jisx0201_kana) {
	sub_state_ = &eucj_to_iso2022jp::jisx0201_kana;
	output_ += "\033(I";
    }

    input_.push_back(c);
    if(input_.size() == 2) {
	output_ += input_[1];
	input_.clear();
    }
}

void eucj_to_iso2022jp::jisx0212(unsigned char c) {
    if(sub_state_ != &eucj_to_iso2022jp::jisx0212) {
	sub_state_ = &eucj_to_iso2022jp::jisx0212;
	output_ += "\033$(D";
    }

    input_.push_back(c);
    if(input_.size() == 2) {
	output_ += (input_[0] - 0x80);
	output_ += (input_[1] - 0x80);
	input_.clear();
    }
}

void eucj_to_iso2022jp::jisx0213_plane1(unsigned char c) {
    if(sub_state_ != &eucj_to_iso2022jp::jisx0213_plane1) {
	sub_state_ = &eucj_to_iso2022jp::jisx0213_plane1;
	output_ += "\033$B";
    }

    input_.push_back(c);
    if(input_.size() == 2) {
	output_ += (input_[0] - 0x80);
	output_ += (input_[1] - 0x80);
	input_.clear();
    }
}

void eucj_to_iso2022jp::jisx0213_plane2(unsigned char c) {
    if(sub_state_ != &eucj_to_iso2022jp::jisx0213_plane2) {
	sub_state_ = &eucj_to_iso2022jp::jisx0213_plane2;
	output_ += "\033$(P";
    }

    input_.push_back(c);
    if(input_.size() == 2) {
	output_ += (input_[0] - 0x80);
	output_ += (input_[1] - 0x80);
	input_.clear();
    }
}

int dummy() {
    return euc_jisx0201_to_ucs2[0];
}
