
#ifdef FRSER_FEAT_DPRINTF

void dxprint_put(const PGM_P fmt, ...);
void dxprint_tx(void (*txfp)(uint8_t));

#define dprintf(x,...) dxprint_put(PSTR(x) , ## __VA_ARGS__)

#define DXP_BUFSZ 64

#else

#define dprintf(x,...)

#define DXP_BUFSZ 0

#endif
