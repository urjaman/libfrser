/*
 * This file is part of the libfrser project.
 *
 * Copyright (C) 2009,2015 Urja Rannikko <urjaman@gmail.com>
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

#ifndef _SPILIB_H_
#define _SPILIB_H_

/* These are the things in spilib.c (generic helpers) */
uint8_t spi_test(void);
uint8_t spi_read(uint32_t addr);
void spi_readn(uint32_t addr, uint32_t len);
void spi_spiop(uint32_t sbytes, uint32_t rbytes);
uint8_t spi_probe_rdid(uint8_t *id);
uint8_t spi_probe_rems(uint8_t *id);
uint8_t spi_probe_res(uint8_t *id);

/* This includes all the hardware-specific functions,
 * to be provided by target. */
#include "spihw.h"

/* The name is spilib.h instead of spi.h
 * in order not to clash with "host" spi.h if this
 * code is not used in the app. */

#endif
