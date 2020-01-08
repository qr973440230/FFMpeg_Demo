
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
    av_log(nullptr, AV_LOG_INFO, "%s", avformat_configuration());
    return 0;
}