#include "audio.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static ALCcontext *context = NULL;
static ALCdevice *device = NULL;

int audio_init() {
	fprintf(stderr, "Initializing audio subsystem ...\n");

	device = alcOpenDevice(NULL);
	if(!device) {
		fprintf(stderr, "audio: Unable to find an OpenAL device!\n");
		return 1;
	}

	context = alcCreateContext(device, NULL);
	if(!context) {
		fprintf(stderr, "audio: Unable to create an OpenAL context!\n");
		return 1;
	}

	alcMakeContextCurrent(context);
	fprintf(stderr, " * Using device '%s'\n", alcGetString(device, ALC_DEVICE_SPECIFIER));
	fprintf(stderr, " * OpenAL context created succesfully.\n");
	return 0;
}

void audio_close() {
	fprintf(stderr, "Closing audio subsystem.\n");
	device = alcGetContextsDevice(context);
	alcMakeContextCurrent(NULL);
	alcDestroyContext(context);
	alcCloseDevice(device);
}

int audio_init_stream(audio_stream *stream, int channels, int bytes) {
    // Dump old errors
    while(alGetError() != AL_NO_ERROR);

    memset(stream, 0, sizeof(audio_stream));

    // Pick format
    stream->format = -1;
    switch(bytes) {
        case 1: switch(channels) {
            case 1: stream->format = AL_FORMAT_MONO8; break;
            case 2: stream->format = AL_FORMAT_STEREO8; break;
        }; break;
        case 2: switch(channels) {
            case 1: stream->format = AL_FORMAT_MONO16; break;
            case 2: stream->format = AL_FORMAT_STEREO16; break;
        }; break;
    };
    if(stream->format == -1) {
        fprintf(stderr, "audio: Could not find suitable audio format!");
        goto exit_0;
    }

    // Generate buffers
    alGenBuffers(AUDIO_BUFFER_COUNT, stream->buffers);
    if(alGetError() != AL_NO_ERROR) {
        fprintf(stderr, "audio: Could not create audio buffers!");
        goto exit_1;
    }

    // Generate a source
    alGenSources(1, &stream->source);
    if(alGetError() != AL_NO_ERROR) {
        fprintf(stderr, "audio: Could not create audio source!");
        goto exit_0;
    }

    stream->starting = AUDIO_BUFFER_COUNT;
    stream->state = AUDIO_PLAYING;

    // All done.
    return 0;

exit_1:
    alDeleteSources(1, &stream->source);
exit_0:
    return 1;
}

int audio_update_stream(audio_stream *stream, unsigned char *data, unsigned int len, int freq) {
	if(len <= 0) {
		return 1;
	}

    // See if we have any empty buffers to fill
    int val;
    if(stream->starting == 0) {
	    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &val);
	    if(val <= 0) {
	        return 2;
	    }

	    // Dequeue, fill, requeue
	    ALuint n;
	    alSourceUnqueueBuffers(stream->source, 1, &n);
	    alBufferData(n, stream->format, data, len, freq);
	    alSourceQueueBuffers(stream->source, 1, &n);
    } else {
    	val = stream->starting;
	    alBufferData(stream->buffers[val-1], stream->format, data, len, freq);
	    stream->starting--;
	    if(stream->starting == 0) {
	    	alSourceQueueBuffers(stream->source, AUDIO_BUFFER_COUNT, stream->buffers);
	    }
    }

    // Check for any errors
    int err = alGetError();
    if(err != AL_NO_ERROR) {
        fprintf(stderr, "audio: Error %d while buffering!\n", err);
    }

    audio_render_stream(stream);
    return 0;
}

void audio_render_stream(audio_stream *stream) {
	if(stream->state == AUDIO_PLAYING && stream->starting == 0) {
	    ALenum state;
	    alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
	    if(state != AL_PLAYING) {
	        alSourcePlay(stream->source);
	        fprintf(stderr, "Forcing audio playback.\n");
	    }
    }
}

void audio_close_stream(audio_stream *stream) {
	stream->format = -1;
	stream->state = AUDIO_STOPPED;
	alSourceStop(stream->source);
	alDeleteSources(1, &stream->source);
	alDeleteBuffers(AUDIO_BUFFER_COUNT, stream->buffers);
}


