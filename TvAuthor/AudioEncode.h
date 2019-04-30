#pragma once
#ifndef AUDIOENCODE_H__
#define AUDIOENCODE_H__

#include <stdio.h>

#define __STDC_CONSTANT_MACROS

//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
};

class CAudioEncode
{
public:
	CAudioEncode(void);
	~CAudioEncode(void);
public:
	AVFormatContext* pFormatCtx;
	AVOutputFormat* fmt;
	AVStream* audio_st;
	AVCodecContext* pCodecCtx;
	AVCodec* pCodec;

	uint8_t* frame_buf;
	AVFrame* pFrame;
	AVPacket pkt;

	FILE *in_file ;
	const char* ain_file;
	const char* out_file;
public:
	int flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index);
	int AudioEncode();
	void Initial();
};

#endif