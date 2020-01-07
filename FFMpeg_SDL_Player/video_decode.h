#ifndef __VIDEO_DEMUX_H__
#define __VIDEO_DEMUX_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"

int open_codec(AVFormatContext* fmt_ctx,enum AVMediaType type, AVCodecContext** codec_ctx, int* stream_index);

#ifdef __cplusplus
}
#endif

#endif