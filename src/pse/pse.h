#ifndef PSE_PSE_H
#define PSE_PSE_H

#include "pse/common.h"
#include "pse/fib.h"
#include "pse/iface.h"

#include <utstring.h>

struct pse {
	iface_t *iface_list;
	fib_t *fib;
	pthread_mutex_t *writer_lock;

	void (*   add_iface)( struct pse *self, iface_t *iface);
	void (*switch_frame)( struct pse *self, UT_string *frame, int src_iface);
	void (*        free)( struct pse *self);
};

pse_t *pse_init(void);

#endif	/* PSE_PSE_H */
