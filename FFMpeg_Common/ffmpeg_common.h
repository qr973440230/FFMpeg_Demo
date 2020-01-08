
#ifndef __FFMPEG_COMMON_H__
#define __FFMPEG_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "SDL.h"

typedef struct AVPacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

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

/****
 * @param fmt_ctx
 * @param type
 * @param codec_ctx
 * @param stream_index
 * @return return 0 if success else < 0;
 */
__declspec(dllexport) int
open_codec(AVFormatContext *fmt_ctx, enum AVMediaType type, AVCodecContext **codec_ctx, int *stream_index);


#ifdef __cplusplus
}
#endif

#endif