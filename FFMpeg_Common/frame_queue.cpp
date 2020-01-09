//
// Created by qinrui on 2020/1/9.
//

#include "frame_queue.h"

int frame_queue_init(FrameQueue **q, unsigned int max_size) {
    FrameQueue * queue = static_cast<FrameQueue *>(av_malloc(sizeof(FrameQueue)));
    if(!queue){
        av_log(nullptr,AV_LOG_ERROR,"av_malloc failure\n");
        return -1;
    }

    memset(queue,0, sizeof(FrameQueue));
    queue->mutex = SDL_CreateMutex();
    if(!queue->mutex){
        av_log(nullptr,AV_LOG_ERROR,"SDL_CreateMutex failure! Error: %s\n",SDL_GetError());
        return -1;
    }

    queue->cond = SDL_CreateCond();
    if(!queue->cond){
        av_log(nullptr,AV_LOG_ERROR,"SDL_CreateCond failure! Error: %s\n",SDL_GetError());
        return -1;
    }

    queue->max_size = FFMIN(max_size,FRAME_QUEUE_MAX_SIZE);
    queue->r_idx = 0;
    queue->w_idx = 0;

    *q = queue;

    return 0;
}



