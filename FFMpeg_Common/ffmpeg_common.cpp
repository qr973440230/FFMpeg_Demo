#include "ffmpeg_common.h"


int open_codec(AVFormatContext *fmt_ctx, enum AVMediaType type,
               AVCodecContext **codec_ctx, AVCodec **codec,
               AVStream **stream, int *stream_index) {
    int ret;
    int index;
    AVStream *st;
    AVCodec *cc;
    AVCodecContext *ctx;

    ret = av_find_best_stream(fmt_ctx, type, -1, -1, nullptr, 0);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_find_best_stream failure! Type: %s\n", av_get_media_type_string(type));
        return ret;
    }
    index = ret;
    st = fmt_ctx->streams[index];
    cc = avcodec_find_decoder(st->codecpar->codec_id);
    if (!cc) {
        av_log(nullptr, AV_LOG_ERROR, "avcodec_find_decoder failure! Codec Id: %d\n", st->codecpar->codec_id);
        return -1;
    }

    ctx = avcodec_alloc_context3(cc);
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

    ret = avcodec_open2(ctx, cc, nullptr);
    if (ret < 0) {
        avcodec_free_context(&ctx);
        av_log(nullptr, AV_LOG_ERROR, "avcodec_open2 failure!\n");
        return ret;
    }

    // open success
    if(codec_ctx){
        *codec_ctx = ctx;
    }
    if(codec){
        *codec = cc;
    }
    if(stream){
        *stream = st;
    }
    if(stream_index){
        *stream_index = index;
    }

    return 0;
}

