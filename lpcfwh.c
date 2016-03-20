/*
	This file was part of bbflash, now frser-m328lpcspi.
	Copyright (C) 2013, Hao Liu and Robert L. Thompson
	Copyright (C) 2013,2015 Urja Rannikko <urjaman@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "main.h"
#include "nibble.h"
#include "lpcfwh.h"
#include "typeu.h"
#include "uart.h"

/* Generic */

static uint8_t nib_byte_read(void) {
	return clocked_nibble_read()
	| (clocked_nibble_read() << 4);
}

static void nib_byte_write(uint8_t byte) {
	clocked_nibble_write(byte);
	clocked_nibble_write_hi(byte);
}

static bool nibble_ready_sync(void) {
	uint8_t nib;
	uint8_t x=32;
	do {
		nib = clocked_nibble_read();
		if (!(x--)) {
			/* Do a proper-ish abort, just to be nice. */
			nibble_abort();
			return false;
		}
	} while (nib != 0);
	return true;
}

static void nibble_send_addr_24b(uint32_t addr) {
	u32_u a;
	a.l = addr;
	clocked_nibble_write_hi(a.b[2]);
	clocked_nibble_write(a.b[2]);
	clocked_nibble_write_hi(a.b[1]);
	clocked_nibble_write(a.b[1]);
	clocked_nibble_write_hi(a.b[0]);
	clocked_nibble_write(a.b[0]);
}

/* LPC start */

#define LPC_START 0b0000
#define LPC_CYCTYPE_READ 0b0100
#define LPC_CYCTYPE_WRITE 0b0110

bool lpc_init(void) {
	return nibble_init();
}

void lpc_cleanup(void) {
	nibble_cleanup();
}


static void lpc_start(void) {
	nibble_start(LPC_START);
}

#define lpc_nibble_write(v) clocked_nibble_write(v)


static void lpc_send_addr(uint32_t addr) {
	/* NOTE: Hard-coded uppest 8b. */
	lpc_nibble_write(0xF);
	clock_cycle();
	nibble_send_addr_24b(addr);
}

int lpc_read_address(uint32_t addr) {
	lpc_start();
	lpc_nibble_write(LPC_CYCTYPE_READ);
	lpc_send_addr(addr);
	nibble_set_dir(INPUT);
	clock_cycle();
	if (!nibble_ready_sync())
		return -1;
	uint8_t byte = nib_byte_read();
	clock_cycle();
	clock_cycle();
	clock_cycle();
	return byte;
}

bool lpc_write_address(uint32_t addr, uint8_t byte) {
	lpc_start();
	lpc_nibble_write(LPC_CYCTYPE_WRITE);
	lpc_send_addr(addr);
	nib_byte_write(byte);
	nibble_set_dir(INPUT);
	clock_cycle();
	clock_cycle();
	if (!nibble_ready_sync())
		return false;
	clock_cycle();
	return true;
}



uint8_t lpc_test(void) {
	nibble_hw_init();
	lpc_init();
	if (lpc_read_address(0xFFFFFFFF)==-1) return 0;
	return 1;
}

/* LPC end, FWH start */

#define FWH_START_READ 0b1101
#define FWH_START_WRITE 0b1110
#define FWH_ABORT 0b1111

static uint8_t fwh_read_128 = 0;

bool fwh_init(void) {
	return nibble_init();
}

void fwh_cleanup(void) {
	nibble_cleanup();
}

#define fwh_nibble_write(v) clocked_nibble_write(v)
#define fwh_start(v) nibble_start(v)

static void fwh_send_imaddr(uint32_t addr) {
	/* NOTE: hard-coded uppest 4b. */
	fwh_nibble_write(0xF);
	nibble_send_addr_24b(addr);
}

int fwh_read_address(uint32_t addr) {
	fwh_start(FWH_START_READ);
	fwh_nibble_write(0);	/* IDSEL hardwired */
	fwh_send_imaddr(addr);
	fwh_nibble_write(0);	/* IMSIZE single byte */
	nibble_set_dir(INPUT);
	clock_cycle();
	if (!nibble_ready_sync())
		return -1;
	uint8_t byte = nib_byte_read();
	clock_cycle();
	nibble_set_dir(OUTPUT);
	fwh_nibble_write(0xf);
	clock_cycle();
	return byte;
}

static int fwh_test_128b(void) {
	fwh_start(FWH_START_READ);
	fwh_nibble_write(0);	/* IDSEL hardwired */
	fwh_send_imaddr(0xFFFFFF80);
	fwh_nibble_write(7);	/* IMSIZE 128b */
	nibble_set_dir(INPUT);
	clock_cycle();
	if (!nibble_ready_sync())
		return 0;
	for (int i=0;i<256;i++) clock_cycle();
	clock_cycle();
	nibble_set_dir(OUTPUT);
	fwh_nibble_write(0xf);
	clock_cycle();
	return 1;
}

void fwh_read_n(uint32_t addr, uint32_t len) {
	if (!fwh_read_128) {
		while (len--) SEND(fwh_read_address(addr++));
		return;
	}
	/* Align read ... */
	while (addr & 0x7F) {
		SEND(fwh_read_address(addr++));
		len--;
		if (!len) return;
	}
	while (len >= 128) {
		fwh_start(FWH_START_READ);
		fwh_nibble_write(0);	/* IDSEL hardwired */
		fwh_send_imaddr(addr);
		fwh_nibble_write(7);	/* IMSIZE 128b */
		nibble_set_dir(INPUT);
		clock_cycle();
		if (!nibble_ready_sync())
			break; // Try with the single-byte version then...
		for (int i=0;i<128;i++) SEND(nib_byte_read());
		clock_cycle();
		nibble_set_dir(OUTPUT);
		fwh_nibble_write(0xf);
		clock_cycle();
		len -= 128;
		addr += 128;
	}
	/* Do the remainder if any. */
	while (len--) SEND(fwh_read_address(addr++));

}

bool fwh_write_address(uint32_t addr, uint8_t byte) {
	fwh_start(FWH_START_WRITE);
	fwh_nibble_write(0);	/* IDSEL hardwired */
	fwh_send_imaddr(addr);
	fwh_nibble_write(0);	/* IMSIZE single byte */
	nib_byte_write(byte);
	nibble_write(0xf);
	nibble_set_dir(INPUT);
	clock_cycle();
	clock_cycle();
	if (!nibble_ready_sync())
		return false;
	clock_cycle();
	return true;
}

uint8_t fwh_test(void) {
	nibble_hw_init();
	fwh_init();
	if (fwh_read_address(0xFFFFFFFF)==-1) return 0;
	fwh_read_128 = fwh_test_128b();
	return 1 + fwh_read_128;
}
