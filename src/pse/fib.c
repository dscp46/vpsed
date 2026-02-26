#include "pse/fib.h"

#include <urcu.h>

void fib_insert( fib_t *self, uint64_t addr, int iface, int is_static);
void fib_prune( fib_t *self, uint64_t addr, int is_static);
int fib_find( fib_t *self, uint64_t addr);
void fib_tick( fib_t *self);
void fib_free( fib_t *self);

fib_entry_t *fib_list_duplicate( fib_entry_t *list);
void fib_list_free( fib_entry_t *list);

fib_t* fib_init()
{
	fib_t *self = (fib_t*) malloc( sizeof(fib_t));
	if( self == NULL )
		return NULL;

	rcu_assign_pointer( self->fib, NULL);
	self->insert = fib_insert;
	self->prune = fib_prune;
	self->find = fib_find;
	self->tick = fib_tick;
	self->free = fib_free;
	self->writer_lock = (pthread_mutex_t*) malloc( sizeof( pthread_mutex_t));
	pthread_mutex_init( self->writer_lock, NULL);
	return self;
}

void fib_insert( fib_t *self, uint64_t addr, int iface, int is_static)
{
	if( self == NULL ) return;

	fib_entry_t *entry;
	fib_entry_t *old_fib, *new_fib;
	pthread_mutex_lock( self->writer_lock);
	old_fib = rcu_dereference( self->fib);
	HASH_FIND_INT( old_fib, &addr, entry);
	
	if( entry != NULL )
	{
		// Entry found. Bump its TTL.
		entry->ttl = 0;
		
		// Override the destination interface
		if( is_static || !entry->is_static )
			entry->iface = iface;
		pthread_mutex_unlock( self->writer_lock);
		return;
	}
	
	entry = (fib_entry_t*) malloc( sizeof(fib_entry_t));
	entry->addr = addr;
	entry->iface = iface;
	entry->ttl = 0;
	entry->is_static = is_static;
	
	new_fib = fib_list_duplicate( old_fib);
	HASH_ADD_INT( new_fib, addr, entry);
	rcu_assign_pointer( self->fib, new_fib);
	pthread_mutex_unlock( self->writer_lock);
	
	synchronize_rcu();
	fib_list_free( old_fib);
}

void fib_prune( fib_t *self, uint64_t addr, int is_static)
{
	if( self == NULL ) return;

	fib_entry_t *entry, *old_fib, *new_fib;
	pthread_mutex_lock( self->writer_lock);

	old_fib = rcu_dereference( self->fib);
	HASH_FIND_INT( old_fib, &addr, entry);
	if( entry == NULL || ( !is_static && entry->is_static ))
	{
		pthread_mutex_unlock( self->writer_lock);
		return;
	}

	new_fib = fib_list_duplicate( old_fib);
	HASH_DEL( new_fib, entry);
	rcu_assign_pointer( self->fib, new_fib);
	pthread_mutex_unlock( self->writer_lock);
	
	synchronize_rcu();
	fib_list_free( old_fib);
	free( entry);
}

int fib_find( fib_t *self, uint64_t addr)
{
	if( self == NULL ) return FIB_FAIL;

	if( (addr & 0xffffffffffff0000ULL) == 0x9C60868298980000ULL ) return FIB_DROP; // N0CALL
	if( (addr & 0xffffffffffff0000ULL) == 0x9C9E868298980000ULL ) return FIB_DROP; // NOCALL

	if( (addr & 0xffff000000000000ULL) == 0x82A0000000000000ULL ) return FIB_MCAST; // AP
	if( (addr & 0xffff000000000000ULL) == 0x86A2000000000000ULL ) return FIB_MCAST; // CQ
	if( (addr & 0xffff000000000000ULL) == 0x88B0000000000000ULL ) return FIB_MCAST; // DX

	if( (addr & 0xffff000000000000ULL) == 0x9288000000000000ULL ) return FIB_MISS; // ID

	fib_entry_t *entry, *fib;
	rcu_read_lock();
	fib = rcu_dereference( self->fib);
	HASH_FIND_INT( fib, &addr, entry);
	rcu_read_unlock();

	return ( entry == NULL ) ? entry->iface : FIB_MISS;
}

void fib_tick( fib_t *self)
{
	fib_entry_t *cur, *tmp, *old_fib, *new_fib;

	pthread_mutex_lock( self->writer_lock);
	old_fib = rcu_dereference( self->fib);
	new_fib = fib_list_duplicate( old_fib);

	HASH_ITER( hh, new_fib, cur, tmp) 
	{
		// Prune dynamic entries with an aged-out TTL
		if( !(cur->is_static) && (++(cur->ttl) > FIB_TTL) )
		{
			HASH_DEL( new_fib, cur);
			free( cur);
		}
	}

	rcu_assign_pointer( self->fib, new_fib);
	pthread_mutex_unlock( self->writer_lock);
	
	synchronize_rcu();
	fib_list_free( old_fib);
}

void fib_free( fib_t *self)
{
	if( self == NULL ) return;

	fib_entry_t *cur, *tmp, *fib;

	rcu_read_lock();
	fib = rcu_dereference( self->fib);
	HASH_ITER( hh, fib, cur, tmp)
	{
		HASH_DEL( fib, cur);
		free( cur);
	}
	rcu_read_unlock();

	pthread_mutex_destroy( self->writer_lock);
	free( self);
}

fib_entry_t *fib_list_duplicate( fib_entry_t *list)
{
	if( list == NULL ) return NULL;

	fib_entry_t *cur, *tmp, *new_item, *new_list = NULL;

	HASH_ITER( hh, list, cur, tmp)
	{
		new_item = (fib_entry_t*) malloc( sizeof( fib_entry_t));
		memcpy( new_item, cur, sizeof( fib_entry_t));
		HASH_ADD_INT( new_list, addr, new_item);
	}

	return new_list;
}

void fib_list_free( fib_entry_t *list)
{
	if( list == NULL ) return;

	fib_entry_t *cur, *tmp;

	HASH_ITER( hh, list, cur, tmp)
	{
		HASH_DEL( list, cur);
		free( cur);
	}
}
