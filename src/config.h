#ifndef CONFIG_H
#define CONFIG_H

typedef struct config {
	int foreground;
	char *kiss_addr;
	char *kiss_port;
	char *unix_fname;
} config_t;

int parse_cli_args( int argc, char *argv[], config_t *cfg);

#endif	/* CONFIG_H */
