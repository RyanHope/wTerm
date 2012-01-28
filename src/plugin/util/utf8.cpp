/**
 * This file is part of wTerm.
 * Copyright (C) 2012 Stefan Bühler <stbuehler@web.de>
 *
 * wTerm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * wTerm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with wTerm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utf8.hpp"

Uint16 parseUtf8Char(const char *str) {
	const unsigned char *s = (const unsigned char*) str;
	unsigned int code;
	int seqlen = 0; /* number of chars following the first */
	unsigned int i = 0;

	if (0 == (s[i] & 0x80)) { seqlen = 0; code = s[i] & 0x7f; }
	else if (0xC0 == (s[i] & 0xFE)) { return 0; /* 0xCO / 0xC1 overlong */ }
	else if (0xC0 == (s[i] & 0xE0)) { seqlen = 1; code = s[i] & 0x1f; }
	else if (0xE0 == (s[i] & 0xF0)) { seqlen = 2; code = s[i] & 0x0f; }
	/* only 16-bit, seqlen 3 not supported */
	/* else if (0xF0 == (s[i] & 0xF8)) { seqlen = 3; code = s[i] & 0x07; } */
	else return 0;

	for (int slen = seqlen; slen > 0; slen--) {
		unsigned char c = s[++i];
		if (!c) return 0; /* unexpected end of string */
		if (0x80 != (c & 0xC0)) return 0;
		code = (code << 6) | (c & 0x3f);
	}

	if ((seqlen == 2) && (code < 0x800)) return 0; /* overlong */

	/* don't care whether more data follows */

	/* seqlen <= 2 always fits into 16 bit */
	return (Uint16) code;
}

void writeUtf8Char(char *buf, Uint16 code) {
	unsigned char *b = (unsigned char*) buf;
	if (code < 0x0080) {
		b[0] = code;
		b[1] = '\0';
	} else if (code < 0x0800) {
		b[0] = 0xC0 | (code >> 6);
		b[1] = 0x80 | (code & 0x3F);
		b[2] = '\0';
	} else {
		b[0] = 0xE0 | (code >> 12);
		b[1] = 0x80 | ((code >> 6) & 0x3F);
		b[2] = 0x80 | (code & 0x3F);
		b[3] = '\0';
	}
}
