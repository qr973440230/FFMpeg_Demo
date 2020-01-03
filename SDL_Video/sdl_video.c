
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "SDL.h"

#ifdef __cplusplus
}
#endif

static const int pixel_w = 640;
static const int pixel_h = 480;
static const int bpp = 12;
static int isExit = 0;
static unsigned char buffer[640 * 680 * 12 / 8] = {0};
#define REFRESH_EVENT (SDL_USEREVENT + 1)

static int refresh_video(void* data) {
	SDL_mutex* mutex = (SDL_mutex*)data;
	while (1) {
		SDL_LockMutex(mutex);
		if (isExit) {
			SDL_UnlockMutex(mutex);
			break;
		}
		SDL_UnlockMutex(mutex);

		SDL_Event sle;
		sle.type = REFRESH_EVENT;
		SDL_PushEvent(&sle);

		SDL_Delay(40);
	}
	return 0;
}


int main(int argc, char* argv[]) {
	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("SDL_Init Failure");
		return 1;
	}

	SDL_Window* window = SDL_CreateWindow("SDL_Video",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		pixel_w, pixel_h, SDL_WINDOW_RESIZABLE);
	if (!window) {
		printf("SDL_CreateWindow Failure");
		return 1;
	}

	SDL_Renderer* render = SDL_CreateRenderer(window, -1, 0);
	if (!render) {
		printf("SDL_CreateRenderer Failure");
		return 1;
	}

	SDL_Texture* texture = SDL_CreateTexture(render, 
		SDL_PIXELFORMAT_YVYU, 
	    SDL_TEXTUREACCESS_STREAMING, 
		pixel_w, pixel_h);

	if (!texture) {
		printf("SDL_CreateTexture Failure");
		return 1;
	}

	SDL_mutex* mutex = SDL_CreateMutex();
	SDL_Thread* thread = SDL_CreateThread(refresh_video, "Refresh Video", mutex);
	SDL_Event sle;
	SDL_Rect slr;

	while (1)
	{
		SDL_WaitEvent(&sle);
		if (sle.type == SDL_QUIT) {
			SDL_LockMutex(mutex);
			isExit = 1;
			SDL_UnlockMutex(mutex);
			break;
		}

		if (sle.type == REFRESH_EVENT) {
			static unsigned char index = 0;
			++index;
			SDL_memset(buffer, index, sizeof(buffer));

			SDL_UpdateTexture(texture, NULL, buffer, pixel_w);
			
			// 左上角
			SDL_RenderClear(render);
			slr.x = 0;
			slr.y = 0;
			slr.w = pixel_w / 2-10;
			slr.h = pixel_h / 2-10;
			SDL_RenderCopy(render, texture, NULL, &slr);
			// 右上角
			slr.x = pixel_w / 2 + 10;
			slr.y = 0;
			slr.w = pixel_w / 2-10;
			slr.h = pixel_h / 2-10;


			SDL_RenderCopy(render, texture, NULL, &slr);
			// 左下角
			slr.x = 0;
			slr.y = pixel_h / 2 + 10;
			slr.w = pixel_w / 2-10;
			slr.h = pixel_h / 2-10;


			SDL_RenderCopy(render, texture, NULL, &slr);
			// 右下角
			slr.x = pixel_w / 2 + 10;
			slr.y = pixel_h / 2 + 10;
			slr.w = pixel_w / 2-10;
			slr.h = pixel_h / 2-10;

			SDL_RenderCopy(render, texture, NULL, &slr);
			SDL_RenderPresent(render);
		}
	}

	// 等待线程结束
	SDL_WaitThread(thread, NULL);

	// 释放资源
	SDL_DestroyMutex(mutex);
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(render);
	SDL_DestroyWindow(window);

	return 0;
}