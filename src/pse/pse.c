#include "ax25/addr.h"
#include "pse/pse.h"
#include "pse/iface.h"

#include <pthread.h>
#include <urcu.h>
#include <utstring.h>

void pse_add_iface( pse_t *self, iface_t *iface);
void pse_detach_iface( pse_t *self, int id);
void pse_switch_frame( pse_t *self, UT_string *frame, int src_iface);
void pse_free( pse_t *self);

pse_t *pse_init(void)
{
	pse_t *self = (pse_t*) malloc( sizeof( pse_t));
	if( self == NULL ) return NULL;

	self->fib = fib_init();
	self->iface_list = NULL;
	self->add_iface = pse_add_iface;
	self->detach_iface = pse_detach_iface;
	self->switch_frame = pse_switch_frame;
	self->free = pse_free;
	self->writer_lock = (pthread_mutex_t *)malloc( sizeof( pthread_mutex_t));
	pthread_mutex_init( self->writer_lock, NULL);
	return self;
}

void pse_add_iface( pse_t *self, iface_t *iface)
{
	if( self == NULL || iface == NULL )
		return;
	iface_t *old_iface_list, *new_iface_list;

	pthread_mutex_lock( self->writer_lock);
	old_iface_list = rcu_dereference( self->iface_list);

	int next_id = pse_next_iface_id( old_iface_list);
	new_iface_list = iface_list_duplicate( old_iface_list);
	iface->id = next_id;
	iface->mcast_enabled = 0;
	iface->pse = self;
	HASH_ADD_INT( new_iface_list, id, iface);

	rcu_assign_pointer( self->iface_list, new_iface_list);
	pthread_mutex_unlock( self->writer_lock);

	synchronize_rcu();
	iface_list_free( old_iface_list);
}

void pse_detach_iface( pse_t *self, int id)
{
	if( self == NULL )
		return;

	iface_t *old_iface_list, *new_iface_list, *iface;

	rcu_read_lock();
	old_iface_list = rcu_dereference( self->iface_list);
	HASH_FIND_INT( old_iface_list, &id, iface);
	if( iface == NULL )
	{
		rcu_read_unlock();
		return;
	}
	rcu_read_unlock();

	// FIXME: Prune all FIB entries linked to interface

	pthread_mutex_lock( self->writer_lock);
	old_iface_list = rcu_dereference( self->iface_list);
	new_iface_list = iface_list_duplicate( old_iface_list);
	HASH_FIND_INT( new_iface_list, &id, iface);
	HASH_DEL( new_iface_list, iface);
	pthread_mutex_unlock( self->writer_lock);

	synchronize_rcu();
	iface_list_free( old_iface_list);
}

void pse_switch_frame( pse_t *self, UT_string *frame, int src_iface)
{
	if( self == NULL || self->iface_list == NULL || frame == NULL )
		return;

	iface_t *found_iface, *iface_list, *cur, *tmp;
	uint64_t dst_addr, src_addr;
	uint8_t *frame_data = (uint8_t *)utstring_body( frame);
	size_t frame_sz = utstring_len( frame);

	rcu_read_lock();
	iface_list = rcu_dereference( self->iface_list);
	HASH_FIND_INT( iface_list, &src_iface, found_iface);
	rcu_read_unlock();

	// Skip runts
	if( frame_sz < 16 )
	{
		if( found_iface != NULL )
			++(found_iface->runts);
		return;
	}

	decode_addr( frame_data  , frame_sz  , &dst_addr);
	decode_addr( frame_data+7, frame_sz-7, &src_addr);

	// Drop well-known bad callsigns
	if( (src_addr & 0xffffffffffff0000ULL) == 0x9C60868298980000ULL ) return; // N0CALL
	if( (src_addr & 0xffffffffffff0000ULL) == 0x9C9E868298980000ULL ) return; // NOCALL

	// Add callsign in FIB, or update its TTL
	self->fib->insert( self->fib, src_addr, src_addr, 0);

	rcu_read_lock();
	iface_list = rcu_dereference( self->iface_list);

	switch( self->fib->find( self->fib, dst_addr) )
	{
	case FIB_MCAST:
		// "Multicast" for APRS and DXCluster
		HASH_ITER( hh, iface_list, cur, tmp)
		{
			if( cur->mcast_enabled && cur->id != src_iface )
				cur->send( cur, frame);
		}
		break;

	case FIB_DROP:
		// Drop packet
		break;

	case FIB_MISS:
		// Broadcast on a FIB miss
		HASH_ITER( hh, iface_list, cur, tmp)
		{
			if( cur->id != src_iface )
				cur->send( cur, frame);
		}
		break;

	case FIB_FAIL:
		// TODO: Raise a minor alarm
		break;

	default:
		// Unicast
		HASH_FIND_INT( iface_list, &src_iface, found_iface);
		if( found_iface != NULL )
			found_iface->send( found_iface, frame);
	}
	rcu_read_unlock();
}

void pse_free( pse_t *self)
{
	if( self == NULL ) return;
	iface_t *cur, *tmp, *iface_list;

	if( self->iface_list != NULL )
	{
		pthread_mutex_lock( self->writer_lock);
		iface_list = rcu_dereference( self->iface_list);

		HASH_ITER( hh, iface_list, cur, tmp)
		{
			HASH_DEL( iface_list, cur);
			free( cur);
		}

		pthread_mutex_unlock( self->writer_lock);
	}

	if( self->writer_lock != NULL )
	{
		pthread_mutex_destroy( self->writer_lock);
		free( self->writer_lock);
	}
	if( self->fib != NULL )
		self->fib->free( self->fib);

	free( self);
}
