#include "config.h"

#include <getopt.h>
#include <stdio.h>

#define KISS_ADDR	"::"
#define KISS_PORT	"8001"
#define KISS_UNIX_PATH	"/run/vpsed/vpsed.sock"

void usage( char *argv[])
{
	printf( "Usage: %s [args]\n", argv[0]);
	printf( "   -f: Run in foreground mode (default: no)\n");
	printf( "   -p <port>: Set KISS TCP server port (default: %s)\n", KISS_PORT);
	printf( "   -t <addr>: Set KISS TCP server address (default: %s)\n", KISS_ADDR);
	printf( "   -u <path>: Set UNIX socket path (default: %s)\n", KISS_UNIX_PATH);
}

int parse_cli_args( int argc, char *argv[], config_t *cfg)
{
	if( argv == NULL || cfg == NULL ) return -1;
	int opt;

	cfg->foreground = 0;
	cfg->kiss_addr = NULL;
	cfg->kiss_port = NULL;
	cfg->unix_fname = NULL;

	// Skip if no argument provided
	if( argc == 1 ) return 0;

	while((opt = getopt( argc, argv, ":fp:t:u:")) != -1)
	{
		switch( opt)
		{
		case 'f':
			cfg->foreground = 1;
			break;

		case 'p':
			cfg->kiss_port = optarg;
			break;

		case 't':
			cfg->kiss_addr = optarg;
			break;

		case 'u':
			cfg->unix_fname = optarg;
			break;

		case ':':
			printf( "Option needs a value\n");
			return 1;

		default:
			printf( "Unrecognized option '%c'.\n", optopt);
			return 1;
		}
	}

	cfg->kiss_addr  = (cfg->kiss_addr  != NULL) ? cfg->kiss_addr  : KISS_ADDR;
	cfg->kiss_port  = (cfg->kiss_port  != NULL) ? cfg->kiss_port  : KISS_PORT;
	cfg->unix_fname = (cfg->unix_fname != NULL) ? cfg->unix_fname : KISS_UNIX_PATH;
	return 0;
}
