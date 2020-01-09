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
 * 解码线程
 * @param data type is VideoPlayer
 * @return
 */
static int decode_worker(void *data) {
    AVPacket pkt;
    VideoPlayerContext *video_player_ctx = static_cast<VideoPlayerContext *>(data);

    av_init_packet(&pkt);
    pkt.data = nullptr;
    pkt.size = 0;

    while (av_read_frame(video_player_ctx->in_fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_player_ctx->video_stream_index) {
            packet_queue_put(video_player_ctx->video_pkt_queue, &pkt);
        } else if (pkt.stream_index == video_player_ctx->audio_stream_index) {
            packet_queue_put(video_player_ctx->audio_pkt_queue, &pkt);
        } else {
            av_packet_unref(&pkt);
        }
    }

    return 0;
}

int main(int argc, char *argv[]) {
    int ret = 0;
    char *file_path = "test.mp4";

    int width = 640;
    int height = 480;
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
                              width, height, 0);
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

    SDL_Event sle;
    while (true) {
        SDL_WaitEvent(&sle);
        if (sle.type == SDL_QUIT) {
            break;
        } else if (sle.type == REFRESH_VIDEO) {

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
    if (video_player_ctx) {
        video_player_close(video_player_ctx);
    }

    return ret;
}