#include <SDL2/SDL.h>
#include <stdio.h>

#include "args.h"
#include "video.h"
#include "audio.h"
#include "player.h"
#include "framework.h"
#include "engine.h"

int main(int argc, char *argv[]) {
	int retval = 1;

	// Handle arguments. Retval 0 means "success", pass.
	// If we got -1, return 0 and exit. (Printed help text or somesuch).
	// If we got > 0, it is a real error => Exit.
	cmd_args args;
	int agv = args_parse(&args, argc, argv);
	if(agv == -1) {
		retval = 0;
		goto exit_0;
	}
	if(agv > 0) {
		retval = agv;
		goto exit_0;
	}

	// Let's start initializing stuff ...
    fprintf(stderr, "Starting utuclient v2 ...\n");
    
    // Init SDL2, basically
    if(framework_init() != 0) {
    	goto exit_0;
    }

    // Initialize video subsystem
    if(video_init(args.fullscreen, args.w, args.h) != 0) {
    	goto exit_1;
    }

    // Initialize audio subsystem
    if(audio_init() != 0) {
    	goto exit_2;
    }

    // Initialize player subsystem
    if(player_init() != 0) {
    	goto exit_3;
    }

    // Run engine.
    fprintf(stderr, "--- Running ---\n");
    retval = engine_run(args.input_file);

    // That's that. Free up everything.
    fprintf(stderr, "--- Closing ---\n");
    player_close();
exit_3:
	audio_close();
exit_2:
	video_close();
exit_1:
    framework_close();
exit_0:
	return retval;
}
