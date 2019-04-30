#pragma once
#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include "MyEncode.h"
using namespace std;

#define USE_H264BSF 1  

//'1': Use AAC Bitstream Filter   
#define USE_AACBSF 1 
//Linux... 引用ffmpeg头文件
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
//#include "SDL/SDL.h"
}

//Output YUV420P
#define OUTPUT_YUV420P 1
class Myffmpeg;
class MyCollectDesktop
{
public:
	MyCollectDesktop(Myffmpeg *pffmpeg);
	~MyCollectDesktop(void);
public:
	 bool Initial();
	 bool InitialAVcode();
	 void UnInitial();
	 void Run();
	 void Close();
	 void CollectDesktop();
	 void AVMP4(const char* in_filename_v, const char* in_filename_a, const char* out_filename);
public:
	static  unsigned _stdcall ThreadProc( void * );
private:
		 //格式上问下结果体，,可以理解为存储数据流的文件，伴随整个生命周期
    AVFormatContext *pFormatCtx;
    //编解码上下文结构体，编码时用于设定参数
    AVCodecContext  *pCodecCtx;
    //编解码器结构体，主要存储编解码器的一些信息
    AVCodec         *pCodec;

	u_int             i, videoindex;
	AVPacket *packet;
	AVFrame *pFrame,*pFrameYUV;  //AVFrame是编码前的源数据（或解码后的数据）
	//SwsContext作为sws_scale的第一个参数,记录数据要转换的格式、大小及转换方式
    struct SwsContext *img_convert_ctx;
	unsigned char *out_buffer;
private:
	HANDLE m_hThread;
	
	bool   m_bQuit;
	Myffmpeg *m_pffmpeg;
private:
	MyEncode *m_pEncode;
public:
	HANDLE m_hEventDesktopmp4;
};

