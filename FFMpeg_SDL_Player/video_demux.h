
#include "libavformat/avformat.h"

extern int open_codec(AVFormatContext* fmt_ctx,enum AVMediaType type, AVCodecContext** codec_ctx, int* stream_index);
