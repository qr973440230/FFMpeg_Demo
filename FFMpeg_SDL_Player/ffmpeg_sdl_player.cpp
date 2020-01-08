#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL.h"
#include "ffmpeg_common.h"

#ifdef __cplusplus
}
#endif

#define REFRESH_EVENT (SDL_USEREVENT + 1)

static bool isQuit = false;
static int refresh_video(void * data){
    while(!isQuit){
        SDL_Event event;
        event.type = REFRESH_EVENT;

        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
}


int main(int argc, char *argv[]) {
    int ret = 0;
    char *filePath = "test.mp4";

    // FFMpeg
    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *video_codec_ctx = nullptr;
    int video_index = -1;
    AVPacket pkt;
    AVFrame *frame = nullptr;
    AVFrame *scale_frame = nullptr;
    SwsContext *video_sws_ctx = nullptr;

    // SDL
    int screen_w;
    int screen_h;
    SDL_Window *window = nullptr;
    SDL_Renderer *render = nullptr;
    SDL_Texture *texture = nullptr;
    SDL_Thread * refresh_thread = nullptr;

    av_log_set_level(AV_LOG_INFO);
    ret = avformat_open_input(&fmt_ctx, filePath, nullptr, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_open_input failure");
        goto ___end;
    }

    ret = avformat_find_stream_info(fmt_ctx, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_find_stream_info failure");
        goto ___end;
    }

    av_dump_format(fmt_ctx, 0, filePath, 0);

    ret = open_codec(fmt_ctx, AVMEDIA_TYPE_VIDEO, &video_codec_ctx, &video_index);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "open_codec failure");
        goto ___end;
    }

    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    frame = av_frame_alloc();
    if (!frame) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_alloc failure");
        ret = -1;
        goto ___end;
    }

    scale_frame = av_frame_alloc();
    if (!scale_frame) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_alloc failure");
        ret = -1;
        goto ___end;
    }

    scale_frame->format = AV_PIX_FMT_YUV420P;
    scale_frame->width = video_codec_ctx->width / 2;
    scale_frame->height = video_codec_ctx->height / 2;
    ret = av_frame_get_buffer(scale_frame, 32);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "av_frame_get_buffer failure");
        goto ___end;
    }

    video_sws_ctx = sws_getContext(video_codec_ctx->width, video_codec_ctx->height,
                                   video_codec_ctx->pix_fmt,
                                   scale_frame->width, scale_frame->height,
                                   static_cast<AVPixelFormat>(scale_frame->format),
                                   SWS_BILINEAR,
                                   nullptr, nullptr, nullptr);

    if (!video_sws_ctx) {
        av_log(nullptr, AV_LOG_ERROR, "sws_getContext failure");
        ret = -1;
        goto ___end;
    }

    // SDL Init
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }

    screen_w = scale_frame->width;
    screen_h = scale_frame->height;

    window = SDL_CreateWindow("FFMpeg SDL Video",
                              SDL_WINDOWPOS_CENTERED,
                              SDL_WINDOWPOS_CENTERED,
                              screen_w, screen_h, 0);
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
                                screen_w,
                                screen_h);

    if (!texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateTexture failure! Error: %s\n", SDL_GetError());
        ret = -1;
        goto ___end;
    }

    // Loop
    SDL_Rect slr;
    slr.x = 0;
    slr.y = 0;
    slr.w = screen_w;
    slr.h = screen_h;

    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_index) {
            ret = avcodec_send_packet(video_codec_ctx, &pkt);
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "avcodec_send_packet failure \n");
                goto ___end;
            }

            while (true) {
                ret = avcodec_receive_frame(video_codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }

                if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "avcodec_receive_frame failure!\n");
                    goto ___end;
                }

                frame->pts = frame->best_effort_timestamp;

                ret = av_frame_make_writable(scale_frame);
                if (ret < 0) {
                    av_log(nullptr, AV_LOG_ERROR, "av_frame_make_writable failure!\n");
                    goto ___end;
                }

                sws_scale(video_sws_ctx,
                          (const uint8_t *const *) frame->data, frame->linesize,
                          0, video_codec_ctx->height,
                          scale_frame->data, scale_frame->linesize);


                SDL_UpdateYUVTexture(texture, nullptr,
                                     scale_frame->data[0], scale_frame->linesize[0],
                                     scale_frame->data[1], scale_frame->linesize[1],
                                     scale_frame->data[2], scale_frame->linesize[2]);

                SDL_RenderClear(render);
                SDL_RenderCopy(render, texture, nullptr, &slr);
                SDL_RenderPresent(render);
                SDL_Delay(40);

                av_frame_unref(frame);
            }
        }

        av_packet_unref(&pkt);
    }

    SDL_Event sle;
    while (true) {
        SDL_WaitEvent(&sle);
        if(sle.type == REFRESH_EVENT){
            ret = avcodec_receive_frame(video_codec_ctx, frame);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                break;
            }

            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "avcodec_receive_frame failure!\n");
                goto ___end;
            }

            frame->pts = frame->best_effort_timestamp;

            ret = av_frame_make_writable(scale_frame);
            if (ret < 0) {
                av_log(nullptr, AV_LOG_ERROR, "av_frame_make_writable failure!\n");
                goto ___end;
            }

            sws_scale(video_sws_ctx,
                      (const uint8_t *const *) frame->data, frame->linesize,
                      0, video_codec_ctx->height,
                      scale_frame->data, scale_frame->linesize);


            SDL_UpdateYUVTexture(texture, nullptr,
                                 scale_frame->data[0], scale_frame->linesize[0],
                                 scale_frame->data[1], scale_frame->linesize[1],
                                 scale_frame->data[2], scale_frame->linesize[2]);

            SDL_RenderClear(render);
            SDL_RenderCopy(render, texture, nullptr, &slr);
            SDL_RenderPresent(render);

            av_frame_unref(frame);
        }else if(sle.type == SDL_QUIT){
            isQuit = true;
            break;
        }
    }

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
    if (video_sws_ctx) {
        sws_freeContext(video_sws_ctx);
    }
    if (frame) {
        av_frame_free(&frame);
    }
    if (scale_frame) {
        av_frame_free(&scale_frame);
    }
    if (video_codec_ctx) {
        avcodec_free_context(&video_codec_ctx);
    }
    if (fmt_ctx) {
        avformat_close_input(&fmt_ctx);
    }

    return ret;
}