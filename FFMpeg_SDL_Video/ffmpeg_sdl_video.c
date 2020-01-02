#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL.h"

#ifdef __cplusplus
}
#endif

int main() {

	// FFMpeg Init
	char* filePath = "test.mp4";
	AVFormatContext * afc = avformat_alloc_context();
	int ret = avformat_open_input(&afc, filePath, NULL,NULL);
	if (ret < 0) {
		printf("avformat_open_input failure! Error: %s",av_err2str(ret));
		return ret;
	}

	ret = avformat_find_stream_info(afc, NULL);
	if (ret < 0) {
		printf("avformat_find_stream_info failure! Error: %s",av_err2str(ret));
		return ret;
	}

	int videoIndex = -1;

	for (int i = 0; i < afc->nb_streams; i++) {
		if (afc->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoIndex = i;
			break;
		}
	}

	if (videoIndex == -1) {
		printf("cannot find video stream");
		return -1;
	}

	AVCodecParameters* avp = afc->streams[videoIndex]->codecpar;
	AVCodec* avcodec = avcodec_find_decoder(avp->codec_id);
	if (!avcodec) {
		printf("cannot find codec");
		return -1;
	}

	AVCodecContext* acc = avcodec_alloc_context3(avcodec);
	if (!acc) {
		printf("cannot alloc context3");
		return -1;
	}

	ret = avcodec_open2(acc, avcodec, NULL);
	if (ret < 0) {
		printf("avcodec_open2 failure! Error: %s", av_err2str(ret));
		return ret;
	}

	AVFrame* frame = av_frame_alloc();
	AVFrame* tmpFrame = av_frame_alloc();
	AVPacket* packet = av_packet_alloc();
	if (!packet) {
		return 1;
	}

	printf("--------File Infomation-----------");
	av_dump_format(afc, 0, filePath, 0);
	printf("----------------------------------");

	struct SwsContext* swsCtx = sws_getContext(acc->width,acc->height,acc->pix_fmt,
		acc->width,acc->height,AV_PIX_FMT_YUV420P,
		SWS_BICUBIC,
		NULL,NULL,NULL);



	// SDL Init

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("SDL_Init Failure");
		return -1;
	}

	int screen_w = acc->width;
	int screen_h = acc->height;

	SDL_Window * window = SDL_CreateWindow("FFMpeg SDL Video", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		screen_w, screen_h, 
		0);
	if (window == NULL) {
		printf("SDL_CreateWindow Failure");
		return -1;
	}

	SDL_Renderer * render = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(render, 
		SDL_PIXELFORMAT_IYUV, 
		SDL_TEXTUREACCESS_STREAMING,
		screen_w,screen_h);

	SDL_Rect slr;
	slr.x = 0;
	slr.y = 0;
	slr.w = screen_w;
	slr.h = screen_h;

	// Loop

	int gotPicture;
	while (av_read_frame(afc, packet) >= 0) {
		if (packet->stream_index == videoIndex) {
			ret = avcodec_decode_video2(acc, frame, &gotPicture, packet);
			if (ret < 0) {
				break;
			}

			if (!gotPicture) {
				break;
			}

				sws_scale(swsCtx, (const unsigned char* const *)frame->data, frame->linesize, 0, acc->height, 
					tmpFrame->data, tmpFrame->linesize);

				SDL_UpdateTexture(texture, NULL, tmpFrame->data[0], tmpFrame->linesize[0]);
				SDL_RenderClear(render);
				SDL_RenderCopy(render, texture, NULL, &slr);
				SDL_RenderPresent(render);

				SDL_Delay(40);
		}
		av_free_packet(packet);
	}

	SDL_Quit();

	sws_freeContext(swsCtx);
	av_frame_free(&frame);
	av_frame_free(&tmpFrame);
	avcodec_close(acc);
	avformat_close_input(afc);

	return 0;
}