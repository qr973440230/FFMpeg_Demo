
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif

int main(int argc,char * argv[]) {
	av_log_set_level(AV_LOG_INFO);

	AVFormatContext* fmt_ctx = NULL;
	int ret;

	ret = avformat_open_input(&fmt_ctx, "./test.mp4",NULL,NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avformat_open_input failure %s\n",av_err2str(ret));
		return ret;
	}

	ret = avformat_find_stream_info(fmt_ctx, NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avformat_find_stream_info failure %s\n",av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	av_dump_format(fmt_ctx, 0, "./test.mp4", 0);

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "av_find_best_stream failure %s\n",av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	int video_stream_index = ret;
	AVStream* video_stream = fmt_ctx->streams[video_stream_index];
	AVCodec * video_codec = avcodec_find_decoder(video_stream->codecpar->codec_id);
	if (!video_codec) {
		av_log(NULL,AV_LOG_ERROR, "avcodec_find_decoder failure\n");
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	AVCodecContext * video_codec_ctx = avcodec_alloc_context3(NULL);
	if (!video_codec_ctx) {
		av_log(NULL,AV_LOG_ERROR, "avcodec_find_decoder failure\n");
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	ret = avcodec_parameters_to_context(video_codec_ctx, video_stream->codecpar);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avcodec_parameters_to_context failure %s\n",av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	ret = avcodec_open2(video_codec_ctx, video_codec, NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avcodec_open2 failure %s\n",av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}
	
	AVFrame* frame = av_frame_alloc();
	if (!frame) {
		av_log(NULL,AV_LOG_ERROR, "av_frame_alloc failure\n");
		avcodec_free_context(&video_codec_ctx);
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	while (av_read_frame(fmt_ctx, &pkt) >= 0) {
		if (pkt.stream_index == video_stream_index) {
			// 解码 
			ret = avcodec_send_packet(video_codec_ctx, &pkt);
			if (ret < 0) {
				av_log(NULL,AV_LOG_ERROR, "avcodec_send_packet failure Error: %s\n",av_err2str(ret));
				break;
			}

			do {
				ret = avcodec_receive_frame(video_codec_ctx, frame);
				if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
					break;
				}
				if (ret < 0) {
					printf("avcodec_receive_frame failure! Error: %s\n", av_err2str(ret));
					av_frame_free(&frame);
					avcodec_free_context(&video_codec_ctx);
					avformat_close_input(&fmt_ctx);
					return -1;
				}

				// 解码Video完成


			} while (1);

		}
		av_packet_unref(&pkt);
	}

	av_frame_free(&frame);
	avcodec_free_context(&video_codec_ctx);
	avformat_close_input(&fmt_ctx);
	return 0;
}