#include "stdafx.h"
#include "MyCollectDesktop.h"

#include "0830Author.h"
#include "Myffmpeg.h"
MyCollectDesktop::MyCollectDesktop(Myffmpeg *pffmpeg)
{
	m_bQuit = true;
	m_hThread = NULL;
	pFormatCtx = NULL;
	pCodecCtx = NULL;
	pCodec = NULL;
	packet= NULL;
	pFrame = NULL;
	pFrameYUV = NULL;
	img_convert_ctx = NULL;
	out_buffer = NULL;
	m_pEncode = new MyEncode;
	m_hEventDesktopmp4 = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_pffmpeg = pffmpeg;
}


MyCollectDesktop::~MyCollectDesktop(void)
{
	if(m_pEncode)
	{
		delete m_pEncode;
		m_pEncode = NULL;
	}
}
 bool MyCollectDesktop::Initial()
 {
	 if(!InitialAVcode())
		 return false;

	// m_pEncode->InitialEncode();
	 return true;
 }
 bool MyCollectDesktop::InitialAVcode()
 {
	
    //ע�Ḵ�����ͱ�����������е�ʹ��ffmpeg�����ȱ�������������
    av_register_all();
    //���ڴ�����������ݣ������������������ݣ��ɲ��ã��籾���ɲ��ã�
    avformat_network_init();
    
    //ע���豸�ĺ��������û�ȡ����ͷ���ݻ���Ƶ�ȣ���Ҫ�˺�����ע��
    avdevice_register_all();
    
    //AVFormatContext��ʼ�����������ýṹ���һЩĬ����Ϣ
    pFormatCtx = avformat_alloc_context();

    //Linux
    AVDictionary* options = NULL;
    //����¼����������¼��ͼ��Ĵ�С��¼��֡�ʵ�
    // av_dict_set(&options,"video_size","480x272",0);//�����ô���ȫ������ʵ�ʲ���ʱ��������Ĭ�ϴ�СΪ640x480
    //av_dict_set(&options,"framerate","5",0);
    //av_dict_set(&options,"offset_x","20",0);
    //av_dict_set(&options,"offset_y","40",0);
    //����linux¼����ʹ��x11grab,����windows����Ҫgdigrab��dshow(��Ҫ��װץ�������screen-capture-recorder)��mac��ʹ��avfoundation��������
    AVInputFormat *ifmt=av_find_input_format("gdigrab");
    
    if(avformat_open_input(&pFormatCtx,"desktop",ifmt,&options)!=0){
       // printf("Couldn't open input stream.\n");
        return false;
    }
    //Ѱ�ҵ���ȡ����
    if(avformat_find_stream_info(pFormatCtx,NULL)<0)
    {
       // printf("Couldn't find stream information.\n");
        return false;
    }
    //pFormatCtx->nb_streams��¼pFormatCtx->streams������ΪAVStream����������Ƶ������Ƶ������Ļ��
    videoindex=-1;
    for(i=0;i< pFormatCtx->nb_streams; i++)
        if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) //��ƪֻ��ע��Ƶ����Ҳֻ����Ƶ����
        {
            videoindex=i;
            break;
        }
    if(videoindex==-1)
    {
        //printf("Didn't find a video stream.\n");
        return false;
    }
    //pFormatCtx->streams[videoindex]������Ƶ����AVStream��
    pCodecCtx=pFormatCtx->streams[videoindex]->codec;
    pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
    if(pCodec==NULL)
    {
        //printf("Codec not found.\n");
        return false;
    }
    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0)
    {
        //printf("Could not open codec.\n");
        return false;
    }
  
  
    pFrame=av_frame_alloc();   //����ԭʼ֡
    pFrameYUV=av_frame_alloc();//ת����yuv���֡�������ڴ˴�
    out_buffer=(unsigned char *)av_malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height));
    //��������ʹ��avpicture_fill����֡��������������ڴ������
    //������ʹ�ñ�������Ϊ�Ѿ�����Ŀռ�Ľṹ��AVPicture����һ�����ڱ������ݵĿռ䣬����ṹ������һ��ָ������data[4]��������������
    avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height);//���Ͼ���ΪpFrameRGB����buffer�����buffer�����ڴ滺�����ݵ�
    //
    
    //AVPacket���������һ��������һ֡����Ϊһ����
     packet=(AVPacket *)av_malloc(sizeof(AVPacket));

