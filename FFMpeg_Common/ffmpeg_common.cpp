#include "ffmpeg_common.h"


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

