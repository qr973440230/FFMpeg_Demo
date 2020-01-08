#include "ffmpeg_common.h"

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

int open_codec(AVFormatContext *fmt_ctx, enum AVMediaType type, AVCodecContext **codec_ctx, int *stream_index) {
    int ret;
    int index;
    ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_find_best_stream failure! Type: %s\n", av_get_media_type_string(type));
        return ret;
    }
    index = ret;
    AVStream *st = fmt_ctx->streams[index];
    AVCodec *codec = avcodec_find_decoder(st->codecpar->codec_id);
    if (!codec) {
        av_log(nullptr, AV_LOG_ERROR, "avcodec_find_decoder failure! Codec Id: %d\n", st->codecpar->codec_id);
        return -1;
    }

    AVCodecContext *ctx = avcodec_alloc_context3(codec);
    if (!ctx) {
        av_log(nullptr, AV_LOG_ERROR, "avcodec_alloc_context3 failure!\n");
        return -1;
    }

    ret = avcodec_parameters_to_context(ctx, st->codecpar);
    if (ret < 0) {
        avcodec_free_context(&ctx);
        av_log(nullptr, AV_LOG_ERROR, "avcodec_parameters_to_context failure!\n");
        return ret;
    }

    ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0) {
        avcodec_free_context(&ctx);
        av_log(nullptr, AV_LOG_ERROR, "avcodec_open2 failure!\n");
        return ret;
    }

    // open success
    *codec_ctx = ctx;
    *stream_index = index;

    return 0;
}