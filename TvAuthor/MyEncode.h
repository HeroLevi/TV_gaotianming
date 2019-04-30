#pragma once
#include <stdio.h>

#define __STDC_CONSTANT_MACROS

//#ifdef _WIN32
//Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};
//#else
//Linux...
//#ifdef __cplusplus
//extern "C"
//{
//#endif
//#include <libavutil/opt.h>
//#include <libavcodec/avcodec.h>
//#include <libavformat/avformat.h>
//#ifdef __cplusplus
//};
//#endif
//#endif

class MyEncode
{
public:
	MyEncode(void);
	~MyEncode(void);
public:
	int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);
	void InitialEncode();
	void StartEncode();
	//void Setmalv(int malv);
private:
	//AVFormatContext* pFormatCtx;
	//AVOutputFormat* fmt;
	//AVStream* video_st;
	//AVCodecContext* pCodecCtx;
	//AVCodec* pCodec;
	//AVPacket pkt;
	//uint8_t* picture_buf;
	//AVFrame* pFrame;
	//int picture_size;
	//int y_size;
	//int framecnt;
	//FILE *in_file;   //Input raw YUV data
	int in_w,in_h;                              //Input data's width and height
	//int framenum;                                   //Frames to encode
	////const char* out_file = "src01.h264";              //Output Filepath 
	////const char* out_file = "src01.ts";
	////const char* out_file = "src01.hevc";
	//const char* out_file;
};

