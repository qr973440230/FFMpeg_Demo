
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL.h"

#ifdef __cplusplus
}
#endif

static Uint8 audioBuffer[4096];
static Uint8 audioLen;
static Uint8 audioPos;

static void fillAudio(void* udata, Uint8* stream, int len) {
	if (audioLen == 0) {
		// 已经播放完毕
		return;
	}

	len = len > audioLen ? audioLen : len;
	SDL_MixAudio(stream, audioBuffer+audioPos, len, SDL_MIX_MAXVOLUME);
	audioPos += len;
	audioLen -= len;
}

int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_AUDIO)) {
		printf("SDL_Init Failure");
		return 1;
	}

	SDL_AudioSpec audioSpec;
	audioSpec.freq = 44100;
	audioSpec.format = AUDIO_S16SYS;
	audioSpec.channels = 2;
	audioSpec.silence = 0;
	audioSpec.samples = 1024;
	audioSpec.callback = fillAudio;

	if (SDL_OpenAudio(&audioSpec, nullptr) < 0) {
		printf("OpenAudio Failure");
		return 1;
	}

	FILE* fp = fopen("test.pcm", "rb+");
	if (fp == nullptr) {
		printf("Open File Failure");
		return 1;
	}

	SDL_PauseAudio(0);
	int data_count = 0;
	while (1) {
		int len = fread(audioBuffer, 1, sizeof(audioBuffer), fp);
		if (len != sizeof(audioBuffer)) {
			break;
		}

		printf("Now Playing %10d Bytes data.\n", data_count);
		data_count += len;

		audioLen = len;
		audioPos = 0;

		while (audioLen > 0) {
			SDL_Delay(100);
		}
	}

	fclose(fp);

	SDL_Quit();

	return 0;
}