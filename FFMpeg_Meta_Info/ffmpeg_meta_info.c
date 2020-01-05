
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

	av_dump_format(fmt_ctx, 0, "./test.mp4", 0);

	avformat_close_input(&fmt_ctx);
	return 0;
}