#include "engine.h"
#include "video.h"
#include "audio.h"
#include "player.h"

#include <stdio.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

int engine_run(const char *input_file) {
	SDL_Event event;
	int retval = 1;
	bool run = true;


	player_src source;
	player_open_src(&source, input_file);

	player_stream stream;
	player_play_src(&stream, &source);

	video_surface surface;
	video_init_surface(&surface, stream.w, stream.h);

	audio_stream sink;
	audio_init_stream(&sink, 2, 2);

	while(run) {
		// Poke at the streamer
		while(run) {
			int ret = player_stream_src(&stream);
			if(ret == -1) {
				// Got a video frame, show it.
				unsigned char *data_ptr = player_stream_video_ptr(&stream);
				video_copy_to_surface(&surface, data_ptr);
				break;
			}
			if(ret == -2) {
				// Got an audio frame, queue it.
				int data_len = player_stream_data_len(&stream);
				if(data_len >= 8192) {
					unsigned char b[8192];
					player_stream_data_get(&stream, b, 8192);
					audio_update_stream(&sink, b, 8192, 44100);
				}
			}
			if(ret == 0) {
				// Stream end
				fprintf(stderr, "engine: Stream finished\n");
				run = false;
			}
		}

		// Handle input events
     	SDL_PollEvent(&event);
		switch(event.type) {
			case SDL_KEYUP:
				if(event.key.keysym.sym == SDLK_ESCAPE) {
					run = false;
				}
				break;
			case SDL_QUIT:
				run = false;
				break;
		}

		video_present_start();
		video_render_surface(&surface);
		video_present_finish();
		//audio_render_stream(&sink);
	}

	audio_close_stream(&sink);
	video_free_surface(&surface);
	player_stop_src(&stream);
	player_close_src(&source);

	return retval;
}