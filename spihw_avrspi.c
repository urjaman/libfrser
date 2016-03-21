/*
 * This file is part of the libfrser project.
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
#include "spihw_avrspi.h"
#include "frser-cfg.h"

/* This is for the usage of the SPI module of the ATmega88/168/328, maybe others. */
/* Doesnt deal with CS. */

static uint8_t spi_initialized = 0;

#ifdef FRSER_FEAT_SPISPEED
static uint8_t spi_set_spd = 0; /* Max speed - F_CPU/2 */

const uint8_t PROGMEM spd_table[7] = {
	0x80, // SPI2X is 0x80 in this table because i say so. - div 2
	0, // div 4
	0x80 | _BV(SPR0), // div 8
	_BV(SPR0), // div 16
	0x80 | _BV(SPR1), // div 32
	_BV(SPR1), // div64
	_BV(SPR1) | _BV(SPR0), //div128
};

const uint32_t PROGMEM spd_hz_table[7] = {
	F_CPU/2, F_CPU/4, F_CPU/8, F_CPU/16, F_CPU/32, F_CPU/64, F_CPU/128
};

uint32_t spi_set_speed(uint32_t hz) {
	/* We can set F_CPU / : 2,4,8,16,32,64,128 */

	uint8_t spd;
	uint32_t hz_spd;

	/* Range check. */
	if (hz<=(F_CPU/128)) {
		spd = 6;
		hz_spd = F_CPU/128;
	} else {
		for (spd=0;spd<7;spd++) {
			hz_spd = pgm_read_dword(&(spd_hz_table[spd]));
			if (hz >= hz_spd) break;
		}
	}
	spi_set_spd = spd;
	if (spi_initialized) { // change speed
		spi_init(); // re-init
	}
	return hz_spd;
}


void spi_init(void) {
	/* DDR and PORT settings come from elsewhere (maybe flash.c) */
	uint8_t spdv = pgm_read_byte(&(spd_table[spi_set_spd]));
	SPCR = _BV(SPE) | _BV(MSTR) | (spdv & 0x03);
	if (spdv&0x80) SPSR |= _BV(SPI2X);
	else SPSR &= ~_BV(SPI2X);
	spi_initialized = 1;
}
#else
void spi_init(void) {
	/* No speed setting code wanted? Max speed. */
	SPCR = _BV(SPE) | _BV(MSTR);
	SPSR |= _BV(SPI2X);
	spi_initialized = 1;
}
#endif

uint8_t spi_uninit(void) {
	if (spi_initialized) {
		SPCR = 0;
		spi_initialized = 0;
		return 1;
	}
	return 0;
}

void spi_awrite_fast(uint8_t d) {
	SPDR = d;
}

void spi_awrite(uint8_t d) {
	uint8_t dummy = SPSR; /* Flush previous SPIF */
	SPDR = d;
	if (SPSR & _BV(WCOL)) {
		loop_until_bit_is_set(SPSR,SPIF);
		SPDR = d;
	}
	(void)dummy; /* Silence compiler */
}

uint8_t spi_aread(void) {
	loop_until_bit_is_set(SPSR,SPIF);
	return SPDR;
}

uint8_t spi_txrx(const uint8_t c) {
	spi_awrite_fast(c);
	return spi_aread();
}



/* This is here only to keep spi_initialized a static variable,
 * hidden from other bits of code. */

void spi_init_cond(void) {
	if (!spi_initialized) spi_init();
}

/* Select and deselect are not part of the "domain" of this .c / target-specific */
