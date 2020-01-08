
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "ffmpeg_common.h"

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[]) {
    av_log_set_level(AV_LOG_INFO);

    AVFormatContext *fmt_ctx = nullptr;
    int ret;

    ret = avformat_open_input(&fmt_ctx, "./test.mp4", nullptr, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_open_input failure!\n");
        return ret;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_find_stream_info failure!\n");
        avformat_close_input(&fmt_ctx);
        return ret;
    }

    av_dump_format(fmt_ctx, 0, "./test.mp4", 0);

    AVCodecContext *video_codec_ctx = nullptr;
    int video_stream_index = -1;

    ret = open_codec(fmt_ctx, AVMEDIA_TYPE_VIDEO, &video_codec_ctx, &video_stream_index);
    if (ret < 0) {
        avformat_close_input(&fmt_ctx);
        return ret;
    }

    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_alloc failure\n");
        avcodec_free_context(&video_codec_ctx);
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_index) {
            // ½âÂë
            ret = avcodec_send_packet(video_codec_ctx, &pkt);
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "avcodec_send_packet failure!\n");
                break;
            }

            do {
                ret = avcodec_receive_frame(video_codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }
                if (ret < 0) {
                    printf("avcodec_receive_frame failure!\n");
                    av_frame_free(&frame);
                    avcodec_free_context(&video_codec_ctx);
                    avformat_close_input(&fmt_ctx);
                    return -1;
                }

                // ½âÂëVideo

            } while (true);

        }
        av_packet_unref(&pkt);
    }

    av_frame_free(&frame);
    avcodec_free_context(&video_codec_ctx);
    avformat_close_input(&fmt_ctx);
    return 0;
}