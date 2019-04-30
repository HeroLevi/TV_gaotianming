#pragma once
#include <stdio.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
	//SDL
#include "include\SDL.h"
//#include "sdl/SDL_thread.h"
};

#define USE_DSHOW111 1
#define MAX_DEVICE_NAME_SIZE 50
//Refresh Event
#define SFM_REFRESH_CAMERA  (SDL_USEREVENT + 2)

class CCollectCamera
{
public:
	CCollectCamera(CString devicename);
	~CCollectCamera(void);
public:
	void Run();
	void Start();
	void Stop();
public:
	HANDLE m_thread;
	int thread_exit;
	static DWORD WINAPI ThreadCamera(LPVOID lpParameter);
	static int sfp_refresh_camera(void *opaque);
public:
	AVFormatContext	*pFormatCtx;
	AVCodecContext	*pCodecCtx;
	AVCodec			*pCodec;
	AVInputFormat   *ifmt;
	AVFrame     	*pFrame;
	AVFrame         *pFrameYUV;
	uint8_t         *out_buffer;
	SDL_Window      *screen; 
	SDL_Texture     *sdlTexture; 
	SDL_Renderer    *sdlRenderer;
	AVPacket        *packet;
	SDL_Thread      *video_tid;
	int             videoindex;
	struct SwsContext *img_convert_ctx;

	char devicename[MAX_DEVICE_NAME_SIZE];
};

