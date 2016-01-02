#include "player.h"
#include "ringbuffer.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>

#include <stdio.h>
#include <stdbool.h>

static int _stream_id = 1;
static int _source_id = 1;

int gen_stream_id() { return _stream_id++; }
int gen_source_id() { return _source_id++; }

int player_init() {
	fprintf(stderr, "Initializing player subsystem ...\n");
	av_register_all();
	avformat_network_init();
	return 0;
}

void player_close() {
	fprintf(stderr, "Closing player subsystem.\n");
	avformat_network_deinit();
}

int player_open_src(player_src *src, const char *input_file) {
	int retval = 1;
	int videostream = -1;
	int audiostream = -1;
	AVFormatContext *format_context = NULL;
	AVCodecContext *audio_codec_context = NULL;
	AVCodecContext *video_codec_context = NULL;
	AVCodec *audio_codec = NULL;
	AVCodec *video_codec = NULL;
	int source_id = gen_source_id();

	fprintf(stderr, "player: source %d: Opening '%s' ...\n", source_id, input_file);

	// Open source and get stream information
	if(avformat_open_input(&format_context, input_file, NULL, NULL) < 0) {
		fprintf(stderr, "player: source %d: Unable to open source '%s'\n", source_id, input_file);
		goto exit_0;
	}
	if(avformat_find_stream_info(format_context, NULL) < 0) {
		fprintf(stderr, "player: source %d: Unable to find stream info\n", source_id);
		goto exit_1;
	}

	// Dump information about file
	//av_dump_format(format_context, 0, input_file, 0);

	// Find the first video stream
	audiostream = av_find_best_stream(format_context, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	videostream = av_find_best_stream(format_context, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if(videostream == -1) {
		fprintf(stderr, "player: source %d: Unable to find video stream.\n", source_id);
		goto exit_1;
	}
	if(audiostream == -1) {
		fprintf(stderr, "player: source %d: Unable to find audio stream.\n", source_id);
		goto exit_1;
	}

	// Find video decoder
	video_codec_context = format_context->streams[videostream]->codec;
	video_codec = avcodec_find_decoder(video_codec_context->codec_id);
	if(!video_codec) {
		fprintf(stderr, "player: source %d: No suitable video decoder found.\n", source_id);
		goto exit_1;
	}
	fprintf(stderr, "player: source %d: Video #%d: %s\n", source_id, videostream, video_codec->long_name);

	// Create a video decoder context
	if(avcodec_open2(video_codec_context, video_codec, NULL) < 0) {
		fprintf(stderr, "player: source %d: Unable to allocate video codec context\n", source_id);
		goto exit_1;
	}

	// Find audio decoder
	audio_codec_context = format_context->streams[audiostream]->codec;
	audio_codec = avcodec_find_decoder(audio_codec_context->codec_id);
	if(!audio_codec) {
		fprintf(stderr, "player: source %d: No suitable audio decoder found.\n", source_id);
		goto exit_1;
	}
	fprintf(stderr, "player: source %d: Audio #%d: %s\n", source_id, audiostream, audio_codec->long_name);

	// Create an audio decoder context
	if(avcodec_open2(audio_codec_context, audio_codec, NULL) < 0) {
		fprintf(stderr, "player: source %d: Unable to allocate audio codec context\n", source_id);
		goto exit_1;
	}



	// Copy variables to video_src
	memset(src, 0, sizeof(player_src));
	src->id = source_id;
	src->format_context = format_context;
	src->video_stream = videostream;
	src->audio_stream = audiostream;
	src->vcodec = video_codec;
	src->acodec = audio_codec;
	src->vcodec_ctx = video_codec_context;
	src->acodec_ctx = audio_codec_context;

	// All done. Print info and return
	fprintf(stderr, "player: source %d: Success!\n", source_id);
	return 0;

exit_1:
	avformat_close_input(&format_context);
exit_0:
	return retval;
}

void player_close_src(player_src *src) {
	fprintf(stderr, "player: source %d: Closed.\n", src->id);
	avformat_close_input((AVFormatContext**)&src->format_context);
	memset(src, 0, sizeof(player_src));
}

int player_play_src(player_stream *stream, player_src *src) {
	AVCodecContext *vcodec_ctx = (AVCodecContext*)src->vcodec_ctx;
	AVCodecContext *acodec_ctx = (AVCodecContext*)src->acodec_ctx;

	AVFrame *vframe = NULL;
	AVFrame *vframe_rgb = NULL;
	unsigned char *buffer = NULL;
	int bytes;
	int stream_id = gen_stream_id();

    vframe = av_frame_alloc();
    if(!vframe) {
    	fprintf(stderr, "player: stream %d: Unable to allocate vframe.\n", stream_id);
    	goto exit_0;
    }

    vframe_rgb = av_frame_alloc();
    if(!vframe_rgb) {
    	fprintf(stderr, "player: stream %d: Unable to allocate vframe_rgb.\n", stream_id);
    	goto exit_1;
    }

    bytes = avpicture_get_size(PIX_FMT_RGB32, vcodec_ctx->width, vcodec_ctx->height);
    buffer = av_malloc(bytes);
    if(!buffer) {
    	fprintf(stderr, "player: stream %d: Unable to allocate buffer.\n", stream_id);
    	goto exit_2;
    }
	avpicture_fill((AVPicture *)vframe_rgb, buffer, PIX_FMT_RGB32, vcodec_ctx->width, vcodec_ctx->height);

	// Audio converter (Any to S16, Stereo, 44100Hz)
	struct SwrContext *swr = swr_alloc_set_opts(
		NULL,
		AV_CH_LAYOUT_STEREO,
		AV_SAMPLE_FMT_S16,
		44100,
		acodec_ctx->channel_layout,
		acodec_ctx->sample_fmt,
		acodec_ctx->sample_rate,
		0, NULL);
	swr_init(swr);

	// Video converter (YUV -> RGBA8888)
	struct SwsContext *sws = sws_getCachedContext(
		NULL,
		vcodec_ctx->width,
		vcodec_ctx->height,
		vcodec_ctx->pix_fmt,
		vcodec_ctx->width,
		vcodec_ctx->height,
		PIX_FMT_RGB32,
		SWS_BICUBIC,
		NULL, NULL, NULL);

	// Fill in the struct
	memset(stream, 0, sizeof(player_stream));
	stream->id = stream_id;
	stream->vframe = vframe;
	stream->vframe_rgb = vframe_rgb;
	stream->state = STREAM_PLAYING;
	stream->src = src;
	stream->w = vcodec_ctx->width;
	stream->h = vcodec_ctx->height;
	stream->vconvert_ctx = sws;
	stream->aconvert_ctx = swr;
	stream->rb = rb_create(32768);

    fprintf(stderr, "player: stream %d: Playback started.\n", stream_id);
	return 0;

exit_2:
	av_frame_free(&vframe_rgb);
exit_1:
	av_frame_free(&vframe);
exit_0:
	return 1;
}

int player_stop_src(player_stream *stream) {
	fprintf(stderr, "player: stream %d: Playback stopped.\n", stream->id);
	av_frame_free((AVFrame **)&stream->vframe);
	av_frame_free((AVFrame **)&stream->vframe_rgb);
	sws_freeContext((struct SwsContext *)stream->vconvert_ctx);
	swr_free((struct SwrContext **)&stream->aconvert_ctx);
	rb_free((ringbuffer*)stream->rb);
	memset(stream, 0, sizeof(player_stream));
	return 0;
}

int player_stream_src(player_stream *stream) {
	int retcode = 1;
	int frame_finished;
	AVPacket avpacket;
	AVFormatContext *format_ctx = (AVFormatContext*)stream->src->format_context;
	AVCodecContext *vcodec_ctx = (AVCodecContext*)stream->src->vcodec_ctx;
	AVCodecContext *acodec_ctx = (AVCodecContext*)stream->src->acodec_ctx;

	// Attempt to read frame. Just return here if it fails.
	if(av_read_frame(format_ctx, &avpacket) < 0) {
		return 0;
	}

	// Handle video
	if(avpacket.stream_index == stream->src->video_stream) {
 		AVFrame *vframe = (AVFrame*)stream->vframe;

        avcodec_decode_video2(vcodec_ctx, vframe, &frame_finished, &avpacket);

        if(frame_finished) {
        	// Find a caches image scaler context (or create a new one)
 			stream->vconvert_ctx  = sws_getCachedContext(
 				(struct SwsContext *)stream->vconvert_ctx,
				vcodec_ctx->width,
				vcodec_ctx->height,
				vcodec_ctx->pix_fmt,
				vframe->width,
				vframe->height,
				PIX_FMT_RGB32,
				SWS_BICUBIC,
				NULL, NULL, NULL);

 			if(!stream->vconvert_ctx) {
 				fprintf(stderr, "Cannot initialize the conversion context!\n");
 				goto exit_1;
 			}

 			// Don't scale, but convert from whatever pixel format to RGB
 			sws_scale(
 				(struct SwsContext *)stream->vconvert_ctx,
 				(const uint8_t * const *)((AVPicture*)vframe)->data,
 				vframe->linesize,
 				0,
 				vcodec_ctx->height,
 				((AVPicture*)stream->vframe_rgb)->data,
 				((AVPicture*)stream->vframe_rgb)->linesize);

 			retcode = -1;
 		}
	}

	// Handle audio
	if(avpacket.stream_index == stream->src->audio_stream) {
		AVFrame *aframe = av_frame_alloc();
		int audio_pkt_size = avpacket.size;

		while(audio_pkt_size > 0) {
			int len = avcodec_decode_audio4(acodec_ctx, aframe, &frame_finished, &avpacket);
			if(len < 0) {
				break;
			}
			audio_pkt_size -= len;

			if(frame_finished) {
				unsigned char *buffer = malloc(32768);
				int m = swr_convert(
					(struct SwrContext *)stream->aconvert_ctx,
					&buffer,
					aframe->nb_samples,
					(const unsigned char **)aframe->extended_data,
					aframe->nb_samples);
				rb_write((ringbuffer*)stream->rb, (char*)buffer, m * 4);
				free(buffer);

				retcode = -2;
				break;
			}
		}

		av_frame_free(&aframe);
	}

	// Free and return.
exit_1:
	av_free_packet(&avpacket);
	return retcode;
}

unsigned char* player_stream_video_ptr(const player_stream *stream) {
	return ((AVPicture*)stream->vframe_rgb)->data[0];
}

int player_stream_data_len(const player_stream *stream) {
	return rb_length((ringbuffer*)stream->rb);
}

int player_stream_data_get(const player_stream *stream, unsigned char *data, int len) {
	return rb_read((ringbuffer*)stream->rb, (char*)data, len);
}
