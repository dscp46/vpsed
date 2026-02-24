#ifndef AX25_ADDR_H
#define AX25_ADDR_H

#include <stddef.h>
#include <stdint.h>

void decode_addr( const uint8_t *frame, size_t n, uint64_t *addr);

#endif	/* AX25_ADDR_H */
