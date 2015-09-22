/*
 * This file is part of the frser-atmega644 project.
 *
 * Copyright (C) 2015 Urja Rannikko <urjaman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "main.h"
#include <stdarg.h>
#include "frser-cfg.h"
#include "dxprint.h"

#ifdef FRSER_FEAT_DPRINTF
#pragma GCC optimize ("Os")
#pragma GCC optimize ("no-tree-switch-conversion")


static uintptr_t xprint_buf[DXP_BUFSZ];
static uint8_t xprint_bwoff = 0;
static uint8_t xprint_broff = 0;
static uint8_t xprint_ovflw = 0;

static uint8_t xpbuf_free(void) {
	uint8_t used;
	if (xprint_bwoff >= xprint_broff) {
		used = xprint_bwoff - xprint_broff;
	} else {
		used = (xprint_bwoff + DXP_BUFSZ) - xprint_broff;
	}
	if (used >= (DXP_BUFSZ-1)) used = DXP_BUFSZ-1;
	return (DXP_BUFSZ-1) - used;
}

/* The caller checked that it fits / that there's data */
static void xpbuf_put(uintptr_t v) {
	xprint_buf[xprint_bwoff++] = v;
	if (xprint_bwoff >= DXP_BUFSZ) xprint_bwoff = 0;
}

static uintptr_t xpbuf_get(void) {
	uintptr_t r = xprint_buf[xprint_broff++];
	if (xprint_broff >= DXP_BUFSZ) xprint_broff = 0;
	return r;
}

static uint8_t ht_f(uint8_t offset) {
	offset |= 0x30;
	if (offset > 0x39) offset += 7;
	return offset;
}

static void u2x(uint8_t *buf,uint8_t val) {
	uint8_t offset;
	offset = ((val>>4)&0x0F);
	buf[0] = ht_f(offset);
	offset = (val&0x0F);
	buf[1] = ht_f(offset);
	buf[2] = 0;
}

static void w2s(uint8_t *buf, unsigned int val) {
	ultoa(val,(char*)buf,10);
}

static void w2x(uint8_t *buf,unsigned int val) {
	ultoa(val,(char*)buf,16);
	strupr((char*)buf); // i dont like "aaaah"...
}

enum partype {
	PT_PMS = 1, //"%s" with PGM_P
	PT_STR = 2, //"%s"
	PT_HB = 3, //"%02X"
	PT_HW = 4, //"%04X"
	PT_DW = 5, //"%u" (0 - 65535)
	PT_DH = 6, // luint2outdual with 16b param
	PT_CB = 7, // "%c"
};

void dxprint_put(PGM_P fmt, ...) {
	va_list ap;
	uint16_t l = 0;
	uint8_t free = xpbuf_free();
	va_start(ap,fmt);
	if (free < 2) {
		ovflw:
		if (xprint_ovflw < 255) xprint_ovflw++;
		end:
		va_end(ap);
		return;
	}
	/* string format specifiers allowed: */
	/* %s = ram string  STR*/
	/* %S = flash string  PMS */
	/* %02X = byte hex HB */
	/* %04X = word hex HW */
	/* %u = uint16_t decimal out  DW */
	/* %U = decimal and hex out word DH */
	/* %c = a character CB */
	while (1) {
		uint8_t c = pgm_read_byte(&(fmt[l++]));
		if ((c != '%')&&(c)) continue;
		if (l > 1) {
			if (free < 2) goto ovflw;
			xpbuf_put((PT_PMS << 12) | (l-1));
			xpbuf_put((uintptr_t)fmt);
			free -= 2;
			if (c == 0) break;
		}
		fmt = &(fmt[l]);
		l = 0;
		c = pgm_read_byte(&(fmt[l++]));
		switch (c) {
			default: goto end;
			case 's': {
			char * s = va_arg(ap,char*);
			size_t sl = strlen(s);
			if (sl>80) sl = 80;
			if (sl > 0) {
				if (free < (sl+1)) goto ovflw;
				xpbuf_put((PT_STR << 12) | sl);
				sl = (sl+1)/2;
				for (uint8_t n=0;n<sl;n++) {
					uint16_t w = (s[(n*2)+1] << 8) | s[n*2];
					xpbuf_put(w);
				}
				free -= sl+1;
			}
			}
			break;
			case 'S': {
			const PGM_P s = va_arg(ap, const PGM_P);
			size_t sl = strlen_P(s);
			if (sl > 128) sl = 128;
			if (sl > 0) {
				xpbuf_put((PT_PMS << 12) | sl);
				xpbuf_put((uintptr_t)s);
				free -= 2;
			}
			}
			break;
			case '0': {
				uint8_t c1 = pgm_read_byte(&(fmt[l++]));
				uint8_t c2 = pgm_read_byte(&(fmt[l++]));
				if ((c2 != 'x') && (c2 != 'X')) goto end;
				if ((c1 != '2') && (c1 != '4')) goto end;
				uint16_t w = va_arg(ap, uint16_t);
				if (free < 2) goto ovflw;
				if (c1 == '2') {
					xpbuf_put((PT_HB << 12) | (w&0xFF));
					free -= 1;
				} else {
					xpbuf_put((PT_HW << 12) | 4);
					xpbuf_put(w);
					free -= 2;
				}
			}
			break;
			case 'd':
			case 'u': {
				uint16_t w = va_arg(ap, uint16_t);
				if (free < 2) goto ovflw;
				xpbuf_put(PT_DW << 12);
				xpbuf_put(w);
				free -= 2;
			}
			break;
			case 'D':
			case 'U': {
				uint16_t w = va_arg(ap, uint16_t);
				if (free < 2) goto ovflw;
				xpbuf_put(PT_DH << 12);
				xpbuf_put(w);
				free -= 2;
			}
			break;
			case 'c': {
				uint8_t dt = va_arg(ap, int); /* char */
				if (free < 1) goto ovflw;
				xpbuf_put((PT_CB << 12) | dt);
				free -= 1;
			}
			break;
		}
		fmt = &(fmt[l]);
		l = 0;
	}
	va_end(ap);
	return;
}

