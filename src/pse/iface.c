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

