//
// Created by qinrui on 2020/1/9.
//

#ifndef FFMPEG_DEMO_VIDEO_PLAYER_H
#define FFMPEG_DEMO_VIDEO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
#include "ffmpeg_common.h"
#include "packet_queue.h"
#include "frame_queue.h"

typedef struct VideoPlayer_TAG {
    const char *file_path;
    AVFormatContext *in_fmt_ctx;
    AVCodecContext *video_codec_ctx;
    AVCodecContext *audio_codec_ctx;
    int video_stream_index;
    int audio_stream_index;
    int video_player_width;
    int video_player_height;
    AVPixelFormat video_player_fmt;
    SwsContext *video_sws_ctx;
    SwrContext *audio_swr_ctx;
    PacketQueue *video_pkt_queue;
    PacketQueue *audio_pkt_queue;
    FrameQueue *video_frame_queue;
    FrameQueue *audio_frame_queue;
} VideoPlayerContext;

int video_player_open(VideoPlayerContext **vp_ctx, const char *path, int width, int height, AVPixelFormat fmt);

void video_player_close(VideoPlayerContext *ctx);

#ifdef __cplusplus
}
#endif

#endif //FFMPEG_DEMO_VIDEO_PLAYER_H
