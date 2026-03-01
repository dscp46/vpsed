#ifndef PSE_IFACE_H
#define PSE_IFACE_H

#include "pse/common.h"

#include <uthash.h>
#include <utstring.h>

struct iface {
	int id;
	int mcast_enabled;
	size_t runts;
	void *p;
	pse_t *pse;
	UT_hash_handle hh;
	void (* send)( struct iface *self, UT_string *frame);
	void (*close)( struct iface *self);
};

int pse_next_iface_id( iface_t *iface_list);
iface_t *iface_list_duplicate( iface_t *cur_list);
void iface_list_free( iface_t *iface_list);

#endif	/* PSE_IFACE_H */
