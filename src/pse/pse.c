#include "ax25/addr.h"
#include "pse/pse.h"
#include "pse/iface.h"

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
	return self;
}

void pse_add_iface( pse_t *self, iface_t *iface)
{
	if( self == NULL || iface == NULL )
		return;

	int next_id = pse_next_iface_id( self->iface_list);
	iface->id = next_id;
	iface->mcast_enabled = 0;
	HASH_ADD_INT( self->iface_list, id, iface);
}

void pse_switch_frame( pse_t *self, UT_string *frame, int src_iface)
{
	if( self == NULL || self->iface_list == NULL || frame == NULL )
		return;

	iface_t *found_iface;
	uint64_t dst_addr, src_addr;
	uint8_t *frame_data = (uint8_t *)utstring_body( frame);
	size_t frame_sz = utstring_len( frame);

	// Skip runts
	if( frame_sz < 16 )
	{
		HASH_FIND_INT( self->iface_list, &src_iface, found_iface);
		if( found_iface != NULL )
			++(found_iface->runts);
		return;
	}

	decode_addr( frame_data  , frame_sz  , &dst_addr);
	decode_addr( frame_data+7, frame_sz-7, &src_addr);

}

void pse_free( pse_t *self)
{
	if( self == NULL ) return;
	iface_t *cur, *tmp;

	if( self->iface_list != NULL )
	{
		HASH_ITER( hh, self->iface_list, cur, tmp)
		{
			HASH_DEL( self->iface_list, cur);
			free( cur);
		}
	}
	
	free( self);
}
