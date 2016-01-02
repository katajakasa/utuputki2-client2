#ifndef ARGS_H
#define ARGS_H

#include <stdbool.h>

typedef struct {
	bool fullscreen;
	int w;
	int h;
	char input_file[1024];
} cmd_args;

int args_parse(cmd_args *args, int argc, char *argv[]);

#endif // ARGS_H
