#include "video.h"
#include <SDL2/SDL.h>
#include <stdio.h>

static SDL_Window *window = NULL;
static SDL_Renderer * renderer = NULL;

int video_init(bool fullscreen, int w, int h) {
	fprintf(stderr, "Initializing video subsystem ...\n");

	// Set up video window
	int flags = fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
	window = SDL_CreateWindow(
		"Utuclient v2.0",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		w, h, flags);
	if(!window) {
		fprintf(stderr, "Unable to set up video window: %s\n", SDL_GetError());
		goto exit_0;
	}

	// Set up a renderer
	renderer = SDL_CreateRenderer(
		window,
		-1, // Dev indec (any)
		SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
	if(!renderer) {
		fprintf(stderr, "Unable to set up a renderer: %s\n", SDL_GetError());
		goto exit_1;
	}

	// Print out renderer information
	SDL_RendererInfo info;
	SDL_GetRendererInfo(renderer, &info);
	fprintf(stderr, " * Renderer: %s\n", info.name);

	// Check flags
	if(info.flags & SDL_RENDERER_SOFTWARE)
		fprintf(stderr, " * Using SDL_RENDERER_SOFTWARE\n");
	if(info.flags & SDL_RENDERER_ACCELERATED)
		fprintf(stderr, " * Using SDL_RENDERER_ACCELERATED\n");
	if(info.flags & SDL_RENDERER_PRESENTVSYNC)
		fprintf(stderr, " * Using SDL_RENDERER_PRESENTVSYNC\n");
	if(info.flags & SDL_RENDERER_TARGETTEXTURE)
		fprintf(stderr, " * Using SDL_RENDERER_TARGETTEXTURE\n");

	// Texture limits
	fprintf(stderr, " * Texture size limit: %d x %d\n",
		info.max_texture_width, info.max_texture_height);

	// All done.
	return 0;

exit_1:
	SDL_DestroyWindow(window);
exit_0:
	return 1;
}

void video_close() {
	fprintf(stderr, "Closing video subsystem.\n");
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	renderer = NULL;
	window = NULL;
}

int video_init_surface(video_surface *surface, int w, int h) {
	SDL_Texture *tex = SDL_CreateTexture(
		renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, w, h);
	if(!tex) {
		fprintf(stderr, "video: Unable to allocate a new texture\n");
		return 1;
	}
	surface->sfc = tex;
	surface->w = w;
	surface->h = h;
	fprintf(stderr, "video: Initialized %dx%d texture.\n", w, h);
	return 0;
}

int video_copy_to_surface(video_surface *surface, const unsigned char* data) {
	SDL_Texture *tex = (SDL_Texture*)surface->sfc;
	unsigned char* pixels;
	int pitch;

	// Lock texture, copy data over, unlock.
	if(SDL_LockTexture(tex, NULL, (void**)&pixels, &pitch) == 0) {
		memcpy(pixels, data, pitch * surface->h);
		SDL_UnlockTexture(tex);
	} else {
		fprintf(stderr, "video: Unable to lock texture for streaming access.\n");
		return 1;
	}
	return 0;
}

void video_free_surface(video_surface *surface) {
	fprintf(stderr, "video: Texture freed.\n");
	SDL_DestroyTexture((SDL_Texture*)surface->sfc);
	surface->w = 0;
	surface->h = 0;
}

void video_render_surface(video_surface *surface) {
	SDL_RenderCopy(renderer, (SDL_Texture*)surface->sfc, NULL, NULL);
}

void video_present_start() {
	SDL_RenderClear(renderer);
}

void video_present_finish() {
	SDL_RenderPresent(renderer);
}
