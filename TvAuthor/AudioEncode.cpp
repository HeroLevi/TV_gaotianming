#include "stdafx.h"
#include "AudioEncode.h"
#include "0830Author.h"
#include "0830AuthorDlg.h"
#include "DlgMain.h"

CAudioEncode::CAudioEncode(void)
{
	pFormatCtx = NULL;
	fmt = NULL;
	audio_st = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
	frame_buf = NULL;
	pFrame = NULL;
	in_file = NULL ;
	out_file = NULL; 
	ain_file = NULL;
}


CAudioEncode::~CAudioEncode(void)
{
}

void CAudioEncode::Initial()
{
	this->out_file = "audio.aac";
}

int CAudioEncode::flush_encoder(AVFormatContext *fmt_ctx,unsigned int stream_index)
{
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities & CODEC_CAP_DELAY))
		return 0;
	while (1) 
	{
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_audio2(fmt_ctx->streams[stream_index]->codec,&enc_pkt,NULL,&got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame)
		{
			ret=0;
			break;
		}

		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

int CAudioEncode::AudioEncode()
{
	
		this->ain_file = "../TempFile/audio.pcm";
		this->out_file = "../TempFile/audio.aac";
	

	fopen_s(&this->in_file,ain_file,"rb");

	av_register_all();

	//Method 1.
	pFormatCtx = avformat_alloc_context();
	fmt = av_guess_format(NULL, out_file, NULL);
	pFormatCtx->oformat = fmt;


	//Method 2.
	//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	//fmt = pFormatCtx->oformat;

	//Open output URL
	if (avio_open(&pFormatCtx->pb,out_file, AVIO_FLAG_READ_WRITE) < 0)
	{
		
		return -1;
	}

	audio_st = avformat_new_stream(pFormatCtx, 0);
	if (audio_st==NULL)
	{
		return -1;
	}
	pCodecCtx = audio_st->codec;
	pCodecCtx->codec_id = fmt->audio_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_AUDIO;
	pCodecCtx->sample_fmt = AV_SAMPLE_FMT_S16;
	pCodecCtx->sample_rate= 48000;
	pCodecCtx->channel_layout=AV_CH_LAYOUT_STEREO;
	pCodecCtx->channels = av_get_channel_layout_nb_channels(pCodecCtx->channel_layout);
	pCodecCtx->bit_rate = 64000;  

	//Show some information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec)
	{
		
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec,NULL) < 0)
	{
	
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrame->nb_samples= pCodecCtx->frame_size;
	pFrame->format= pCodecCtx->sample_fmt;
	
	int size = av_samples_get_buffer_size(NULL,pCodecCtx->channels,pCodecCtx->frame_size,pCodecCtx->sample_fmt,1);
	frame_buf = (uint8_t *)av_malloc(size);
	avcodec_fill_audio_frame(pFrame, pCodecCtx->channels,pCodecCtx->sample_fmt,(const uint8_t*)frame_buf,size,1);
	
	//Write Header
	avformat_write_header(pFormatCtx,NULL);

	av_new_packet(&pkt,size);

	int framenum = 500;
	for (int i=0; i<framenum; i++)
	{
		//Read PCM
		if (fread(frame_buf, 1, size, in_file) <= 0)
		{
			
			avio_close(pFormatCtx->pb);
			fclose(in_file);
			return -1;
		}
		else if(feof(in_file))
		{
			break;
		}

		pFrame->data[0] = frame_buf;  //PCM Data

		pFrame->pts=i*100;
		int got_frame=0;
		//Encode
		int ret = avcodec_encode_audio2(pCodecCtx,&pkt,pFrame,&got_frame);
		if(ret < 0)
		{
			
			return -1;
		}
		if (got_frame==1)
		{

			pkt.stream_index = audio_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}
	
	//Flush Encoder
	int rets = flush_encoder(pFormatCtx,0);
	if (rets < 0) 
	{
		
		return -1;
	}

	//Write Trailer
	av_write_trailer(pFormatCtx);

	//Clean
	if (audio_st)
	{
		avcodec_close(audio_st->codec);
		av_free(pFrame);
		av_free(frame_buf);
	}

	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;
}