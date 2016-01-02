#ifndef PLAYER_H
#define PLAYER_H

enum STREAM_STATE {
	STREAM_STOPPED = 0,
	STREAM_PAUSED,
	STREAM_PLAYING
};

typedef struct {
	int id;
	void *format_context;
	int video_stream;
	int audio_stream;
	void *acodec;
	void *vcodec;
	void *vcodec_ctx;
	void *acodec_ctx;
} player_src;

typedef struct {
	int id;
	int w;
	int h;
	int state;
	void *rb;
	void *vframe;
	void *vframe_rgb;
	void *vconvert_ctx;
	void *aconvert_ctx;
	player_src *src;
} player_stream;

int player_init();
void player_close();

int player_open_src(player_src *src, const char *input_file);
void player_close_src(player_src *src);

int player_play_src(player_stream *stream, player_src *src);
int player_stream_src(player_stream *stream);
int player_stop_src(player_stream *stream);

unsigned char* player_stream_video_ptr(const player_stream *stream);
int player_stream_data_len(const player_stream *stream);
int player_stream_data_get(const player_stream *stream, unsigned char *data, int len);

#endif // PLAYER_H
