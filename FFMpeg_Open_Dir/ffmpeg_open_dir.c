#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavutil/log.h"

#ifdef __cplusplus
}
#endif


int main(int argc, char* argv[]) {
	AVIODirContext * avIODirCtx = NULL;
	AVIODirEntry* avIODirEntity = NULL;
	
	av_log_set_level(AV_LOG_DEBUG);

	int ret;
	ret = avio_open_dir(&avIODirCtx, "./", NULL);
	if (ret < 0) {
		// Windows 未实现该方法
		av_log(NULL,AV_LOG_ERROR,"Cannot open dir! Error: %s\n",av_err2str(ret));
		return ret;
	}

	while (1) {
		ret = avio_read_dir(avIODirCtx, &avIODirEntity);
		if (ret < 0) {
			av_log(NULL, AV_LOG_ERROR, "Cannot open dir! Error: %s\n", av_err2str(ret));
			avio_close_dir(&avIODirCtx); // 防止内存泄漏
			return ret;
		}

		if (!avIODirEntity)
			break;
		
		av_log(NULL, AV_LOG_INFO, "name: %s size: %d", avIODirEntity->name, avIODirEntity->size);

		avio_free_directory_entry(&avIODirEntity);
	}

	avio_close_dir(&avIODirCtx);

	return 0;
}