//#if OUTPUT_YUV420P
    //FILE *fp_yuv=fopen("output.yuv","wb+");
//#endif
    
    img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	 return true;
 }

 void MyCollectDesktop::Run()
 {
	m_hThread=(HANDLE) _beginthreadex(NULL,0,&ThreadProc,this,0,0);

	
 }

 unsigned _stdcall MyCollectDesktop::ThreadProc( void * lpvoid)
 {
	 MyCollectDesktop *pthis = (MyCollectDesktop*)lpvoid;
	 pthis->CollectDesktop();
	 return 0;
 }

 void MyCollectDesktop::CollectDesktop()
 {
	 int ret;
		 //, got_picture;
	 int nCountNum = 0;
	 //int nCount = 0;

	  while(m_bQuit) 
	  {
		  FILE *fp_yuv;
		  fopen_s(&fp_yuv,"../TempFile/output.yuv","wb+");
		 // nCount = nCount%10;
		  DWORD dwOldTime = GetTickCount();
		  HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
		   LARGE_INTEGER liFirstFire ;
    liFirstFire.QuadPart = -10000; // negative means relative time
   // LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds
    BOOL bOK = SetWaitableTimer(hWakeUp,&liFirstFire,40,NULL,NULL,FALSE);
		  for(int i = 0;i < 250;i++)
		  {
			 // 
			  WaitForSingleObject(hWakeUp,INFINITE);
			    //��ȡһ����
		       // Sleep(40);
			    //  Sleep(40);
				if(av_read_frame(pFormatCtx, packet)>=0)
				{
					int got_picture = 0;
					if(packet->stream_index==videoindex)
					{
						ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);//��packet����pFrame,pFrame->data = packet->data?  pFrame->linesize=packet->linesize
						if(ret < 0){
							//printf("Decode Error.\n");
							return ;
						}
						if(got_picture)
						{
							//ת������Դ����pFrameת����pFrameYUV��pFrameYUV��ǰ�����ø�ʽΪyuv420P
							 sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
	#if OUTPUT_YUV420P
							//FILE *fp_yuv=fopen("output.yuv","wb+");
						//	FILE *fp_yuv = NULL;
		                  //  fopen_s(&fp_yuv,"output.yuv","wb+");
							if(fp_yuv == NULL)
							{
								int n = GetLastError();
								av_free_packet(packet);
								continue;
							}
								
							int y_size=pCodecCtx->width*pCodecCtx->height;
							fwrite(pFrameYUV->data[0],1,y_size,fp_yuv);    //Y
							fwrite(pFrameYUV->data[1],1,y_size/4,fp_yuv);  //U
							fwrite(pFrameYUV->data[2],1,y_size/4,fp_yuv);  //V
						//	fclose(fp_yuv);

						
	#endif 
						} 
					}

					av_free_packet(packet);

					
				}
				
		  }
          fclose(fp_yuv);
		  DWORD dwTime = (GetTickCount() - dwOldTime)/1000;
		  //����
		m_pEncode->StartEncode();

		
		WaitForSingleObject(m_pffmpeg->m_pAudio->m_EventAudiomp4,INFINITE);
		CTime t = CTime::GetCurrentTime();
		char szFileName[MAX_PATH] = {0};
		char szPath[MAX_PATH] = {0};
		CString strFile = L"../TempFile/";
		CString strTime	=  t.Format(L"%H-%M-%S");
		strTime+= L".mp4";
		strFile = strFile +strTime;
		WideCharToMultiByte(CP_ACP,0,strFile,-1,szPath,MAX_PATH,0,0);
		WideCharToMultiByte(CP_ACP,0,strTime,-1,szFileName,MAX_PATH,0,0);
		AVMP4("../TempFile/ds.h264","../TempFile/audio.aac",szPath);
		//����
		((TCPKernel*)theApp.GetKernel())->SendFileData(szPath,szFileName);

		SetEvent(m_hEventDesktopmp4);
       }



 }

 void MyCollectDesktop::AVMP4(const char* in_filename_v, const char* in_filename_a, const char* out_filename)
{
	AVOutputFormat *ofmt = NULL;
	//�����Ӧһ��AVFormatContext�������Ӧһ��AVFormatContext
	AVFormatContext *ifmt_ctx_v = NULL, *ifmt_ctx_a = NULL,*ofmt_ctx = NULL;
	AVPacket pkt;
	int ret, i;
	int videoindex_v=-1,videoindex_out=-1;
	int audioindex_a=-1,audioindex_out=-1;
	int frame_index=0;
	int64_t cur_pts_v=0,cur_pts_a=0;

	av_register_all();
	//���루Input��
	if ((ret = avformat_open_input(&ifmt_ctx_v, in_filename_v, 0, 0)) < 0)
	{
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_v, 0)) < 0) 
	{
		goto end;
	}

	if ((ret = avformat_open_input(&ifmt_ctx_a, in_filename_a, 0, 0)) < 0) 
	{
		goto end;
	}
	if ((ret = avformat_find_stream_info(ifmt_ctx_a, 0)) < 0) 
	{
		goto end;
	}
	printf("Input Information=====================\n");
	av_dump_format(ifmt_ctx_v, 0, in_filename_v, 0);
	av_dump_format(ifmt_ctx_a, 0, in_filename_a, 0);
	printf("======================================\n");
	//�����Output��
	avformat_alloc_output_context2(&ofmt_ctx, NULL, NULL, out_filename);
	if (!ofmt_ctx) 
	{
		ret = AVERROR_UNKNOWN;
		goto end;
	}
	ofmt = ofmt_ctx->oformat;

	for (i = 0; i < ifmt_ctx_v->nb_streams; i++) 
	{
		//���������������������Create output AVStream according to input AVStream��
		if(ifmt_ctx_v->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO)
		{
			AVStream *in_stream = ifmt_ctx_v->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			videoindex_v=i;
			if (!out_stream) 
			{
				printf( "Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			videoindex_out=out_stream->index;
			//����AVCodecContext�����ã�Copy the settings of AVCodecContext��
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) 
			{
				printf( "Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
			break;
		}
	}

	for (i = 0; i < ifmt_ctx_a->nb_streams; i++) 
	{
		//���������������������Create output AVStream according to input AVStream��
		if(ifmt_ctx_a->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO)
		{
			AVStream *in_stream = ifmt_ctx_a->streams[i];
			AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
			audioindex_a=i;
			if (!out_stream) 
			{
				printf( "Failed allocating output stream\n");
				ret = AVERROR_UNKNOWN;
				goto end;
			}
			audioindex_out=out_stream->index;
			//����AVCodecContext�����ã�Copy the settings of AVCodecContext��
			if (avcodec_copy_context(out_stream->codec, in_stream->codec) < 0) 
			{
				printf( "Failed to copy context from input to output stream codec context\n");
				goto end;
			}
			out_stream->codec->codec_tag = 0;
			if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
				out_stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;

			break;
		}
	}

	//���һ�¸�ʽ------------------
	printf("Output Information====================\n");
	av_dump_format(ofmt_ctx, 0, out_filename, 1);
	printf("======================================\n");
	//������ļ���Open output file��
	if (!(ofmt->flags & AVFMT_NOFILE)) 
	{
		if (avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE) < 0) 
		{
			goto end;
		}
	}
	//д�ļ�ͷ��Write file header��
	if (avformat_write_header(ofmt_ctx, NULL) < 0) 
	{
		goto end;
	}

	//FIX
#if USE_H264BSF
	AVBitStreamFilterContext* h264bsfc =  av_bitstream_filter_init("h264_mp4toannexb"); 
#endif
#if USE_AACBSF
	AVBitStreamFilterContext* aacbsfc =  av_bitstream_filter_init("aac_adtstoasc"); 
#endif

	while (1) 
	{
		AVFormatContext *ifmt_ctx;
		int stream_index=0;
		AVStream *in_stream, *out_stream;

		//��ȡһ��AVPacket��Get an AVPacket��
		if(av_compare_ts(cur_pts_v,ifmt_ctx_v->streams[videoindex_v]->time_base,cur_pts_a,ifmt_ctx_a->streams[audioindex_a]->time_base) <= 0)
		{
			ifmt_ctx=ifmt_ctx_v;
			stream_index=videoindex_out;

			if(av_read_frame(ifmt_ctx, &pkt) >= 0)
			{
				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if(pkt.stream_index==videoindex_v)
					{
						//FIX��No PTS (Example: Raw H.264)
						//Simple Write PTS
						if(pkt.pts==AV_NOPTS_VALUE)
						{
							//Write PTS
							AVRational time_base1=in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}

						cur_pts_v=pkt.pts;
						break;
					}
				}while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else
			{
				break;
			}
		}
		else
		{
			ifmt_ctx=ifmt_ctx_a;
			stream_index=audioindex_out;
			if(av_read_frame(ifmt_ctx, &pkt) >= 0)
			{
				do{
					in_stream  = ifmt_ctx->streams[pkt.stream_index];
					out_stream = ofmt_ctx->streams[stream_index];

					if(pkt.stream_index==audioindex_a)
					{
						//FIX��No PTS
						//Simple Write PTS
						if(pkt.pts==AV_NOPTS_VALUE)
						{
							//Write PTS
							AVRational time_base1=in_stream->time_base;
							//Duration between 2 frames (us)
							int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(in_stream->r_frame_rate);
							//Parameters
							pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							pkt.dts=pkt.pts;
							pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
							frame_index++;
						}
						cur_pts_a=pkt.pts;

						break;
					}
				}while(av_read_frame(ifmt_ctx, &pkt) >= 0);
			}
			else
			{
				break;
			}
		}

		//FIX:Bitstream Filter
#if USE_H264BSF
		av_bitstream_filter_filter(h264bsfc, in_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
#if USE_AACBSF
		av_bitstream_filter_filter(aacbsfc, out_stream->codec, NULL, &pkt.data, &pkt.size, pkt.data, pkt.size, 0);
#endif
		/* copy packet */
		//ת��PTS/DTS��Convert PTS/DTS��
		pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
		pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
		pkt.pos = -1;
		pkt.stream_index=stream_index;

		printf("Write 1 Packet. size:%5d\tpts:%8d\n",pkt.size,pkt.pts);
		//д�루Write��
		if (av_interleaved_write_frame(ofmt_ctx, &pkt) < 0) 
		{
			break;
		}
		av_free_packet(&pkt);

	}
	//д�ļ�β��Write file trailer��
	av_write_trailer(ofmt_ctx);

#if USE_H264BSF
	av_bitstream_filter_close(h264bsfc);
#endif
#if USE_AACBSF
	av_bitstream_filter_close(aacbsfc);
#endif

end:
	avformat_close_input(&ifmt_ctx_v);
	avformat_close_input(&ifmt_ctx_a);
	/* close output */
	if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
		avio_close(ofmt_ctx->pb);
	avformat_free_context(ofmt_ctx);
	if (ret < 0 && ret != AVERROR_EOF) 
	{
		printf( "Error occurred.\n");
	}

}



 void MyCollectDesktop::Close()
 {
	 m_bQuit = false;
	 if(m_hThread)
	 {
		  if(WAIT_TIMEOUT == WaitForSingleObject(m_hThread,100))
		 TerminateThread(m_hThread,-1);

		  CloseHandle(m_hThread);
		  m_hThread = NULL;

	 }
 }

 void MyCollectDesktop::UnInitial()
 {

	if(img_convert_ctx)
    sws_freeContext(img_convert_ctx);

    av_free(out_buffer);
    av_free(pFrame);
    av_free(pFrameYUV);
    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);
 }