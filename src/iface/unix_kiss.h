#ifndef IFACE_UNIX_KISS_H
#define IFACE_UNIX_KISS_H

#include "pse/iface.h"
#include "pse/pse.h"

#include <pthread.h>

typedef struct unix_kiss_priv {
	int fd;
	pse_t *pse;
	pthread_t *reader;
} unix_kiss_t;

iface_t *unix_kiss_new( int fd);

#endif	/* IFACE_UNIX_KISS_H */
