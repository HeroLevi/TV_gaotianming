#pragma once
#include <stdio.h>
#include <process.h>
#include <Windows.h>
#include "MyEncode.h"
using namespace std;

#define USE_H264BSF 1  

//'1': Use AAC Bitstream Filter   
#define USE_AACBSF 1 
//Linux... ����ffmpegͷ�ļ�
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
		 //��ʽ�����½���壬,�������Ϊ�洢���������ļ�������������������
    AVFormatContext *pFormatCtx;
    //����������Ľṹ�壬����ʱ�����趨����
    AVCodecContext  *pCodecCtx;
    //��������ṹ�壬��Ҫ�洢���������һЩ��Ϣ
    AVCodec         *pCodec;

	u_int             i, videoindex;
	AVPacket *packet;
	AVFrame *pFrame,*pFrameYUV;  //AVFrame�Ǳ���ǰ��Դ���ݣ�����������ݣ�
	//SwsContext��Ϊsws_scale�ĵ�һ������,��¼����Ҫת���ĸ�ʽ����С��ת����ʽ
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

