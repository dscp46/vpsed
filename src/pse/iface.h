#ifndef PSE_IFACE_H
#define PSE_IFACE_H

#include "pse/common.h"

#include <uthash.h>
#include <utstring.h>

typedef struct iface_stats {
	size_t sent;
	size_t received;
	size_t giants;
	size_t runts;
} iface_stats_t;

struct iface {
	int id;
	int mcast_enabled;
	iface_stats_t *stats;
	void *p;
	UT_hash_handle hh;
	void (*   send)( struct iface *self, UT_string *frame);
	void (*set_pse)( struct iface *self, pse_t *pse);
	void (*  close)( struct iface *self);
};

int pse_next_iface_id( iface_t *iface_list);
iface_t *iface_list_duplicate( iface_t *cur_list);
void iface_list_free( iface_t *iface_list);

iface_stats_t *iface_stats_init( void);

#endif	/* PSE_IFACE_H */
