#include "ax25/addr.h"
#include "pse/pse.h"
#include "pse/iface.h"

#include <pthread.h>
#include <urcu.h>
#include <utstring.h>

void pse_add_iface( pse_t *self, iface_t *iface);
void pse_switch_frame( pse_t *self, UT_string *frame, int src_iface);
void pse_free( pse_t *self);

pse_t *pse_init(void)
{
	pse_t *self = (pse_t*) malloc( sizeof( pse_t));
	if( self == NULL ) return NULL;

	self->iface_list = NULL;
	self->add_iface = pse_add_iface;
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
	HASH_ADD_INT( new_iface_list, id, iface);

	rcu_assign_pointer( self->iface_list, new_iface_list);
	pthread_mutex_unlock( self->writer_lock);

	synchronize_rcu();
	iface_list_free( old_iface_list);
}

void pse_switch_frame( pse_t *self, UT_string *frame, int src_iface)
{
	if( self == NULL || self->iface_list == NULL || frame == NULL )
		return;

	iface_t *found_iface, *iface_list;
	uint64_t dst_addr, src_addr;
	uint8_t *frame_data = (uint8_t *)utstring_body( frame);
	size_t frame_sz = utstring_len( frame);

	// Skip runts
	if( frame_sz < 16 )
	{
		rcu_read_lock();
		iface_list = rcu_dereference( self->iface_list);
		HASH_FIND_INT( iface_list, &src_iface, found_iface);
		if( found_iface != NULL )
			++(found_iface->runts);
		rcu_read_unlock();
		return;
	}

	decode_addr( frame_data  , frame_sz  , &dst_addr);
	decode_addr( frame_data+7, frame_sz-7, &src_addr);

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

	free( self);
}
