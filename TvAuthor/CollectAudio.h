#pragma once
#ifndef COLLECTAUDIO_H__
#define COLLECTAUDIO_H__

#include "AudioEncode.h"

#include <iostream>
#include <Mmdeviceapi.h>
#include <Mmsystem.h>
#include <Mmreg.h>
#include <Audioclient.h>
#include <Avrt.h>
#include <process.h>
using namespace std;

#pragma comment(lib,"Winmm.lib")
#pragma comment(lib,"Avrt.lib")

typedef struct AUDIOSTRUCT
{
	IMMDevice * pMMDevice;
    bool bInt16;
    HANDLE hStartedEvent;
    HANDLE hStopEvent;
    UINT32 pnFrames;
}AudioStruct;

class Myffmpeg;
class CCollectAudio
{
public:
	CCollectAudio(Myffmpeg *pffmpeg);
	~CCollectAudio(void);
//线程函数
public:
	static DWORD WINAPI ThreadSound(LPVOID lpParameter);
	HANDLE m_hStartedEvent;
    HANDLE m_hStopEvent;
	HANDLE m_threadsound;
	HANDLE m_EventAudiomp4;
	Myffmpeg *m_pffmpeg;
//录音
public:
	IMMDevice * m_pMMDevice;
	IMMDevice* GetDefaultDevice();
	bool m_bflagchangefile;
	HRESULT CollectAudio(IMMDevice *pMMDevice,bool bInt16,HANDLE hStartedEvent,HANDLE hStopEvent,PUINT32 pnFrames);
	HMMIO file;
//AAC编码类
public:
	CAudioEncode myaudioencode;
	HANDLE m_hthreadaudioendoce;
	HANDLE m_eventaudioencode;
	static DWORD WINAPI Threadaudioencode(LPVOID lpParameter);
	bool m_bflagfinishaudio;
//封装函数
	void Initial();
	void Run();
	void Close();
	void Destroy();
};

#endif