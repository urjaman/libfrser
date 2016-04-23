/* Flashrom serial interface AVR implementation */
#ifndef _FRSER_INT_H_
#define _FRSER_INT_H_

#define S_ACK 0x06
#define S_NAK 0x15

#define S_CMD_NOP		0x00            /* No operation                                 */
#define S_CMD_Q_IFACE           0x01            /* Query interface version                      */
#define S_CMD_Q_CMDMAP		0x02		/* Query supported commands bitmap		*/
#define S_CMD_Q_PGMNAME         0x03            /* Query programmer name                        */
#define S_CMD_Q_SERBUF          0x04            /* Query Serial Buffer Size                     */
#define S_CMD_Q_BUSTYPE         0x05            /* Query supported bustypes                     */
#define S_CMD_Q_CHIPSIZE        0x06            /* Query supported chipsize (2^n format)        */
#define S_CMD_Q_OPBUF           0x07            /* Query operation buffer size                  */

#define S_CMD_Q_WRNMAXLEN	0x08		/* Query Write to opbuf: Write-N maximum length */
#define S_CMD_R_BYTE            0x09            /* Read a single byte                           */
#define S_CMD_R_NBYTES          0x0A            /* Read n bytes                                 */
#define S_CMD_O_INIT            0x0B            /* Initialize operation buffer                  */
#define S_CMD_O_WRITEB          0x0C            /* Write opbuf: Write byte with address         */
#define S_CMD_O_WRITEN		0x0D		/* Write to opbuf: Write-N			*/
#define S_CMD_O_DELAY           0x0E            /* Write opbuf: udelay                          */
#define S_CMD_O_EXEC            0x0F            /* Execute operation buffer                     */

#define S_CMD_SYNCNOP		0x10		/* Special no-operation that returns NAK+ACK	*/
#define S_CMD_Q_RDNMAXLEN	0x11		/* Query read-n maximum length			*/
#define S_CMD_S_BUSTYPE		0x12		/* Set used bustype(s).				*/
#define S_CMD_O_SPIOP		0x13		/* Perform SPI operation.			*/
#define S_CMD_S_SPI_FREQ	0x14		/* Set SPI clock frequency			*/
#define S_CMD_S_PIN_STATE	0x15		/* Enable/disable output drivers		*/
// #define S_CMD_O_TOGGLERDY	0x16		/* Write to opbuf: wait JEDEC toggle ready	*/
#define S_CMD_O_POLL		0x17		/* Write to opbuf: poll (details in code/soon doc) */

#define S_CMD_O_POLL_DLY	0x18		/* Write to opbuf: poll (details in code/soon doc) */

/* The biggest valid command value */
#define S_MAXCMD 0x18
/* The maximum static length of parameters (poll_dly)) */
#define S_MAXLEN 0x08

/* "Library" Configuration Decisions - useful to have the command list above for reference with these */
#include "frser-cfg.h"

/* These are the commands that are always implemented */
#define _FR_BM_B0_BASE 0xBF
#define _FR_BM_B1_BASE 0xC9
#define _FR_BM_B2_BASE 0x03

#if (defined FRSER_FEAT_PARALLEL) || (defined FRSER_FEAT_LPCFWH)
#define FRSER_FEAT_NONSPI
#endif

#ifdef FRSER_FEAT_PARALLEL
#define _FR_BM_B0_ADD0 0x40
#else
#define _FR_BM_B0_ADD0 0
#endif

#ifdef FRSER_FEAT_NONSPI
#define _FR_BM_B1_ADD0 0x36
#define _FR_BM_B2_ADD0 0x80
#define _FR_BM_B3_BASE 0x01
#else
#define _FR_BM_B1_ADD0 0
#define _FR_BM_B2_ADD0 0
#define _FR_BM_B3_BASE 0
#endif

#if (defined FRSER_FEAT_SPI) || (defined FRSER_FEAT_LPCFWH)
#define FRSER_FEAT_S_BUSTYPE
#endif

#ifdef FRSER_FEAT_S_BUSTYPE
#define _FR_BM_B2_ADD1 0x04
#else
#define _FR_BM_B2_ADD1 0
#endif


#ifdef FRSER_FEAT_PIN_STATE
#define _FR_BM_B2_ADD2 0x20
#else
#define _FR_BM_B2_ADD2 0
#endif

#ifdef FRSER_FEAT_SPI
#ifdef FRSER_FEAT_SPISPEED
#define _FR_BM_B2_ADD3 0x18
#else
#define _FR_BM_B2_ADD3 0x08
#endif
#else
#define _FR_BM_B2_ADD3 0
#endif

#define FRSER_BM_B0 (_FR_BM_B0_BASE | _FR_BM_B0_ADD0)
#define FRSER_BM_B1 (_FR_BM_B1_BASE | _FR_BM_B1_ADD0)
#define FRSER_BM_B2 (_FR_BM_B2_BASE | _FR_BM_B2_ADD0 | _FR_BM_B2_ADD1 | _FR_BM_B2_ADD2 | _FR_BM_B2_ADD3)
#define FRSER_BM_B3 (_FR_BM_B3_BASE)

#include <stdint.h>

/* Teensy vs AVR compat. */
#ifndef pgm_read_ptr

#if INTPTR_MAX > 0x7FFFUL
#define pgm_read_ptr pgm_read_dword
#else
#define pgm_read_ptr pgm_read_word
#endif

#endif

#endif /* _FRSER_INT_H_ */
