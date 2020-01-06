#include <stdio.h>
#include "avpacket_queue.h"

int packet_queue_get(PacketQueue* pq, AVPacket* pkt, int block) 
{
	AVPacketList* pkt1;
	int ret;

	SDL_LockMutex(pq->mutex);
	while (1) {
		pkt1 = pq->first_pkt;
		if (pkt1) {
			// has data
			pq->first_pkt = pkt1->next;
			if (!pq->first_pkt) {
				// last data
				pq->last_pkt = NULL;
			}
			pq->nb_packets--;
			pq->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;

			// release pkt1
			av_free(pkt1);
			ret = 1;
			break;
		}
		else if (!block) {
			// no block
			ret = 0;
			break;
		}
		else {
			// wait
			SDL_CondWait(pq->cond, pq->mutex);
		}
	}
	SDL_UnlockMutex(pq->mutex);

	return ret;
}

int packet_queue_put(PacketQueue* pq, AVPacket* pkt) 
{
	AVPacketList* pkt1;
	if (av_packet_make_refcounted(pkt) < 0) {
		return -1;
	}

	pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
	if (!pkt1) {
		return -1;
	}

	pkt1->next = NULL;
	pkt1->pkt = *pkt;

	// 加锁
	SDL_LockMutex(pq->mutex);

	if (!pq->last_pkt) {
		// 列表为空
		pq->first_pkt = pkt1;
	}
	else {
		pq->last_pkt->next = pkt1;
	}
	pq->last_pkt = pkt1;
	pq->nb_packets++;
	pq->size += pkt1->pkt.size;

	SDL_CondSignal(pq->cond);
	SDL_UnlockMutex(pq->mutex);

	return 0;
}
