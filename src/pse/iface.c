#include "pse/iface.h"

#include <uthash.h>

int pse_next_iface_id( iface_t *iface_list)
{
	if( iface_list == NULL )
		return 0;

	iface_t *result;
	int next_id = -1;

	do
	{
		++next_id;
		HASH_FIND_INT( iface_list, &next_id, result);
	}
	while( result != NULL );

	return next_id;
}

iface_t *iface_list_duplicate( iface_t *cur_list)
{
	if( cur_list == NULL ) return NULL;

	iface_t *cur, *tmp, *new_item, *new_list = NULL;
	HASH_ITER( hh, cur_list, cur, tmp)
	{
		new_item = (iface_t*) malloc( sizeof( iface_t));
		memcpy( new_item, cur, sizeof( iface_t));
		HASH_ADD_INT( new_list, id, new_item);
	}

	return new_list;
}

void iface_list_free( iface_t *iface_list)
{
	if( iface_list == NULL ) return;
	iface_t *cur, *tmp;
	HASH_ITER( hh, iface_list, cur, tmp)
	{
		HASH_DEL( iface_list, cur);
		if( cur->close != NULL )
			cur->close( cur);
		free( cur);
	}
}
