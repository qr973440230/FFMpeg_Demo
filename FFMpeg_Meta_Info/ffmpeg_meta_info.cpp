
#include <cstdio>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"

#ifdef __cplusplus
}
#endif

int main(int argc, char *argv[]) {
    av_log_set_level(AV_LOG_INFO);

    AVFormatContext *fmt_ctx = nullptr;
    int ret;

    ret = avformat_open_input(&fmt_ctx, "./test.mp4", nullptr, nullptr);
    if (ret < 0) {
        av_log(nullptr, AV_LOG_ERROR, "avformat_open_input failure!\n");
        return ret;
    }

    av_dump_format(fmt_ctx, 0, "./test.mp4", 0);

    avformat_close_input(&fmt_ctx);
    return 0;
}