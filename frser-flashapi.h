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

/* These are the functions (mostly flashing related) that frser will call. */
/* Some of of them must be able to execute a part of the frser
   operation over UART (flash_readn and flash_spiop). */
/* flash_readn: send len bytes over uart. */
/* flash_spiop: receive sbytes, send S_ACK, send rbytes */

#ifndef _FRSER_FLASHAPI_H_
#define _FRSER_FLASHAPI_H_

/* The flashapi functions need to know S_ACK and the defines here come from there,
   thus give frser-int.h */
#include "frser-int.h"

#ifdef FRSER_FEAT_PIN_STATE
/* This will be used to disable pin drivers. */
void flash_set_safe(void);
#endif

/* This can be always called to re-init the interface, or to enable pin drivers. */
void flash_select_protocol(uint8_t allowed_protocols);

#ifdef FRSER_FEAT_NONSPI
uint8_t flash_read(uint32_t addr);
void flash_readn(uint32_t addr, uint32_t len);
void flash_write(uint32_t addr, uint8_t data);
#endif

#ifdef FRSER_FEAT_SPI
void flash_spiop(uint32_t sbytes, uint32_t rbytes);
#ifdef FRSER_FEAT_SPISPEED
uint32_t spi_set_speed(uint32_t hz);
#endif
#endif

#ifdef FRSER_FEAT_DYNPROTO
uint8_t flash_plausible_protocols(void);
#endif

#ifdef FRSER_FEAT_DBG_CONSOLE
/* The console must exit if it sees a S_CMD_NOP (0) */
void ciface_main(void);
#endif

#endif
