#ifndef PSE_FIB_H
#define PSE_FIB_H

#include <pthread.h>
#include <stdint.h>
#include <urcu.h>
#include <uthash.h>

#define FIB_MCAST	-1
#define FIB_DROP	-2
#define FIB_MISS	-3
#define FIB_FAIL	-4

// TTL: 30 minutes
#define	FIB_TTL		1800

typedef struct fib_entry {
	uint64_t addr;
	int iface;
	int ttl;
	int is_static;
	UT_hash_handle hh;
	struct rcu_head rcu;
} fib_entry_t;

typedef struct fib {
	fib_entry_t *fib;
	pthread_mutex_t *writer_lock;

	void (*insert)( struct fib *self, uint64_t addr, int iface, int is_static);
	void (* prune)( struct fib *self, uint64_t addr, int is_static);
	int  (*  find)( struct fib *self, uint64_t addr);
	void (*  tick)( struct fib *self);
	void (*  free)( struct fib *self);
} fib_t;


fib_t* fib_init();

#endif	/* PSE_FIB_H */
