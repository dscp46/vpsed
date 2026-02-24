#include <stdio.h>
#include <stdlib.h>

#include "config.h"

int main( int argc, char *argv[])
{
	int retval;
	config_t *cfg = (config_t *) malloc( sizeof(config_t));

	if((retval = parse_cli_args( argc, argv, cfg)))
	{
		free( cfg);
		return retval;
	}


	free(cfg);
	return 0;
}
