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


typedef struct {
    AVFrame *frame;
} Frame;

typedef struct {
    Frame *frames;
    PacketQueue * pkt_queue;
    unsigned int r_idx;
    unsigned int w_idx;
    unsigned int size;
    unsigned int max_size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} FrameQueue;

/***
 * init queue see
 * @param q
 * @param max_size
 * @return
 */
__declspec(dllexport) int frame_queue_init(FrameQueue **q, unsigned int max_size,PacketQueue * pkt_q);

/***
 * destroy queue
 * @param q
 */
__declspec(dllexport) void frame_queue_destroy(FrameQueue *q);

/***
 * abandon frame
 * @param frame
 */
__declspec(dllexport) void frame_queue_unref_item(Frame *frame);

/***
 * frame_queue_peek_writable(q);
 * …………
 * frame_queue_push(q);
 * @param q
 * @return
 */
__declspec(dllexport) Frame *frame_queue_peek_writable(FrameQueue *q);

/***
 * call after frame_queue_peek_writable
 * @param q
 */
__declspec(dllexport) void frame_queue_push(FrameQueue *q);

/***
 * frame_queue_peek_readable(q);
 * …………
 * frame_queue_next(q);
 * @param q
 * @return
 */
__declspec(dllexport) Frame *frame_queue_peek_readable(FrameQueue *q);

/***
 * call after frame_queue_peek_readable, frame pointer move to next
 * @param q
 */
__declspec(dllexport) void frame_queue_next(FrameQueue *q);

#ifdef __cplusplus
}
#endif

#endif //FFMPEG_DEMO_FRAME_QUEUE_H
