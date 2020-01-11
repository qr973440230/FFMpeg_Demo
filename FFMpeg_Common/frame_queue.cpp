//
// Created by qinrui on 2020/1/9.
//

#include "frame_queue.h"

int frame_queue_init(FrameQueue **q, unsigned int max_size, PacketQueue *pkt_q) {
    FrameQueue *queue = static_cast<FrameQueue *>(av_mallocz(sizeof(FrameQueue)));
    if (!queue) {
        av_log(nullptr, AV_LOG_ERROR, "av_malloc failure\n");
        return -1;
    }

    Frame *frames = static_cast<Frame *>(av_mallocz_array(max_size, sizeof(Frame)));
    if (!frames) {
        av_free(queue);
        av_log(nullptr, AV_LOG_ERROR, "av_malloc_array failure\n");
        return -1;
    }

    // 初始化Frames
    for (unsigned int i = 0; i < max_size; i++) {
        AVFrame *frame = av_frame_alloc();
        if (!frame) {
            for (unsigned int j = 0; j < i; j++) {
                av_frame_free(&frames[i].frame);
            }
            av_free(frames);
            av_free(queue);
            return -1;
        }
        frames[i].frame = frame;
    }
    queue->frames = frames;

    queue->mutex = SDL_CreateMutex();
    if (!queue->mutex) {
        av_free(frames);
        av_free(queue);
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateMutex failure! Error: %s\n", SDL_GetError());
        return -1;
    }

    queue->cond = SDL_CreateCond();
    if (!queue->cond) {
        av_free(frames);
        av_free(queue);
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateCond failure! Error: %s\n", SDL_GetError());
        return -1;
    }

    queue->pkt_queue = pkt_q;
    queue->max_size = max_size;
    queue->size = 0;
    queue->r_idx = 0;
    queue->w_idx = 0;
    *q = queue;

    return 0;
}

void frame_queue_destroy(FrameQueue *q) {
    for (unsigned int i = 0; i < q->max_size; i++) {
        Frame frame = q->frames[i];
        frame_queue_unref_item(&frame);
        av_frame_free(&frame.frame);
    }
    av_free(q->frames);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
    av_free(q);
}

void frame_queue_unref_item(Frame *frame) {
    av_frame_unref(frame->frame);
}

Frame *frame_queue_peek_writable(FrameQueue *q) {
    SDL_LockMutex(q->mutex);
    while (q->size >= q->max_size && !q->pkt_queue->abort_request) {
        // no can write data
        SDL_CondWait(q->cond, q->mutex);
    }
    SDL_UnlockMutex(q->mutex);

    if (q->pkt_queue->abort_request) {
        return nullptr;
    }

    return &q->frames[q->w_idx];
}

void frame_queue_push(FrameQueue *q) {
    if (++q->w_idx == q->max_size) {
        q->w_idx = 0;
    }
    SDL_LockMutex(q->mutex);
    q->size++;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

Frame *frame_queue_peek_readable(FrameQueue *q) {
    SDL_LockMutex(q->mutex);
    while (q->size <= 0 && !q->pkt_queue->abort_request) {
        // no can read data
        SDL_CondWait(q->cond, q->mutex);
    }
    SDL_UnlockMutex(q->mutex);

    if (q->pkt_queue->abort_request) {
        return nullptr;
    }

    return &q->frames[q->r_idx];
}

void frame_queue_next(FrameQueue *q) {
    frame_queue_unref_item(&q->frames[q->r_idx]);
    if (++q->r_idx == q->max_size) {
        q->r_idx = 0;
    }
    SDL_LockMutex(q->mutex);
    q->size--;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}






