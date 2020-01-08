#include <cstdio>
#ifdef __cplusplus
extern "C" {
#endif

#include "libavformat/avformat.h"
#include "libavutil/log.h"

#ifdef __cplusplus
}
#endif


int main(int argc, char* argv[]) {
    av_log_set_level(AV_LOG_INFO);

	AVIODirContext * avIODirCtx = nullptr;
	AVIODirEntry* avIODirEntity = nullptr;

	int ret;
	ret = avio_open_dir(&avIODirCtx, "./", nullptr);
	if (ret < 0) {
		// Windows 未实现该方法
		av_log(nullptr,AV_LOG_ERROR,"Cannot open dir!\n");
		return ret;
	}

	while (true) {
		ret = avio_read_dir(avIODirCtx, &avIODirEntity);
		if (ret < 0) {
			av_log(nullptr, AV_LOG_ERROR, "Cannot open dir!\n");
			avio_close_dir(&avIODirCtx); // 防止内存泄漏
			return ret;
		}

		if (!avIODirEntity)
			break;
		
		av_log(nullptr, AV_LOG_INFO, "name: %s size: %lld", avIODirEntity->name, avIODirEntity->size);

		avio_free_directory_entry(&avIODirEntity);
	}

	avio_close_dir(&avIODirCtx);

	return 0;
}
