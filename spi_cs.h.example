
#ifndef _SPI_CS_H_
#define _SPI_CS_H_

/* These can be functions if it makes sense, but these compile to so few instructions. */

#define spi_select() do { DDRB |=_BV(0); } while(0)
/* delay because slow pullup. */
#define spi_deselect() do { DDRB &= ~_BV(0); _delay_us(1); } while(0);

#endif
