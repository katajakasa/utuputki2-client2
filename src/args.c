#include "args.h"
#include <argtable2.h>
#include <stdio.h>
#include <string.h>

int args_parse(cmd_args *args, int argc, char *argv[]) {
	int retval = 1;
    struct arg_lit *help = arg_lit0("h", "help", "print this help and exit");
    struct arg_lit *vers = arg_lit0("v", "version", "print version information and exit");
    struct arg_file *file = arg_file1("f", "file", "<file>", "Movie to play");
    struct arg_int *width = arg_int1("x", "width", "<int>", "Window width");
    struct arg_int *height = arg_int1("y", "height", "<int>", "Window height");
    struct arg_lit *fullscreen = arg_lit0("m", "fullscreen", "Fullscreen mode");
    struct arg_end *end = arg_end(30);
    void* argtable[] = {help, vers, file, width, height, fullscreen, end};
    const char* progname = "utuclient2";

    // Make sure everything got allocated
    if(arg_nullcheck(argtable) != 0) {
        fprintf(stderr, "%s: insufficient memory\n", progname);
        goto exit_0;
    }

    // Parse arguments
    int nerrors = arg_parse(argc, argv, argtable);

    // Handle help
    if(help->count > 0) {
        fprintf(stderr, "Usage: %s", progname);
        arg_print_syntax(stderr, argtable, "\n");
        fprintf(stderr, "\nArguments:\n");
        arg_print_glossary(stderr, argtable, "%-25s %s\n");
        retval = -1;
        goto exit_0;
    }

    // Handle version
    if(vers->count > 0) {
        fprintf(stderr, "%s v0.1\n", progname);
        fprintf(stderr, "C/SDL2/FFMPEG Utuputki2 client.\n");
        retval = -1;
        goto exit_0;
    }

    // Handle errors
    if(nerrors > 0) {
        arg_print_errors(stderr, end, progname);
        fprintf(stderr, "Try '%s --help' for more information.\n", progname);
        goto exit_0;
    }

    // Set cmd_args variable.
    memset(args, 0, sizeof(cmd_args));
    strncpy(args->input_file, file->filename[0], sizeof(args->input_file));
    args->w = width->ival[0];
    args->h = height->ival[0];
    args->fullscreen = (fullscreen->count > 0);

    // All done, finish up.
    retval = 0;
exit_0:
	// Free argtable2 stuff
    arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
	return retval;
}
