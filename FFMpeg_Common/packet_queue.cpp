//
// Created by qinrui on 2020/1/9.
//

#include "packet_queue.h"

int packet_queue_init(PacketQueue **pq) {
    PacketQueue *pkt_queue = static_cast<PacketQueue *>(av_malloc(sizeof(PacketQueue)));
    if (!pkt_queue) {
        av_log(nullptr, AV_LOG_ERROR, "av_malloc failure!\n");
        return -1;
    }
    memset(pkt_queue, 0, sizeof(PacketQueue));

    pkt_queue->mutex = SDL_CreateMutex();
    if (!pkt_queue->mutex) {
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateMutex failure! Error: %s\n", SDL_GetError());
        return -1;
    }
    pkt_queue->cond = SDL_CreateCond();
    if (!pkt_queue->cond) {
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateCond failure! Error: %s\n", SDL_GetError());
        return -1;
    }

    *pq = pkt_queue;
    return 0;
}

void packet_queue_destroy(PacketQueue *pq) {
    packet_queue_clear(pq);
    SDL_DestroyMutex(pq->mutex);
    SDL_DestroyCond(pq->cond);
    av_free(pq);
}

int packet_queue_get(PacketQueue *pq, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(pq->mutex);
    while (true) {
        pkt1 = pq->first_pkt;
        if (pkt1) {
            // has data
            pq->first_pkt = pkt1->next;
            if (!pq->first_pkt) {
                // last data
                pq->last_pkt = nullptr;
            }
            pq->nb_packets--;
            pq->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;

            // release pkt1
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            // no block
            ret = 0;
            break;
        } else {
            // wait
            SDL_CondWait(pq->cond, pq->mutex);
        }
    }
    SDL_UnlockMutex(pq->mutex);

    return ret;
}

int packet_queue_put(PacketQueue *pq, AVPacket *pkt) {
    AVPacketList *pkt1;
    if (av_packet_make_refcounted(pkt) < 0) {
        return -1;
    }

    pkt1 = (AVPacketList *) av_malloc(sizeof(AVPacketList));
    if (!pkt1) {
        return -1;
    }

    pkt1->next = nullptr;
    pkt1->pkt = *pkt;

    // lock
    SDL_LockMutex(pq->mutex);

    if (!pq->last_pkt) {
        // isEmpty
        pq->first_pkt = pkt1;
    } else {
        pq->last_pkt->next = pkt1;
    }
    pq->last_pkt = pkt1;
    pq->nb_packets++;
    pq->size += pkt1->pkt.size;

    SDL_CondSignal(pq->cond);
    SDL_UnlockMutex(pq->mutex);

    return 0;
}

void packet_queue_clear(PacketQueue *pq) {
    AVPacketList *pkt, *pkt_next;
    SDL_LockMutex(pq->mutex);
    for (pkt = pq->first_pkt; pkt; pkt = pkt_next) {
        pkt_next = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_free(pkt);
    }

    pq->first_pkt = nullptr;
    pq->last_pkt = nullptr;
    pq->nb_packets = 0;
    pq->size = 0;
    SDL_UnlockMutex(pq->mutex);
}


