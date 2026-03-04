#include "iface/unix_kiss.h"

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>

#define UNIX_READER_BUF_SZ	256

static void unix_kiss_send( iface_t *self, UT_string *frame);
static void unix_kiss_set_pse( iface_t *self, pse_t *pse);
static void unix_kiss_free( iface_t *self);
static void* unix_kiss_reader( void* runarg);

iface_t *unix_kiss_new( int fd)
{
	iface_t *self = (iface_t *) malloc( sizeof( iface_t));
	if( self == NULL ) return NULL;
	unix_kiss_t *priv = (unix_kiss_t *) malloc( sizeof( unix_kiss_t));
	if( priv == NULL )
	{
		free( self);
		return NULL;
	}
	priv->reader = (pthread_t*) malloc( sizeof(pthread_t));

	priv->fd = fd;
	priv->pse = NULL;
	self->stats = iface_stats_init();
	self->p = priv;
	self->send = unix_kiss_send;
	self->set_pse = unix_kiss_set_pse;
	self->close = unix_kiss_free;

	priv->run_reader = 1;
	pthread_create( priv->reader, NULL, unix_kiss_reader, priv);

	return self;
}

static void unix_kiss_send( iface_t *self, UT_string *frame)
{
	if( self == NULL || frame == NULL ) return;

	unix_kiss_t *priv = (unix_kiss_t*) self->p;
	if( priv == NULL ) return;

	const char *buffer = utstring_body( frame);
	size_t len = utstring_len( frame), written;
	do
	{
		written += write( priv->fd, buffer, len);
	}
	while( written < len);
}

static void unix_kiss_set_pse( iface_t *self, pse_t *pse)
{
	if( self == NULL ) return;

	unix_kiss_t *priv = (unix_kiss_t*) self->p;
	if( priv == NULL ) return;

	priv->pse = pse;
}

static void unix_kiss_free( iface_t *self)
{
	if( self == NULL ) return;
	unix_kiss_t *priv = self->p;
	void *retval;

	if( self->stats != NULL )
		free( self->stats);

	if( priv != NULL )
	{
		if( priv->fd >= 0 )
			shutdown( priv->fd, SHUT_RDWR);

		if( priv->reader != NULL)
		{
			priv->run_reader = 0;
			pthread_join( *(priv->reader), &retval);
			free( priv->reader);
		}

		if( priv->pse != NULL )
			priv->pse->detach_iface( priv->pse, self->id);

		if( priv->fd >= 0 )
			close( priv->fd);

		free( priv);
	}
	free( self);
}

static void* unix_kiss_reader( void* runarg)
{
	if( runarg == NULL ) return NULL;
	unix_kiss_t *priv = (unix_kiss_t*) runarg;

	char *buffer = malloc( UNIX_READER_BUF_SZ);
	ssize_t read_bytes;

	while( priv->run_reader)
	{
		if( priv->pse == NULL)
		{
			sleep(1);
			continue;
		}

		if((read_bytes = read( priv->fd, buffer, UNIX_READER_BUF_SZ)) < 0 )
		{
			if( read_bytes == EAGAIN || read_bytes == EWOULDBLOCK )
				continue;
			break;
		}

		// TODO: KISS processing
	}

	free( buffer);
	return NULL;
}