void dxprint_tx(void (*txfp)(uint8_t))
{
	uint8_t tbuf[6];
	if ((xprint_ovflw) && (xpbuf_free() >= 6)) {
		/* Output a special message ... */
		if (xprint_ovflw < 255) {
			xpbuf_put((PT_PMS << 12) | 25);
			xpbuf_put((uintptr_t)PSTR("xprint() overflow! - lost "));
			xpbuf_put(PT_DW << 12);
			xpbuf_put(xprint_ovflw);
			xpbuf_put((PT_PMS << 12) | 11);
			xpbuf_put((uintptr_t)PSTR(" messages\r\n"));
		} else {
			xpbuf_put((PT_PMS << 12) | 42);
			xpbuf_put((uintptr_t)PSTR("xprint() overflow! - lost >=255 messages\r\n"));
		}
		xprint_ovflw = 0;
	}
	if (xprint_bwoff == xprint_broff) {
		txfp(0);
		return;
	}
	uint16_t param = xpbuf_get();
	enum partype t = (param >> 12) & 0xF;
	param &= 0xFFF;
	switch (t) {
		case PT_STR: {
			txfp(param);
			uint8_t sl = (param+1)/2;
			for (uint8_t n=0;n<sl;n++) {
				uintptr_t c1c2 = xpbuf_get();
				txfp(c1c2&0xFF);
				if (c1c2>>8) txfp(c1c2>>8);
			}
			}
			break;
		case PT_PMS: {
			txfp(param);
			PGM_P p = (PGM_P)xpbuf_get();
			for (uint8_t n=0;n<param;n++) {
				txfp(pgm_read_byte(p));
				p++;
			}
			}
			break;
		case PT_HB:
			u2x(tbuf,param);
			txfp(2);
			txfp(tbuf[0]);
			txfp(tbuf[1]);
			break;
		case PT_HW: {
			uint16_t v = xpbuf_get();
			txfp(4);
			u2x(tbuf,v>>8);
			txfp(tbuf[0]);
			txfp(tbuf[1]);
			u2x(tbuf,v&0xFF);
			txfp(tbuf[0]);
			txfp(tbuf[1]);
			}
			break;
		case PT_DW: {
			uintptr_t v = xpbuf_get();
			w2s(tbuf,v);
			uint8_t sl = strlen((char*)tbuf);
			txfp(sl);
			for (uint8_t i=0;i<sl;i++) txfp(tbuf[i]);
			}
			break;
		case PT_DH: {
			uintptr_t v = xpbuf_get();
			uint8_t l1;
			uint8_t l2;
			w2x(tbuf,v);
			l1 = strlen((char*)tbuf);
			w2s(tbuf,v);
			l2 = strlen((char*)tbuf);
			txfp(l1+l2+5);
			for (uint8_t i=0;i<l2;i++) txfp(tbuf[i]);
			txfp(' ');
			txfp('(');
			w2x(tbuf,v);
			for (uint8_t i=0;i<l1;i++) txfp(tbuf[i]);
			txfp('h');
			txfp(')');
			txfp(' ');
			}
			break;
		case PT_CB:
			txfp(1);
			txfp(param);
			break;
		default:
			txfp(0);
			break;
	}
	return;
}

#pragma GCC reset_options
#endif /* FRSER_FEAT_DPRINTF */
