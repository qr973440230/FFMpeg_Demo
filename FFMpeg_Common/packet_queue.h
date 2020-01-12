//
// Created by qinrui on 2020/1/9.
//

#ifndef FFMPEG_DEMO_PACKET_QUEUE_H
#define FFMPEG_DEMO_PACKET_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "SDL.h"

typedef struct PacketQueue{
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    int abort_request;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

/***
 * init queue
 * @param q
 * @return
 */
__declspec(dllexport) int packet_queue_init(PacketQueue **q);

/***
 * destroy queue
 * @param q
 */
__declspec(dllexport) void packet_queue_destroy(PacketQueue *q);

/***
 * abort queue
 * @param q
 */
__declspec(dllexport) void packet_queue_abort(PacketQueue *q);

/***
 *
 * @param q
 * @param pkt
 * @param block
 * @return return 1 if success, 0 if !block && queue is empty,function while wait cond signal if block
 */
__declspec(dllexport) int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);

/***
 *
 * @param q
 * @param pkt
 * @return return 0 if success else < 0
 */
__declspec(dllexport) int packet_queue_put(PacketQueue *q, AVPacket *pkt);

__declspec(dllexport) int packet_queue_put_null_pkt(PacketQueue *q, int stream_index);
/***
 * clear queue
 * @param q
 */
__declspec(dllexport) void packet_queue_clear(PacketQueue *q);


#ifdef __cplusplus
}
#endif

#endif //FFMPEG_DEMO_PACKET_QUEUE_H
