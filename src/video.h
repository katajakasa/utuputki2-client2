#ifndef VIDEO_H
#define VIDEO_H

#include <stdbool.h>

typedef struct {
	int w;
	int h;
	void *sfc;
} video_surface;

int video_init(bool fullscreen, int w, int h);
void video_close();

int video_init_surface(video_surface *surface, int w, int h);
int video_copy_to_surface(video_surface *surface, const unsigned char* data);
void video_free_surface(video_surface *surface);
void video_render_surface(video_surface *surface);

void video_present_start();
void video_present_finish();

#endif // VIDEO_H
