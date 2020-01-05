
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
	char* dst = "./output.aac";

	//1.open_input
	AVFormatContext* fmt_ctx = NULL;
	ret = avformat_open_input(&fmt_ctx, src, NULL,NULL);
	if (ret < 0) {
		av_log(NULL,AV_LOG_ERROR, "avformat_open_input failure! Error: %s\n",av_err2str(ret));
		return ret;
	}

	av_dump_format(fmt_ctx, 0, src, 0);
	
	// 2.open output file
	FILE * dst_fd = fopen(dst, "wb");
	if (!dst_fd) {
		av_log(NULL, AV_LOG_ERROR, "Cannot open output file");
		avformat_close_input(&fmt_ctx);
		return -1;
	}

	// 3.find audio stream_index
	ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (ret < 0) {
		av_log(NULL, AV_LOG_ERROR, "av_find_best_steam failure! Error: %s\n", av_err2str(ret));
		avformat_close_input(&fmt_ctx);
		fclose(dst_fd);
		return ret;
	}
	
	int audio_index;
	audio_index = ret;

	AVPacket pkt;
	av_init_packet(&pkt);

	while (av_read_frame(fmt_ctx, &pkt) >= 0) {
		if (pkt.stream_index == audio_index) {
			// 4.write audio header TODO: µÈ´ýÍê³É

			// 5.write pkt data to dst
			fwrite(pkt.data, 1, pkt.size, dst_fd);
			fflush(dst_fd);
		}

		av_packet_unref(&pkt);
	}



	fclose(dst_fd);
	avformat_close_input(&fmt_ctx);
	return 0;
}