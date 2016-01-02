#ifndef AUDIO_H
#define AUDIO_H

#define AUDIO_BUFFER_COUNT 2

enum AUDIO_STATE {
	AUDIO_STOPPED = 0,
	AUDIO_PAUSED,
	AUDIO_PLAYING
};

typedef struct {
	unsigned int source;
	unsigned int buffers[AUDIO_BUFFER_COUNT];
	int format;
	int state;
	int starting;
} audio_stream;

int audio_init();
void audio_close();

int audio_init_stream(audio_stream *stream, int channels, int bytes);
int audio_update_stream(audio_stream *stream, unsigned char *data, unsigned int len, int freq);
void audio_render_stream(audio_stream *stream);
void audio_close_stream(audio_stream *stream);

#endif // AUDIO_H
