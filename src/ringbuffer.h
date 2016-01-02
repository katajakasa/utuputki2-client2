/*
 * Header file for generic ringbuffer in ringbuffer.c.
 *
 * Copyright (c) 2011, Tuomas Virtanen
 * license: GPLv2; see main.c for details.
*/

#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

typedef struct {
    int size;
    int len;
    int wpos, rpos;
    char* data;
} ringbuffer;


ringbuffer* rb_create(int size);
void rb_free(ringbuffer* rb);
int rb_write(ringbuffer *rb, const char* data, int len);
int rb_read(ringbuffer *rb, char* data, int len);
int rb_peek(const ringbuffer *rb, char* data, int len);
int rb_advance(ringbuffer *rb, int len);
int rb_length(const ringbuffer *rb);
int rb_size(const ringbuffer *rb);

#endif // __RINGBUFFER_H
