#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "SDL.h"
#include "ffmpeg_common.h"
#include "video_player.h"
#include "packet_queue.h"

#ifdef __cplusplus
}
#endif

#define REFRESH_VIDEO (SDL_USEREVENT + 1)

/***
 * decode Frame to FrameQueue
 * @param data
 * @return
 */
static int decode_video_worker(void *data) {
    int ret;
    AVPacket pkt;
    VideoPlayerContext *video_player_ctx;
    AVFrame *src_frame;

    video_player_ctx = static_cast<VideoPlayerContext *>(data);

    src_frame = av_frame_alloc();

    if (!src_frame) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_alloc failure \n");
        ret = -1;
        goto __end;
    }

    av_init_packet(&pkt);
    pkt.size = 0;
    pkt.data = nullptr;

    while (true) {
        ret = packet_queue_get(video_player_ctx->video_pkt_queue, &pkt, 1);
        if (ret < 0) {
            // abort
            break;
        }
        ret = avcodec_send_packet(video_player_ctx->video_codec_ctx, &pkt);
        if (ret < 0) {
            av_log(nullptr, AV_LOG_ERROR, "avcodec_send_packet failure\n");
            goto __end;
        }

        while (true) {
            ret = avcodec_receive_frame(video_player_ctx->video_codec_ctx, src_frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "avcodec_send_packet failure\n");
                goto __end;
            }

            Frame *f = frame_queue_peek_writable(video_player_ctx->video_frame_queue);
            if (!f) {
                // abort
                goto __end;
            }

            av_frame_move_ref(f->frame, src_frame);
            frame_queue_push(video_player_ctx->video_frame_queue);

            av_frame_unref(src_frame);
        }

        av_packet_unref(&pkt);
    }

    __end:
    if (src_frame) {
        av_frame_free(&src_frame);
    }

    return ret;
}

/***
 * decode packet to PacketQueue
 * @param data type is VideoPlayer
 * @return
 */
static int decode_worker(void *data) {
    AVPacket pkt;
    SDL_Thread *decode_video_thread = nullptr;
    VideoPlayerContext *video_player_ctx;

    video_player_ctx = static_cast<VideoPlayerContext *>(data);

    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    decode_video_thread = SDL_CreateThread(decode_video_worker, "decode_video_worker", video_player_ctx);

    while (av_read_frame(video_player_ctx->in_fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_player_ctx->video_stream_index) {
            packet_queue_put(video_player_ctx->video_pkt_queue, &pkt);
        } else if (pkt.stream_index == video_player_ctx->audio_stream_index) {
            packet_queue_put(video_player_ctx->audio_pkt_queue, &pkt);
        } else {
            av_packet_unref(&pkt);
        }
    }

    SDL_WaitThread(decode_video_thread, nullptr);

    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 0;
    char *file_path = "test.mp4";

    int width = 200;
    int height = 200;
    AVPixelFormat fmt = AV_PIX_FMT_YUV420P;

    SDL_Thread *decode_thread;

    // FFMpeg
    VideoPlayerContext *video_player_ctx = nullptr;
    ret = video_player_open(&video_player_ctx, file_path, width, height, fmt);

    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "ffmpeg_init_failure\n");
        return ret;
    }
    decode_thread = SDL_CreateThread(decode_worker, "decode_worker", video_player_ctx);

    // SDL
    SDL_Window *window = nullptr;
    SDL_Renderer *render = nullptr;
    SDL_Texture *texture = nullptr;

    // SDL Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }

    window = SDL_CreateWindow("FFMpeg SDL Video",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_RESIZABLE);
    if (!window) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }

    render = SDL_CreateRenderer(window, -1, 0);
    if (!render) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateRenderer failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }
    texture = SDL_CreateTexture(render,
                                SDL_PIXELFORMAT_IYUV,
                                SDL_TEXTUREACCESS_STREAMING,
                                width,
                                height);

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }

    // Loop
    SDL_Rect slr;
    slr.x = 0;
    slr.y = 0;
    slr.w = width;
    slr.h = height;


    SDL_CreateThread([](void *) -> int {
        while (true) {
            SDL_Event eve;
            eve.type = REFRESH_VIDEO;
            SDL_PushEvent(&eve);

            SDL_Delay(40);
        }
        return 0;
    }, "refresh_video", nullptr);

    AVFrame *scale_frame = av_frame_alloc();
    scale_frame->width = video_player_ctx->video_player_width;
    scale_frame->height = video_player_ctx->video_player_height;
    scale_frame->format = video_player_ctx->video_player_fmt;
    ret = av_frame_get_buffer(scale_frame, 32);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_get_buffer failure \n");
        goto ___end;
    }

    SDL_Event sle;
    while (true) {
        SDL_WaitEvent(&sle);
        if (sle.type == SDL_QUIT) {
            packet_queue_abort(video_player_ctx->video_pkt_queue);
            break;
        } else if (sle.type == REFRESH_VIDEO) {
            Frame *f = frame_queue_peek_readable(video_player_ctx->video_frame_queue);

            AVFrame *frame = f->frame;

            sws_scale(video_player_ctx->video_sws_ctx,
                      (const uint8_t *const *) frame->data, frame->linesize,
                      0, video_player_ctx->video_codec_ctx->height,
                      scale_frame->data, scale_frame->linesize);


            SDL_UpdateYUVTexture(texture, nullptr,
                                 scale_frame->data[0], scale_frame->linesize[0],
                                 scale_frame->data[1], scale_frame->linesize[1],
                                 scale_frame->data[2], scale_frame->linesize[2]);

            SDL_RenderClear(render);
            SDL_RenderCopy(render, texture, nullptr, &slr);
            SDL_RenderPresent(render);


            frame_queue_next(video_player_ctx->video_frame_queue);
        }
    }

    SDL_WaitThread(decode_thread, nullptr);

    ___end:
    // clean sdl2
    if (window) {
        SDL_DestroyWindow(window);
    }
    if (render) {
        SDL_DestroyRenderer(render);
    }
    if (texture) {
        SDL_DestroyTexture(texture);
    }
    SDL_Quit();

    // clean ffmpeg
    if (video_player_ctx) {
        video_player_close(video_player_ctx);
    }

    return ret;
}