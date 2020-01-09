//
// Created by qinrui on 2020/1/9.
//

#include "video_player.h"

int video_player_open(VideoPlayerContext **vp_ctx, const char *path, int width, int height, AVPixelFormat fmt) {
    int ret;

    VideoPlayerContext *video_player_ctx = static_cast<VideoPlayerContext *>(av_malloc(sizeof(VideoPlayerContext)));
    if (!video_player_ctx) {
        av_log(nullptr, AV_LOG_ERROR, "av_malloc failure!\n");
        return -1;
    }
    video_player_ctx->file_path = path;

    video_player_ctx->in_fmt_ctx = nullptr;
    ret = avformat_open_input(&video_player_ctx->in_fmt_ctx, video_player_ctx->file_path, nullptr, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_open_input failure\n");
        return ret;
    }

    ret = avformat_find_stream_info(video_player_ctx->in_fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_find_stream_info failure\n");
        return ret;
    }

    av_dump_format(video_player_ctx->in_fmt_ctx, 0, video_player_ctx->file_path, 0);

    video_player_ctx->video_codec_ctx = nullptr;
    video_player_ctx->video_stream_index = -1;
    ret = open_codec(video_player_ctx->in_fmt_ctx,
                     AVMEDIA_TYPE_VIDEO,
                     &video_player_ctx->video_codec_ctx,
                     &video_player_ctx->video_stream_index);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_INFO, "no video stream\n");
    }

    video_player_ctx->audio_codec_ctx = nullptr;
    video_player_ctx->audio_stream_index = -1;
    ret = open_codec(video_player_ctx->in_fmt_ctx,
                     AVMEDIA_TYPE_AUDIO,
                     &video_player_ctx->audio_codec_ctx,
                     &video_player_ctx->audio_stream_index);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_INFO, "no audio stream\n");
    }

    if (video_player_ctx->video_stream_index == -1 && video_player_ctx->audio_stream_index == -1) {
        av_log(nullptr, AV_LOG_ERROR, "no audio stream && no video stream");
        return -1;
    }

    if (video_player_ctx->video_stream_index != -1) {
        if (width <= 0 || height <= 0) {
            video_player_ctx->video_player_width = video_player_ctx->video_codec_ctx->width;
            video_player_ctx->video_player_height = video_player_ctx->video_codec_ctx->height;
        } else {
            video_player_ctx->video_player_width = width;
            video_player_ctx->video_player_height = height;
        }

        video_player_ctx->video_player_fmt = fmt;
        video_player_ctx->video_sws_ctx = sws_getContext(video_player_ctx->video_codec_ctx->width,
                                                         video_player_ctx->video_codec_ctx->height,
                                                         video_player_ctx->video_codec_ctx->pix_fmt,
                                                         video_player_ctx->video_player_width,
                                                         video_player_ctx->video_player_height,
                                                         video_player_ctx->video_player_fmt,
                                                         SWS_BILINEAR,
                                                         nullptr, nullptr, nullptr);
        video_player_ctx->video_pkt_queue = nullptr;
        packet_queue_init(&video_player_ctx->video_pkt_queue);
        if(!video_player_ctx->video_pkt_queue){
            av_log(nullptr,AV_LOG_ERROR,"av_malloc failure\n");
            return -1;
        }
    }

    if (video_player_ctx->audio_stream_index != -1) {
        uint64_t in_channel_layout = av_get_default_channel_layout(video_player_ctx->audio_codec_ctx->channels);
        uint64_t out_channel_layout = in_channel_layout;
        video_player_ctx->audio_swr_ctx = swr_alloc();
        if (video_player_ctx->audio_swr_ctx) {
            swr_alloc_set_opts(video_player_ctx->audio_swr_ctx,
                               out_channel_layout,
                               AV_SAMPLE_FMT_S16,
                               video_player_ctx->audio_codec_ctx->sample_rate,
                               in_channel_layout,
                               video_player_ctx->audio_codec_ctx->sample_fmt,
                               video_player_ctx->audio_codec_ctx->sample_rate,
                               0,
                               nullptr);
            swr_init(video_player_ctx->audio_swr_ctx);
        }

        video_player_ctx->audio_pkt_queue = nullptr;
        packet_queue_init(&video_player_ctx->audio_pkt_queue);
        if(!video_player_ctx->audio_pkt_queue){
            av_log(nullptr,AV_LOG_ERROR,"av_malloc failure\n");
            return -1;
        }
    }

    *vp_ctx = video_player_ctx;
    return 0;
}

void video_player_close(VideoPlayerContext *ctx) {
    if(ctx->video_pkt_queue){
        packet_queue_destroy(ctx->video_pkt_queue);
    }
    if(ctx->video_sws_ctx){
        sws_freeContext(ctx->video_sws_ctx);
    }
    if(ctx->video_codec_ctx){
        avcodec_free_context(&ctx->video_codec_ctx);
    }

    if(ctx->audio_pkt_queue){
        packet_queue_destroy(ctx->audio_pkt_queue);
    }
    if(ctx->audio_swr_ctx){
        swr_free(&ctx->audio_swr_ctx);
    }
    if(ctx->audio_codec_ctx){
        avcodec_free_context(&ctx->audio_codec_ctx);
    }
    if(ctx->in_fmt_ctx){
        avformat_free_context(ctx->in_fmt_ctx);
    }

    av_free(ctx);
}
