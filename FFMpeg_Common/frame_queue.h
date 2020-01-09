//
// Created by qinrui on 2020/1/9.
//

#ifndef FFMPEG_DEMO_FRAME_QUEUE_H
#define FFMPEG_DEMO_FRAME_QUEUE_H

#include <SDL_mutex.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "libavformat/avformat.h"
#include "packet_queue.h"

#define FRAME_QUEUE_MAX_SIZE 24

typedef struct {
    AVFrame *frame;
} Frame;

typedef struct {
    Frame queue[FRAME_QUEUE_MAX_SIZE];
    int r_idx;
    int w_idx;
    int size;
    int max_size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} FrameQueue;

int frame_queue_init(FrameQueue **q, unsigned int max_size);


#ifdef __cplusplus
}
#endif

#endif //FFMPEG_DEMO_FRAME_QUEUE_H
