#ifndef _SPIHW_AVRSPI_H_
#define _SPIHW_AVRSPI_H_
/* This will give spi_set_speed if needed. A bit illogically maybe, but anyways... */
#include "frser-flashapi.h"

void spi_init(void);
uint8_t spi_uninit(void);
uint8_t spi_txrx(const uint8_t c);

#define FRSER_ASYNC_SPI_API
void spi_awrite_fast(uint8_t d);
void spi_awrite(uint8_t d);
uint8_t spi_aread(void);

/* Async SPI API:
 * write_fast: assumes SPI idle*, starts a transfer
 * write: will wait for space (or use a FIFO if available)
 * read: returns the byte associated with the last write_fast / write (=will wait for transfer complete).
 *
 * *SPI is idle after spi_read / spi_txrx / spi_init.
 * Do NOT:
 * write* + write_fast
 * (no write)/read + read
 * - those undefined actions can lead to anything, including infinite loop/reset/undefined TX value/things appearing to work.
 * Note: spi_txrx is allowed to be write_fast + read. thus do not write* + spi_txrx.
 */

void spi_init_cond(void);

#endif
