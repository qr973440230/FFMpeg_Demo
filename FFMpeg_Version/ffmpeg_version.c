// FFMpeg_Demo.cpp: 定义应用程序的入口点。
//

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libavcodec/avcodec.h"

#ifdef __cplusplus
}
#endif

int main(int argc,char * argv[]){
	printf("%d", avcodec_version());
	return 0;
}
