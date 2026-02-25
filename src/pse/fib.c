#include "pse/fib.h"

void fib_insert( fib_t *self, uint64_t addr, int iface, int is_static);
void fib_prune( fib_t *self, uint64_t addr, int is_static);
int fib_find( fib_t *self, uint64_t addr);
void fib_tick( fib_t *self);
void fib_free( fib_t *self);

fib_t* fib_init()
{
	fib_t *self = (fib_t*) malloc( sizeof(fib_t));
	if( self == NULL )
		return NULL;

	self->fib  = NULL;
	self->insert = fib_insert;
	self->prune = fib_prune;
	self->find = fib_find;
	self->tick = fib_tick;
	self->free = fib_free;
	return self;
}

void fib_insert( fib_t *self, uint64_t addr, int iface, int is_static)
{
	if( self == NULL ) return;

	fib_entry_t *entry;
	HASH_FIND_INT( self->fib, &addr, entry);

	// Skip if we find a static entry
	if( !is_static && entry != NULL && entry->is_static )
		return;

	entry = (fib_entry_t*) malloc( sizeof(fib_entry_t));
	entry->addr = addr;
	entry->iface = iface;
	entry->ttl = 0;
	entry->is_static = is_static;
	HASH_ADD_INT( self->fib, addr, entry);
}

void fib_prune( fib_t *self, uint64_t addr, int is_static)
{
	if( self == NULL ) return;

	fib_entry_t *entry;
	HASH_FIND_INT( self->fib, &addr, entry);

	if( entry == NULL || ( !is_static && entry->is_static ))
		return;

	HASH_DEL( self->fib, entry);
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

	fib_entry_t *entry;
	HASH_FIND_INT( self->fib, &addr, entry);

	return ( entry == NULL ) ? entry->iface : FIB_MISS;
}

void fib_tick( fib_t *self)
{
	fib_entry_t *cur, *tmp;

	HASH_ITER( hh, self->fib, cur, tmp) 
	{
		// Prune dynamic entries with an aged-out TTL
		if( !(cur->is_static) && (++(cur->ttl) > FIB_TTL) )
		{
			HASH_DEL( self->fib, cur);
			free( cur);
		}
	}
}

void fib_free( fib_t *self)
{
	if( self == NULL ) return;

	fib_entry_t *cur, *tmp;
	HASH_ITER( hh, self->fib, cur, tmp)
	{
		HASH_DEL( self->fib, cur);
		free( cur);
	}

	free( self);
}
