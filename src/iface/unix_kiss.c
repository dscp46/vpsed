#include "iface/unix_kiss.h"

#include <unistd.h>

void unix_kiss_send( iface_t *self, UT_string *frame);
void unix_kiss_free( iface_t *self);

iface_t *unix_kiss_new( int fd)
{
	iface_t *self = (iface_t *) malloc( sizeof( iface_t));
	if( self == NULL ) return NULL;
	unix_kiss_t *opaque = (unix_kiss_t *) malloc( sizeof( unix_kiss_t));
	if( opaque == NULL )
	{
		free( self);
		return NULL;
	}

	opaque->fd = fd;
	self->p = opaque;
	self->pse = NULL;
	self->send = unix_kiss_send;
	self->close = unix_kiss_free;

	// FIXME: run socket reader thread

	return self;
}

void unix_kiss_send( iface_t *self, UT_string *frame)
{
	if( self == NULL || frame == NULL ) return;

	unix_kiss_t *opaque = (unix_kiss_t*) self->p;
	if( opaque == NULL ) return;

	const char *buffer = utstring_body( frame);
	size_t len = utstring_len( frame), written;
	do
	{
		written += write( opaque->fd, buffer, len);
	}
	while( written < len);
}

void unix_kiss_free( iface_t *self)
{
	if( self == NULL ) return;
	unix_kiss_t *opaque = self->p;
	if( opaque != NULL )
	{
		if( self->pse != NULL )
			self->pse->detach_iface( self->pse, self->id);

		if( opaque->fd >= 0 )
			close( opaque->fd);

		free( opaque);
	}
	free( self);
}
