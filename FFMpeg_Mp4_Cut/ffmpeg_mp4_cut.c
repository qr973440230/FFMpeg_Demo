
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

	int ret;
	char* src = "./test.mp4";
	char* dst = "./output.mp4";
	uint64_t from_seconds = 3;
	uint64_t to_seconds = 6;

	// 1.open_input
	AVFormatContext* fmt_ctx = NULL;
	ret = avformat_open_input(&fmt_ctx, src, NULL,NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avformat_open_input failure! Error: %s\n",av_err2str(ret));
		return ret;
	}

	av_dump_format(fmt_ctx, 0, src, 0);
	
	// 2.open_out_context
	AVFormatContext* ofmt_ctx = NULL;
	ret = avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, dst);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "Cannot alloc output context! Error: %s\n",av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	// 3.copy all Stream
	int nb_streams = fmt_ctx->nb_streams;
	for (int i = 0; i < nb_streams; i++) {
		AVStream* out_stream;
		AVStream* in_stream;
		in_stream = fmt_ctx->streams[i];
		out_stream = avformat_new_stream(ofmt_ctx, NULL);
		if (!out_stream) {
			av_log(NULL, AV_LOG_ERROR, "Cannot new stream!\n");
			avformat_free_context(ofmt_ctx);
			avformat_close_input(&fmt_ctx);
			return -1;
		}

		ret = avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot copy parameters! Error: %s\n",av_err2str(ret));
			avformat_free_context(ofmt_ctx);
			avformat_close_input(&fmt_ctx);
			return ret;
		}

		out_stream->codecpar->codec_tag = 0;
	}

	av_dump_format(ofmt_ctx, 0, dst, 1);

	if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
		ret = avio_open(&ofmt_ctx->pb,dst,AVIO_FLAG_WRITE);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot open file! File: %s! Error: %s\n",dst,av_err2str(ret));
			avformat_free_context(ofmt_ctx);
			avformat_close_input(&fmt_ctx);
			return ret;
		}
	}

	// 4.write header 
	ret = avformat_write_header(ofmt_ctx, NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR,"Cannot write header! Error: %s\n", av_err2str(ret));
		avformat_free_context(ofmt_ctx);
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	// 5.seek_to
	ret = av_seek_frame(fmt_ctx, -1, from_seconds * AV_TIME_BASE, AVSEEK_FLAG_ANY);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR,"Cannot seek to %ds! Error: %s\n",from_seconds,av_err2str(ret));
		avformat_free_context(ofmt_ctx);
		avformat_close_input(&fmt_ctx);
		return ret;
	}

	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	while (1) {
		AVStream* in_stream;
		AVStream* out_stream;
		ret = av_read_frame(fmt_ctx, &pkt);
		if (ret < 0) {
			break;
		}

		in_stream = fmt_ctx->streams[pkt.stream_index];
		out_stream = ofmt_ctx->streams[pkt.stream_index];

		// 6.adjust end
		if (av_q2d(in_stream->time_base) * pkt.pts > to_seconds) {
			av_packet_unref(&pkt);
			break;
		}

		// copy packet
		pkt.pts = av_rescale_q_rnd(pkt.pts,
			in_stream->time_base, out_stream->time_base,
			AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
		pkt.dts = av_rescale_q_rnd(pkt.dts,
			in_stream->time_base, out_stream->time_base,
			AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
		pkt.duration = av_rescale_q(pkt.duration,
			in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;

		ret = av_interleaved_write_frame(ofmt_ctx, &pkt);
		if (ret < 0) {
			av_log(NULL,AV_LOG_ERROR,"Cannot interleaved write frame! Error: %s\n", av_err2str(ret));
			av_packet_unref(&pkt);
			break;
		}
		av_packet_unref(&pkt);
	}

	av_write_trailer(ofmt_ctx);

	if (!(ofmt_ctx->flags & AVFMT_NOFILE)) {
		avio_closep(&ofmt_ctx->pb);
	}

	avformat_free_context(ofmt_ctx);
	avformat_close_input(&fmt_ctx);

	return 0;
}