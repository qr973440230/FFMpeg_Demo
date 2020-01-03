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

static int open_codec_context(AVCodecContext** dec_ctx, 
	AVFormatContext* fmt_ctx, enum AVMediaType type)
{
	int ret;
	AVCodec* dec = NULL;
	AVDictionary* opts = NULL;

	ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
	if (ret < 0) {
		fprintf(stderr, "Could not find %s stream\n",av_get_media_type_string(type));
		return ret;
	} else {
		if (!dec) {
			fprintf(stderr, "Failed to find %s codec\n",
				av_get_media_type_string(type));
			return AVERROR(EINVAL);
		}

		*dec_ctx = avcodec_alloc_context3(dec);
		if (!*dec_ctx) {
			fprintf(stderr, "Failed to allocate the %s codec context\n",
				av_get_media_type_string(type));
			return AVERROR(ENOMEM);
		}

		av_dict_set(&opts, "refcounted_frames", "0", 0);
		if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) {
			fprintf(stderr, "Failed to open %s codec\n",
				av_get_media_type_string(type));
			return ret;
		}
	}

	return 0;
}

int main(int argc,char * argv[]) {

	// FFMpeg Init
	char* filePath = "test.mp4";

	AVFormatContext * fmt_ctx = NULL;
	int ret = avformat_open_input(&fmt_ctx, filePath, NULL,NULL);
	if (ret < 0) {
		printf("avformat_open_input failure! Error: %s \n",av_err2str(ret));
		return ret;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		printf("avformat_find_stream_info failure! Error: %s \n",av_err2str(ret));
		return ret;
	}

	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (ret < 0) {
		printf("av_find_best_stream failure Error: %s \n", av_err2str(ret));
		return ret;
	}

	int videoIndex = ret;
	AVStream* videoStream = fmt_ctx->streams[videoIndex];
	AVCodec* videoCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
	if (!videoCodec) {
		printf("avcodec_find_decoder failure \n");
		return -1;
	}

	AVCodecContext* videoCodecCtx = avcodec_alloc_context3(videoCodec);
	if (!videoCodecCtx) {
		printf("avcodec_alloc_context3 failure");
		return -1;
	}

	ret = avcodec_parameters_to_context(videoCodecCtx, videoStream->codecpar);
	if (ret < 0) {
		printf("avcodec_parameters_to_context failure! Error: %s\n", av_err2str(ret));
		return ret;
	}

	ret = avcodec_open2(videoCodecCtx, videoCodec, NULL);
	if(ret < 0){
		printf("avcodec_open2 failure! Error: %s\n", av_err2str(ret));
		return ret;
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		printf("av_frame_alloc failure\n");
		return -1;
	}

	AVFrame* scaleFrame = av_frame_alloc();
	if (!scaleFrame) {
		printf("av_frame_alloc failure\n");
		return -1;
	}
	scaleFrame->format = AV_PIX_FMT_YUV420P;
	scaleFrame->width = videoCodecCtx->width / 2;
	scaleFrame->height = videoCodecCtx->height / 2;
	ret = av_frame_get_buffer(scaleFrame, 32);
	if (ret < 0) {
		printf("av_frame_get_buffer failure! Error: %s\n", av_err2str(ret));
		return ret;
	}

	struct SwsContext* swsCtx = sws_getContext(videoCodecCtx->width, videoCodecCtx->height, 
		videoCodecCtx->pix_fmt,
		scaleFrame->width,scaleFrame->height,
		(enum AVPixelFormat)scaleFrame->format,
		SWS_BILINEAR,
		NULL, NULL, NULL);

	printf("--------File Infomation-----------\n");
	av_dump_format(fmt_ctx, 0, filePath, 0);
	printf("----------------------------------\n");

	// SDL Init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("SDL_Init Failure\n");
		return -1;
	}

	int screen_w = videoCodecCtx->width;
	int screen_h = videoCodecCtx->height;

	SDL_Window * window = SDL_CreateWindow("FFMpeg SDL Video", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		screen_w, screen_h, 0);
	if (window == NULL) {
		printf("SDL_CreateWindow Failure\n");
		return -1;
	}

	SDL_Renderer * render = SDL_CreateRenderer(window, -1, 0);
	SDL_Texture* texture = SDL_CreateTexture(render, 
		SDL_PIXELFORMAT_IYUV, 
		SDL_TEXTUREACCESS_STREAMING,
		screen_w,screen_h);

	// Loop

	SDL_Rect slr;
	slr.x = 0;
	slr.y = 0;
	slr.w = screen_w;
	slr.h = screen_h;
	while(av_read_frame(fmt_ctx,&pkt)>=0){
		if (pkt.stream_index == videoIndex) {
			ret = avcodec_send_packet(videoCodecCtx, &pkt);
			if (ret < 0) {
				printf("avcodec_send_packet failure! Error: %s\n",av_err2str(ret));
				break;
			}

			while (1) {
				ret = avcodec_receive_frame(videoCodecCtx, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				if (ret < 0) {
					printf("avcodec_receive_frame failure! Error: %s\n", av_err2str(ret));
					return -1;
				}

				frame->pts = frame->best_effort_timestamp;

				SDL_UpdateTexture(texture, NULL, frame->data[0], frame->linesize[0]);
				SDL_RenderClear(render);
				SDL_RenderCopy(render, texture, NULL, &slr);
				SDL_RenderPresent(render);
				SDL_Delay(40);

				av_frame_unref(frame);
			}
		}

		av_packet_unref(&pkt);
	}

	SDL_Quit();

	avcodec_free_context(&videoCodecCtx);
	avformat_close_input(&fmt_ctx);
	av_frame_free(&frame);

	return 0;
}