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

typedef struct{
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

/***
 * init queue
 * @param pq
 * @return
 */
__declspec(dllexport) int packet_queue_init(PacketQueue **pq);

/***
 * destroy queue
 * @param pq
 */
__declspec(dllexport) void packet_queue_destroy(PacketQueue *pq);

/***
 *
 * @param pq
 * @param pkt
 * @param block
 * @return return 1 if success, 0 if !block && queue is empty,function while wait cond signal if block
 */
__declspec(dllexport) int packet_queue_get(PacketQueue *pq, AVPacket *pkt, int block);

/***
 *
 * @param pq
 * @param pkt
 * @return return 0 if success else < 0
 */
__declspec(dllexport) int packet_queue_put(PacketQueue *pq, AVPacket *pkt);

/***
 * clear queue
 * @param pq
 */
__declspec(dllexport) void packet_queue_clear(PacketQueue *pq);


#ifdef __cplusplus
}
#endif

#endif //FFMPEG_DEMO_PACKET_QUEUE_H
