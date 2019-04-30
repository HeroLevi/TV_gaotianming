#include "stdafx.h"
#include "CollectCamera.h"
#include "0830Author.h"
#include "DlgMain.h"


CCollectCamera::CCollectCamera(CString devicename)
{
	//---------------------------------
	ZeroMemory(this->devicename,MAX_DEVICE_NAME_SIZE);
	this->m_thread = NULL;
	this->thread_exit = 0;
	videoindex = -1;

	pFormatCtx = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
	ifmt = NULL;
	pFrame = NULL;
	pFrameYUV = NULL;
	out_buffer = NULL;
	screen = NULL;
	sdlTexture = NULL; 
	sdlRenderer = NULL;
	packet = NULL;
	video_tid = NULL;
	img_convert_ctx = NULL;
	//----------------------------------
	WideCharToMultiByte(CP_OEMCP,0,devicename.GetBuffer(),-1,this->devicename,MAX_DEVICE_NAME_SIZE,NULL,FALSE);
}

CCollectCamera::~CCollectCamera(void)
{
}

int CCollectCamera::sfp_refresh_camera(void *opaque)
{
	CCollectCamera * pthis = (CCollectCamera *)opaque;
	while (pthis->thread_exit == 0)
	{
		SDL_Event event;
		event.type = SFM_REFRESH_CAMERA;
		SDL_PushEvent(&event);
		SDL_Delay(40);
	}
	return 0;
}

void CCollectCamera::Run()
{
	int ret, got_picture;
	unsigned int i;
	
	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	avdevice_register_all();

//Windows

	this->ifmt=av_find_input_format("dshow");
	//Set own video device's name
	if(avformat_open_input(&pFormatCtx,this->devicename,ifmt,NULL) != 0)
	{
		return ;
	}

	if(avformat_find_stream_info(pFormatCtx,NULL) < 0)
	{
		return ;
	}

	videoindex = -1;
	for(i=0;i<pFormatCtx->nb_streams;i++) 
	{
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			videoindex = i;
			break;
		}
	}
	if(videoindex == -1)
	{
		return ;
	}

	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec == NULL)
	{
		return ;
	}
	if(avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
	{
		return ;
	}

	this->pFrame=av_frame_alloc();
	this->pFrameYUV=av_frame_alloc();
	out_buffer=(uint8_t *)av_malloc(avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
	avpicture_fill((AVPicture *)pFrameYUV, out_buffer, PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);
	
	//SDL----------------------------
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
	{  
		return ;
	} 
	int screen_w=0,screen_h=0;
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;

	screen = SDL_CreateWindowFrom((void *)((CDlgMain *)theApp.m_pMainWnd)->m_picture.GetSafeHwnd());

	if(!screen)
	{  
		return ;
	}

	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);
	sdlTexture = SDL_CreateTexture(sdlRenderer,SDL_PIXELFORMAT_IYUV,SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);

	SDL_Rect rect;
	rect.x = 0;    
	rect.y = 0;    
	rect.w = screen_w;    
	rect.h = screen_h;  
	//SDL End------------------------

	this->packet=(AVPacket *)av_malloc(sizeof(AVPacket));

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 
	//------------------------------
	//Event Loop
	SDL_Event event;
	for (;;) 
	{
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_CAMERA)
		{
			//------------------------------
			if(av_read_frame(pFormatCtx, packet)>=0)
			{
				if(packet->stream_index==videoindex)
				{
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if(ret < 0)
					{
						return ;
					}
					if(got_picture)
					{
						sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize,0,pCodecCtx->height,pFrameYUV->data,pFrameYUV->linesize); 

						SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0]);
						SDL_RenderClear( sdlRenderer );    
						SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);
						SDL_RenderPresent( sdlRenderer );
					}
				}
				av_free_packet(packet);
			}
			if(thread_exit == 1)
			{
				break;
			}
		}
		else if(event.type==SDL_QUIT){
			thread_exit=1;
			break;
		}

	}
}

void CCollectCamera::Start()
{
	this->m_thread = CreateThread(NULL,0,&ThreadCamera,this,0,NULL);
	Sleep(100);
	this->video_tid = SDL_CreateThread(sfp_refresh_camera,NULL,this);
}

DWORD WINAPI CCollectCamera::ThreadCamera(LPVOID lpParameter)
{
	CCollectCamera * pthis = (CCollectCamera *)lpParameter;
	pthis->Run();
	return 0;
}

void CCollectCamera::Stop()
{
	this->thread_exit = 1;
	if(WAIT_TIMEOUT == WaitForSingleObject(this->m_thread,100))
	{
		TerminateThread(this->m_thread,-1);
	}

	if(this->m_thread)
	{
		CloseHandle(this->m_thread);
		this->m_thread = NULL;
	}

	sws_freeContext(img_convert_ctx);

	SDL_DestroyTexture(this->sdlTexture);
	SDL_DestroyRenderer(this->sdlRenderer);
	SDL_Quit();

	av_free(out_buffer);
	av_free(pFrameYUV);
	av_free(pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
}
