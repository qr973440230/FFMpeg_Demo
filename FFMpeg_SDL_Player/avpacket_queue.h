#ifndef __AV_PACKET_QUEUE_H__
#define __AV_PACKET_QUEUE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "SDL.h"

typedef struct PacketQueue{
	AVPacketList* first_pkt, * last_pkt;
	int nb_packets;
	int size;
	SDL_mutex* mutex;
	SDL_cond* cond;
} PacketQueue;

int packet_queue_get(PacketQueue* pq, AVPacket* pkt, int block);

int packet_queue_put(PacketQueue* pq, AVPacket* pkt);

#ifdef __cplusplus
}
#endif

#endif