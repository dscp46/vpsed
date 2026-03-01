#ifndef IFACE_UNIX_KISS_H
#define IFACE_UNIX_KISS_H

#include "pse/iface.h"
#include "pse/pse.h"

typedef struct unix_kiss_opaque {
	int fd;
} unix_kiss_t;

iface_t *unix_kiss_new( int fd);

#endif	/* IFACE_UNIX_KISS_H */
