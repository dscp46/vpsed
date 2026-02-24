#include "ax25/addr.h"

#include <endian.h>
#include <stdio.h>
#include <string.h>

void decode_addr( const uint8_t *frame, size_t n, uint64_t *addr)
{
	if( frame == NULL || addr == NULL || n<7) return;

	memcpy( addr, frame, 7);
	*addr = be64toh( *addr);
	*addr &= 0xffffffffffff1e00ULL;
}

