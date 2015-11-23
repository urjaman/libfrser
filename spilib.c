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
#include "spilib.h"
#include "uart.h"

/* This is just hardware-independent generic code to help with spi. */

static void spi_localop_start(uint8_t sbytes, const uint8_t* sarr) {
	uint8_t i;
	spi_select();
	for(i=0;i<sbytes;i++) spi_txrx(sarr[i]);
}

static void spi_localop_end(uint8_t rbytes, uint8_t* rarr) {
	uint8_t i;
	for(i=0;i<rbytes;i++) rarr[i] = spi_txrx(0xFF);
	spi_deselect();
}

static void spi_localop(uint8_t sbytes, const uint8_t* sarr, uint8_t rbytes, uint8_t* rarr) {
	spi_localop_start(sbytes,sarr);
	spi_localop_end(rbytes,rarr);
}

#ifndef FRSER_ASYNC_SPI_API
/* Code for simple spi_txrx-only API. */
static void spi_spiop_start(uint32_t sbytes) {
	spi_select();
	while (sbytes--) spi_txrx(RECEIVE());
}

static void spi_spiop_end(uint32_t rbytes) {
	while (rbytes--) SEND(spi_txrx(0xFF));
	spi_deselect();
}
#else
static void spi_spiop_start(uint32_t sbytes) {
	spi_select();
	if (sbytes) {
		while (sbytes--) {
			spi_awrite(RECEIVE());
		}
		spi_aread(); // read implicitly waits for all writes to be done, in order to return data from last write...
	}
}

static void spi_spiop_end(uint32_t rbytes) {
	if (rbytes) {
		spi_awrite_fast(0xFF);
		rbytes--;
		while (rbytes--) {
			uint8_t d = spi_aread();
			spi_awrite_fast(0xFF);
			SEND(d);
		}
		SEND(spi_aread());
	}
	spi_deselect();
}
#endif

void spi_spiop(uint32_t sbytes, uint32_t rbytes) {
	spi_spiop_start(sbytes);
	SEND(S_ACK);
	spi_spiop_end(rbytes);
}

uint8_t oddparity(uint8_t val) {
	val = (val ^ (val >> 4)) & 0xf;
	val = (val ^ (val >> 2)) & 0x3;
	return (val ^ (val >> 1)) & 0x1;
}

uint8_t spi_probe_rdid(uint8_t *id) {
	const uint8_t out[1] = { 0x9F };
	uint8_t in[3];
	spi_localop(1,out,3,in);
	if (!oddparity(in[0])) return 0;
	if ((in[0] == 0xFF)&&(in[1] == 0xFF)&&(in[2] == 0xFF)) return 0;
	if ((in[0] == 0)&&(in[1] == 0)&&(in[2] == 0)) return 0;
	if (id) memcpy(id,in,3);
	return 1;
}

uint8_t spi_probe_rems(uint8_t *id) {
	const uint8_t out[4] = { 0x90, 0, 0, 0 };
	uint8_t in[2];
	spi_localop(4,out,2,in);
	if ((in[0] == 0xFF)&&(in[1] == 0xFF)) return 0;
	if ((in[0] == 0)&&(in[1] == 0)) return 0;
	if (id) memcpy(id,in,2);
	return 1;
}

uint8_t spi_probe_res(uint8_t *id) {
	const uint8_t out[4] = { 0xAB, 0, 0, 0 };
	uint8_t in[1];
	spi_localop(4,out,1,in);
	if (in[0] == 0xFF) return 0;
	if (in[0] == 0) return 0;
	if (id) *id = in[0];
	return 1;
}

uint8_t spi_test(void) {
	spi_init();
	if (spi_probe_rdid(NULL)) return 1;
	if (spi_probe_rems(NULL)) return 1;
	if (spi_probe_res(NULL)) return 1;
	spi_uninit();
	return 0;
}

uint8_t spi_read(uint32_t addr) {
	uint8_t sarr[4];
	uint8_t rarrv;
	sarr[0] = 0x03; /* Read */
	sarr[1] = (addr>>16)&0xFF;
	sarr[2] = (addr>> 8)&0xFF;
	sarr[3] = (addr    )&0xFF;
	spi_localop(4,sarr,1,&rarrv);
	return rarrv;
}

void spi_readn(uint32_t addr, uint32_t len) {
	uint8_t sarr[4];
	sarr[0] = 0x03;
	sarr[1] = (addr>>16)&0xFF;
	sarr[2] = (addr>> 8)&0xFF;
	sarr[3] = (addr    )&0xFF;
	spi_localop_start(4,sarr);
	spi_spiop_end(len);
}
