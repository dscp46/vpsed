#ifndef PSE_IFACE_H
#define PSE_IFACE_H

#include <uthash.h>

typedef struct iface {
	int id;
	int mcast_enabled;
	size_t runts;
	void *p;
	UT_hash_handle hh;
} iface_t;

int pse_next_iface_id( iface_t *iface_list);

#endif	/* PSE_IFACE_H */
