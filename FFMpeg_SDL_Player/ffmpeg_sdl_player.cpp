#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "SDL.h"
#include "avpacket_queue.h"
#include "video_demux.h"

#ifdef __cplusplus
}
#endif


int main(int argc,char * argv[]) {

	// FFMpeg Init
	char* filePath = "test.mp4";

	AVFormatContext * fmt_ctx = NULL;
	int ret = avformat_open_input(&fmt_ctx, filePath, NULL,NULL);
	if (ret < 0) {
		printf("avformat_open_input failure!\n");
		return ret;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		printf("avformat_find_stream_info failure!\n");
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	printf("--------File Infomation-----------\n");
	av_dump_format(fmt_ctx, 0, filePath, 0);
	printf("----------------------------------\n");

	int video_index = -1;
	AVCodecContext* video_codec_ctx;
	ret = open_codec(fmt_ctx,AVMEDIA_TYPE_VIDEO,&video_codec_ctx,&video_index);
	if (ret < 0) {
		printf("open video codec failure!\n");
		avformat_close_input(&fmt_ctx);
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

	AVFrame* scale_frame = av_frame_alloc();
	if (!scale_frame) {
		printf("av_frame_alloc failure\n");
		return -1;
	}

	scale_frame->format = AV_PIX_FMT_YUV420P;
	scale_frame->width = video_codec_ctx->width / 2;
	scale_frame->height = video_codec_ctx->height / 2;
	ret = av_frame_get_buffer(scale_frame, 32);
	if (ret < 0) {
		printf("av_frame_get_buffer failure!\n");
		return ret;
	}

	struct SwsContext* video_sws_ctx = sws_getContext(video_codec_ctx->width, video_codec_ctx->height,
		video_codec_ctx->pix_fmt,
		scale_frame->width, scale_frame->height,
		(enum AVPixelFormat)scale_frame->format,
		SWS_BILINEAR,
		NULL, NULL, NULL);

	// SDL Init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("SDL_Init Failure\n");
		return -1;
	}

	int screen_w = scale_frame->width;
	int screen_h = scale_frame->height;

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
		screen_w,
		screen_h);

	// Loop
	SDL_Rect slr;
	slr.x = 0;
	slr.y = 0;
	slr.w = screen_w;
	slr.h = screen_h;

	while(av_read_frame(fmt_ctx,&pkt)>=0){
		if (pkt.stream_index == video_index) {
			ret = avcodec_send_packet(video_codec_ctx, &pkt);
			if (ret < 0) {
				printf("avcodec_send_packet failure!\n");
				break;
			}

			while (1) {
				ret = avcodec_receive_frame(video_codec_ctx, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				if (ret < 0) {
					printf("avcodec_receive_frame failure!\n");
					sws_freeContext(video_sws_ctx);
					av_frame_free(&frame);
					av_frame_free(&scale_frame);
					avcodec_free_context(&video_codec_ctx);
					avformat_close_input(&fmt_ctx);
					return -1;
				}

				frame->pts = frame->best_effort_timestamp;

				ret = av_frame_make_writable(scale_frame);
				if (ret < 0) {
					printf("av_frame_make_writable failure!\n");
					sws_freeContext(video_sws_ctx);
					av_frame_free(&frame);
					av_frame_free(&scale_frame);
					avcodec_free_context(&video_codec_ctx);
					avformat_close_input(&fmt_ctx);
					return -1;
				}

				sws_scale(video_sws_ctx,
					(const uint8_t* const*)frame->data, frame->linesize, 
					0, video_codec_ctx->height,
					scale_frame->data, scale_frame->linesize);


				SDL_UpdateYUVTexture(texture, NULL,
					scale_frame->data[0], scale_frame->linesize[0],
					scale_frame->data[1], scale_frame->linesize[1],
					scale_frame->data[2], scale_frame->linesize[2]);

				SDL_RenderClear(render);
				SDL_RenderCopy(render, texture, NULL, &slr);
				SDL_RenderPresent(render);
				SDL_Delay(40);

				av_frame_unref(frame);
			}
		}

		av_packet_unref(&pkt);
	}

	while (1) {
		ret = avcodec_receive_frame(video_codec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		}
		if (ret < 0) {
			printf("avcodec_receive_frame failure!\n");
			sws_freeContext(video_sws_ctx);
			av_frame_free(&frame);
			av_frame_free(&scale_frame);
			avcodec_free_context(&video_codec_ctx);
			avformat_close_input(&fmt_ctx);
			return -1;
		}

		frame->pts = frame->best_effort_timestamp;

		ret = av_frame_make_writable(scale_frame);
		if (ret < 0) {
			printf("av_frame_make_writable failure!\n");
			sws_freeContext(video_sws_ctx);
			av_frame_free(&frame);
			av_frame_free(&scale_frame);
			avcodec_free_context(&video_codec_ctx);
			avformat_close_input(&fmt_ctx);
			return -1;
		}

		sws_scale(video_sws_ctx,
			(const uint8_t* const*)frame->data, frame->linesize, 
			0, video_codec_ctx->height,
			scale_frame->data, scale_frame->linesize);

		SDL_UpdateYUVTexture(texture, NULL,
			scale_frame->data[0], scale_frame->linesize[0],
			scale_frame->data[1], scale_frame->linesize[1],
			scale_frame->data[2], scale_frame->linesize[2]);

		SDL_RenderClear(render);
		SDL_RenderCopy(render, texture, NULL, &slr);
		SDL_RenderPresent(render);

		SDL_Delay(40); // 1s==1000ms 1000/40ms == 25ึก 1ร๋25ึก

		av_frame_unref(frame);
	}

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

	sws_freeContext(video_sws_ctx);
	av_frame_free(&frame);
	av_frame_free(&scale_frame);
	avcodec_free_context(&video_codec_ctx);
	avformat_close_input(&fmt_ctx);

	return 0;
}