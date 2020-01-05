#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/imgutils.h"
#include "SDL.h"

#ifdef __cplusplus
}
#endif

#define MAX_AUDIO_FRAME_SIZE 192000 // 1s 48khz 32bit audio 48000*32/8
unsigned int audioLen = 0;
unsigned char* audioChunk = NULL;
unsigned char* audioPos = NULL;

static void fillAudio(void* udata, Uint8* stream, int len) {
	SDL_memset(stream, 0, len);
	if (audioLen == 0) {
		return;
	}

	len = (len > audioLen) ? audioLen : len;

	SDL_MixAudio(stream, audioPos, len, SDL_MIX_MAXVOLUME);
	audioPos += len;
	audioLen -= len;
}

static int decodeAudio(AVCodecContext* audio_codec_ctx, AVPacket* pkt, AVFrame* frame,struct SwrContext * swr_ctx,unsigned char * out_buf) {
	int ret;
	ret = avcodec_send_packet(audio_codec_ctx, pkt);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "avcodec_send_packet failure Error: %s\n", av_err2str(ret));
		return ret;
	}

	while (1) {
		ret = avcodec_receive_frame(audio_codec_ctx, frame);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			break;
		}

		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "avcodec_receive_frame failure Error: %s\n", av_err2str(ret));
			return ret;
		}

		int bytes = av_get_bytes_per_sample(audio_codec_ctx->sample_fmt);
		if (bytes < 0) {
			av_log(NULL, AV_LOG_ERROR, "av_get_bytes_per_sample failure Error: %s\n", av_err2str(ret));
			return -1;
		}

		swr_convert(swr_ctx, &out_buf, MAX_AUDIO_FRAME_SIZE, 
			(const uint8_t**)frame->data, frame->nb_samples);

		while (audioLen > 0) {
			SDL_Delay(1);
		}

		audioChunk = (unsigned char*)out_buf;
		audioPos = audioChunk;
		audioLen = bytes;
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
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	printf("--------File Infomation-----------\n");
	av_dump_format(fmt_ctx, 0, filePath, 0);
	printf("----------------------------------\n");

	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (ret < 0) {
		printf("av_find_best_stream failure Error: %s \n", av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	int audio_index = ret;
	AVStream* audio_st = fmt_ctx->streams[audio_index];
	AVCodec* audio_codec = avcodec_find_decoder(audio_st->codecpar->codec_id);
	if (!audio_codec) {
		printf("avcodec_find_decoder failure \n");
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	AVCodecContext* audio_codec_ctx = avcodec_alloc_context3(audio_codec);
	if (!audio_codec_ctx) {
		printf("avcodec_alloc_context3 failure");
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	ret = avcodec_parameters_to_context(audio_codec_ctx, audio_st->codecpar);
	if (ret < 0) {
		printf("avcodec_parameters_to_context failure! Error: %s\n", av_err2str(ret));
		avcodec_free_context(&audio_codec_ctx);
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	ret = avcodec_open2(audio_codec_ctx, audio_codec, NULL);
	if(ret < 0){
		printf("avcodec_open2 failure! Error: %s\n", av_err2str(ret));
		avcodec_free_context(&audio_codec_ctx);
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	uint64_t in_channel_layout = av_get_default_channel_layout(audio_codec_ctx->channels);
	uint64_t out_channel_layout = in_channel_layout;
	struct SwrContext* audio_swr_ctx = swr_alloc();
	if (audio_swr_ctx) {
		swr_alloc_set_opts(audio_swr_ctx,
			out_channel_layout, AV_SAMPLE_FMT_S16, audio_codec_ctx->sample_rate,
			in_channel_layout, audio_codec_ctx->sample_fmt, audio_codec_ctx->sample_rate,
			0,
			NULL);
	}
	swr_init(audio_swr_ctx);

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		swr_free(&audio_swr_ctx);
		avcodec_free_context(&audio_codec_ctx);
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	unsigned char* out_buf = NULL;
	out_buf = (unsigned char*)av_malloc(MAX_AUDIO_FRAME_SIZE);



	// SDL Init
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("SDL_Init Failure\n");
		avcodec_free_context(&audio_codec_ctx);
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	SDL_AudioSpec audio_want_spec,audio_spec;
	audio_want_spec.freq = audio_codec_ctx->sample_rate;
	audio_want_spec.format = AUDIO_S16SYS;
	audio_want_spec.channels = audio_codec_ctx->channels;
	audio_want_spec.silence = 0;
	audio_want_spec.samples = 1024;
	audio_want_spec.callback = fillAudio;

	ret = SDL_OpenAudio(&audio_want_spec, &audio_spec);
	if (ret < 0) {
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "SDL_OpenAudio failure Error: %s", SDL_GetError());
		avcodec_free_context(&audio_codec_ctx);
		avformat_close_input(&fmt_ctx);		
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	SDL_PauseAudio(0);

	while (av_read_frame(fmt_ctx,&pkt) >= 0){
		if (pkt.stream_index == audio_index) {
			if (decodeAudio(audio_codec_ctx, &pkt, frame,audio_swr_ctx,out_buf) < 0) {
				av_packet_unref(&pkt);
				break;
			}
		}

		av_packet_unref(&pkt);
	}


	SDL_Quit();

	av_frame_free(&frame);
	av_free(out_buf);
	swr_free(&audio_swr_ctx);
	avcodec_free_context(&audio_codec_ctx);
	avformat_free_context(fmt_ctx);

	return 0;
}