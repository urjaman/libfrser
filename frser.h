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
#ifndef _FRSER_H_
#define _FRSER_H_

#include "frser-cfg.h"

/* Call this and frser owns your thingy. */
void frser_main(void) __attribute__((noreturn));

/* Alternatively, you can use this API: */
void frser_init(void);
void frser_operation(uint8_t op);

#ifdef FRSER_FEAT_LAST_OP
uint8_t get_last_op(void);
#endif

/* These allow you to override to a single bustype (eg. FWH-only). */
/* I suppose you could put one of them into frser-cfg.h too.. */
/* Do still also pick the FRSER_FEAT_* that includes your bustype. */

//#define FORCE_BUSTYPE CHIP_BUSTYPE_PARALLEL
//#define FORCE_BUSTYPE CHIP_BUSTYPE_LPC
//#define FORCE_BUSTYPE CHIP_BUSTYPE_FWH
//#define FORCE_BUSTYPE CHIP_BUSTYPE_SPI

#define CHIP_BUSTYPE_PARALLEL (1 << 0)
#define CHIP_BUSTYPE_LPC (1 << 1)
#define CHIP_BUSTYPE_FWH (1 << 2)
#define CHIP_BUSTYPE_SPI (1 << 3)

#ifdef FRSER_FEAT_PARALLEL
#define _FR_SUPP0 CHIP_BUSTYPE_PARALLEL
#else
#define _FR_SUPP0 0
#endif

#ifdef FRSER_FEAT_LPCFWH
#define _FR_SUPP1 CHIP_BUSTYPE_LPC | CHIP_BUSTYPE_FWH
#else
#define _FR_SUPP1 0
#endif

#ifdef FRSER_FEAT_SPI
#define _FR_SUPP2 CHIP_BUSTYPE_SPI
#else
#define _FR_SUPP2 0
#endif

#ifdef FORCE_BUSTYPE
#define SUPPORTED_BUSTYPES FORCE_BUSTYPE
#else
#define SUPPORTED_BUSTYPES (_FR_SUPP0 | _FR_SUPP1 | _FR_SUPP2)
#endif

#endif /* _FRSER_H_ */
