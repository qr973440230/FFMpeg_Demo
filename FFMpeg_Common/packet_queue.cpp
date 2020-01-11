//
// Created by qinrui on 2020/1/9.
//

#include "packet_queue.h"

int packet_queue_init(PacketQueue **q) {
    PacketQueue *pkt_queue = static_cast<PacketQueue *>(av_mallocz(sizeof(PacketQueue)));
    if (!pkt_queue) {
        av_log(nullptr, AV_LOG_ERROR, "av_malloc failure!\n");
        return -1;
    }

    pkt_queue->mutex = SDL_CreateMutex();
    if (!pkt_queue->mutex) {
        av_free(pkt_queue);
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateMutex failure! Error: %s\n", SDL_GetError());
        return -1;
    }
    pkt_queue->cond = SDL_CreateCond();
    if (!pkt_queue->cond) {
        SDL_DestroyMutex(pkt_queue->mutex);
        av_free(pkt_queue);
        av_log(nullptr, AV_LOG_ERROR, "SDL_CreateCond failure! Error: %s\n", SDL_GetError());
        return -1;
    }

    *q = pkt_queue;
    return 0;
}

void packet_queue_destroy(PacketQueue *q) {
    packet_queue_clear(q);
    SDL_DestroyMutex(q->mutex);
    SDL_DestroyCond(q->cond);
    av_free(q);
}

void packet_queue_abort(PacketQueue *q) {
    SDL_LockMutex(q->mutex);
    q->abort_request = 1;
    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);
}

int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;

    SDL_LockMutex(q->mutex);
    while (true) {
        if (q->abort_request) {
            ret = -1;
            break;
        }
        pkt1 = q->first_pkt;
        if (pkt1) {
            // has data
            q->first_pkt = pkt1->next;
            if (!q->first_pkt) {
                // last data
                q->last_pkt = nullptr;
            }
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
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
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);

    return ret;
}

int packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    if (q->abort_request) {
        return -1;
    }

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
    SDL_LockMutex(q->mutex);

    if (!q->last_pkt) {
        // isEmpty
        q->first_pkt = pkt1;
    } else {
        q->last_pkt->next = pkt1;
    }
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;

    SDL_CondSignal(q->cond);
    SDL_UnlockMutex(q->mutex);

    return 0;
}

int packet_queue_put_null_pkt(PacketQueue *q) {
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.size = 0;
    pkt.data = nullptr;
    return packet_queue_put(q, &pkt);
}


void packet_queue_clear(PacketQueue *q) {
    AVPacketList *pkt, *pkt_next;
    SDL_LockMutex(q->mutex);
    for (pkt = q->first_pkt; pkt; pkt = pkt_next) {
        pkt_next = pkt->next;
        av_packet_unref(&pkt->pkt);
        av_free(pkt);
    }

    q->first_pkt = nullptr;
    q->last_pkt = nullptr;
    q->nb_packets = 0;
    q->size = 0;
    SDL_UnlockMutex(q->mutex);
}



