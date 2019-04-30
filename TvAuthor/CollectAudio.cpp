#include "stdafx.h"
#include "CollectAudio.h"
#include "0830Author.h"
#include "0830AuthorDlg.h"
#include "DlgMain.h"
#include "Myffmpeg.h"
CCollectAudio::CCollectAudio(Myffmpeg *pffmpeg)
{
	m_pMMDevice = NULL;
	m_hStartedEvent = NULL;
	m_hStopEvent = NULL;
	m_bflagchangefile = false;
	m_hthreadaudioendoce = NULL;
	m_eventaudioencode = NULL;
	m_threadsound = NULL;
	m_bflagfinishaudio = false;
	file = NULL;
	m_EventAudiomp4 = CreateEvent(NULL,FALSE,FALSE,NULL);
	m_pffmpeg = pffmpeg;
}

CCollectAudio::~CCollectAudio(void)
{
}

IMMDevice* CCollectAudio::GetDefaultDevice()
{
	IMMDevice* pDevice = NULL;
	IMMDeviceEnumerator *pMMDeviceEnumerator = NULL;
	HRESULT hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator);
	if(FAILED(hr)) return NULL;

  
	hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pDevice); // 采集麦克风 //
	hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eCapture, eMultimedia, &pDevice); 
  //  pMMDeviceEnumerator->Release();
	
	return pDevice;
}

void CCollectAudio::Initial()
{
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	this->m_pMMDevice = this->GetDefaultDevice();
	this->m_hStartedEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    this->m_hStopEvent = CreateEvent(NULL,TRUE,FALSE,NULL);

	this->myaudioencode.Initial();
}

DWORD WINAPI CCollectAudio::ThreadSound(LPVOID lpParameter)
{
    CoUninitialize();
	AudioStruct *pArgs = (AudioStruct *)lpParameter;
	CoInitialize(NULL);
	
	((CDlgMain*)theApp.m_pMainWnd)->m_ffmpeg.m_pAudio->CollectAudio(pArgs->pMMDevice,pArgs->bInt16,pArgs->hStartedEvent,pArgs->hStopEvent,&pArgs->pnFrames);

    CoUninitialize();

	return 0;
}
typedef LONGLONG REFERENCE_TIME;
HRESULT CCollectAudio::CollectAudio(IMMDevice *pMMDevice,bool bInt16,HANDLE hStartedEvent,HANDLE hStopEvent,PUINT32 pnFrames)
{
	//接受返回值
	HRESULT hr;
	//activate an IAudioClient
    IAudioClient *pAudioClient = NULL;
    hr = pMMDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (void**)&pAudioClient);
    if (FAILED(hr)) 
	{
		
        return hr;
    }

	// get the default device periodicity
    REFERENCE_TIME hnsDefaultDevicePeriod;
    hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod,NULL);
    if (FAILED(hr)) 
	{
		
        pAudioClient->Release();
        return hr;
    }

	 // get the default device format
    WAVEFORMATEX *pwfx;
    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr))
	{
		
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        return hr;
    }

	if (bInt16)
	{
        // coerce int-16 wave format
        // can do this in-place since we're not changing the size of the format
        // also, the engine will auto-convert from float to int for us
        switch (pwfx->wFormatTag)
		{
            case WAVE_FORMAT_IEEE_FLOAT:
                pwfx->wFormatTag = WAVE_FORMAT_PCM;
                pwfx->wBitsPerSample = 16;
                pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                break;
            case WAVE_FORMAT_EXTENSIBLE:
                {
                    // naked scope for case-local variable
                    PWAVEFORMATEXTENSIBLE pEx = reinterpret_cast<PWAVEFORMATEXTENSIBLE>(pwfx);
                    if (IsEqualGUID(KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pEx->SubFormat)) 
					{
                        pEx->SubFormat = KSDATAFORMAT_SUBTYPE_PCM;
                        pEx->Samples.wValidBitsPerSample = 16;
                        pwfx->wBitsPerSample = 16;
                        pwfx->nBlockAlign = pwfx->nChannels * pwfx->wBitsPerSample / 8;
                        pwfx->nAvgBytesPerSec = pwfx->nBlockAlign * pwfx->nSamplesPerSec;
                    }
					else
					{
						
                        CoTaskMemFree(pwfx);
                        pAudioClient->Release();
                        return E_UNEXPECTED;
                    }
                }
                break;
            default:
			
                CoTaskMemFree(pwfx);
                pAudioClient->Release();
                return E_UNEXPECTED;
        }
    }

	// WriteWaveHeader
 //   MMCKINFO ckRIFF = {0};
 //   MMCKINFO ckData = {0};
 //   hr = WriteWaveHeader(hFile, pwfx, &ckRIFF, &ckData);
 //   if (FAILED(hr)) 
	//{
 //       // WriteWaveHeader does its own logging
 //       CoTaskMemFree(pwfx);
 //       pAudioClient->Release();
 //       return hr;
 //   }

	// create a periodic waitable timer
    HANDLE hWakeUp = CreateWaitableTimer(NULL, FALSE, NULL);
    if (NULL == hWakeUp)
	{
        DWORD dwErr = GetLastError();
		
        CoTaskMemFree(pwfx);
        pAudioClient->Release();
        return HRESULT_FROM_WIN32(dwErr);
    }

	UINT32 nBlockAlign = pwfx->nBlockAlign;
    *pnFrames = 0;//#define REFTIMES_PER_SEC (10000000) 
     LONGLONG hnsRequestedDuration = (10000000) ; 
    hr = pAudioClient->Initialize(
		AUDCLNT_SHAREMODE_SHARED,
		 /*AUDCLNT_STREAMFLAGS_EVENTCALLBACK |*/ AUDCLNT_STREAMFLAGS_NOPERSIST,
		hnsRequestedDuration,
		0,
		pwfx,
		0);
    if (FAILED(hr)) 
	{
		
        CloseHandle(hWakeUp);
        pAudioClient->Release();
        return hr;
    }
   // CoTaskMemFree(pwfx);
	/*TOTEST*/
	 REFERENCE_TIME hnsStreamLatency;
		 hr = pAudioClient->GetStreamLatency(&hnsStreamLatency); 
	//REFERENCE_TIME hnsDefaultDevicePeriod; 
    REFERENCE_TIME hnsMinimumDevicePeriod; 
	 hr = pAudioClient->GetDevicePeriod(&hnsDefaultDevicePeriod, &hnsMinimumDevicePeriod); 
		
    // activate an IAudioCaptureClient
    IAudioCaptureClient *pAudioCaptureClient;
    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient),(void**)&pAudioCaptureClient);
	 
    if (FAILED(hr)) 
	{
		
        CloseHandle(hWakeUp);
        pAudioClient->Release();
        return hr;
    }

	// register with MMCSS
    DWORD nTaskIndex = 0;
    HANDLE hTask = AvSetMmThreadCharacteristics(L"Capture",&nTaskIndex);
    if (NULL == hTask) 
	{
        DWORD dwErr = GetLastError();
	
        pAudioCaptureClient->Release();
        CloseHandle(hWakeUp);
        pAudioClient->Release();
        return HRESULT_FROM_WIN32(dwErr);
    }    

    // set the waitable timer
    LARGE_INTEGER liFirstFire;
    liFirstFire.QuadPart = -hnsDefaultDevicePeriod / 2; // negative means relative time
    LONG lTimeBetweenFires = (LONG)hnsDefaultDevicePeriod / 2 / (10 * 1000); // convert to milliseconds
    BOOL bOK = SetWaitableTimer(hWakeUp,&liFirstFire,lTimeBetweenFires,NULL,NULL,FALSE);
    if (!bOK)
	{
        DWORD dwErr = GetLastError();
		
        AvRevertMmThreadCharacteristics(hTask);
        pAudioCaptureClient->Release();
        CloseHandle(hWakeUp);
        pAudioClient->Release();
        return HRESULT_FROM_WIN32(dwErr);
    }

	// call IAudioClient::Start
    hr = pAudioClient->Start();
    if (FAILED(hr))
	{
	
        AvRevertMmThreadCharacteristics(hTask);
        pAudioCaptureClient->Release();
        CloseHandle(hWakeUp);
        pAudioClient->Release();
        return hr;
    }
    SetEvent(hStartedEvent);

	// loopback capture loop
    HANDLE waitArray[2] = {hStopEvent,hWakeUp};
    DWORD dwWaitResult;

    bool bDone = false;
	//FILE *filepcm;

	UINT32 nNextPacketSize;
	BYTE *pData;
    UINT32 nNumFramesToRead;
    DWORD dwFlags;

	//首先打开audio.pcm
	this->m_bflagchangefile = false;
	
	DWORD  dwOldTime = GetTickCount();
	int nCount = 0;
	for (UINT32 nPasses = 0; !bDone; nPasses++) 
	{ 
		if(nCount ==0){
			file = mmioOpen(_T("../TempFile/audio.pcm"),NULL,MMIO_CREATE|MMIO_WRITE);
			nCount = 1;
		}

		dwWaitResult = WaitForMultipleObjects(sizeof(waitArray)/sizeof(waitArray[0]),waitArray,FALSE,INFINITE);
		if (WAIT_OBJECT_0 == dwWaitResult)
		{
			bDone = true;
			continue; // exits loop
		}

		if (WAIT_OBJECT_0 + 1 != dwWaitResult) 
		{
			
			pAudioClient->Stop();
			CancelWaitableTimer(hWakeUp);
			AvRevertMmThreadCharacteristics(hTask);
			pAudioCaptureClient->Release();
			CloseHandle(hWakeUp);
			pAudioClient->Release();
			return E_UNEXPECTED;
		}

		// got a "wake up" event - see if there's data
		//
		if(nPasses == 2000)
		{
			DWORD dwTime =  (GetTickCount() - dwOldTime)/1000;
			mmioClose(file,0);
			file = NULL;
			nPasses = 0;

			this->m_bflagchangefile = !this->m_bflagchangefile;

			//完成pcm编码
		    myaudioencode.AudioEncode();
			SetEvent(m_EventAudiomp4);
			WaitForSingleObject(m_pffmpeg->m_pDesktop->m_hEventDesktopmp4,INFINITE);
			nCount = 0;
			//SetEvent(this->m_eventaudioencode);
		}
        
		hr = pAudioCaptureClient->GetNextPacketSize(&nNextPacketSize);
		if (FAILED(hr)) 
		{
		
			pAudioClient->Stop();
			CancelWaitableTimer(hWakeUp);
			AvRevertMmThreadCharacteristics(hTask);
			pAudioCaptureClient->Release();
			CloseHandle(hWakeUp);
			pAudioClient->Release();            
			return hr;
		}

		if (0 == nNextPacketSize)
		{
			// no data yet
			continue;
		}

		// get the captured data

		hr = pAudioCaptureClient->GetBuffer(&pData,&nNumFramesToRead,&dwFlags,NULL,NULL);
		if (FAILED(hr)) 
		{
			
			pAudioClient->Stop();
			CancelWaitableTimer(hWakeUp);
			AvRevertMmThreadCharacteristics(hTask);
			pAudioCaptureClient->Release();
			CloseHandle(hWakeUp);
			pAudioClient->Release();            
			return hr;            
		}

		if (0 == nNumFramesToRead) 
		{
		
			pAudioClient->Stop();
			CancelWaitableTimer(hWakeUp);
			AvRevertMmThreadCharacteristics(hTask);
			pAudioCaptureClient->Release();
			CloseHandle(hWakeUp);
			pAudioClient->Release();            
			return E_UNEXPECTED;            
		}

		mmioWrite(file,reinterpret_cast<char _huge *>(pData),nNumFramesToRead * nBlockAlign);
        
		hr = pAudioCaptureClient->ReleaseBuffer(nNumFramesToRead);
		if (FAILED(hr))
		{
		
			pAudioClient->Stop();
			CancelWaitableTimer(hWakeUp);
			AvRevertMmThreadCharacteristics(hTask);
			pAudioCaptureClient->Release();
			CloseHandle(hWakeUp);
			pAudioClient->Release();            
			return hr;            
		}
       
	} // capture loop

	if(file != NULL)
	{
		mmioClose(file,0);
		file = NULL;
		this->m_bflagchangefile = !this->m_bflagchangefile;
		SetEvent(this->m_eventaudioencode);
	}

	//endding
    pAudioClient->Stop();
    CancelWaitableTimer(hWakeUp);
    AvRevertMmThreadCharacteristics(hTask);
    pAudioCaptureClient->Release();
    CloseHandle(hWakeUp);
    pAudioClient->Release();

    return hr;
}
AudioStruct as;
void CCollectAudio::Run()
{
	this->m_eventaudioencode = CreateEvent(NULL,FALSE,FALSE,NULL);
	this->m_hthreadaudioendoce = CreateThread(NULL,0,&Threadaudioencode,this,0,NULL);

	
	as.pMMDevice = this->m_pMMDevice;
	as.bInt16 = true;
	as.hStartedEvent = this->m_hStartedEvent;
	as.hStopEvent = this->m_hStopEvent;
	as.pnFrames = 0;

	m_threadsound = CreateThread(NULL,0,&ThreadSound,&as,0,NULL);

}

DWORD WINAPI CCollectAudio::Threadaudioencode(LPVOID lpParameter)
{
	CCollectAudio * pthis = (CCollectAudio *)lpParameter;

	while(1)
	{
		if(WAIT_OBJECT_0 == WaitForSingleObject(pthis->m_eventaudioencode,INFINITE))
		{
			//进行AAC音频编码 0表示编码成功
		
			if(pthis->myaudioencode.AudioEncode() == 0)
			{
				pthis->m_bflagfinishaudio = true;
			}
		}
	}
	return 0;
}

void CCollectAudio::Close()
{
	SetEvent(m_hStopEvent);

	if(WAIT_TIMEOUT == WaitForSingleObject(this->m_threadsound,100))
	{
		TerminateThread(this->m_threadsound,-1);
	}

	if(this->m_threadsound)
	{
		CloseHandle(this->m_threadsound);
		this->m_threadsound = NULL;
	}

	if(file != NULL)
	{
		mmioClose(file,0);
		file = NULL;
		this->m_bflagchangefile = !this->m_bflagchangefile;
		SetEvent(this->m_eventaudioencode);
	}

	if(WAIT_TIMEOUT == WaitForSingleObject(this->m_hthreadaudioendoce,500))
	{
		TerminateThread(this->m_hthreadaudioendoce,-1);
	}

	if(this->m_hthreadaudioendoce)
	{
		CloseHandle(this->m_hthreadaudioendoce);
		this->m_hthreadaudioendoce = NULL;
	}

	if(m_eventaudioencode)
	{
		CloseHandle(m_eventaudioencode);
		m_eventaudioencode = NULL;
	}
}

void CCollectAudio::Destroy()
{
	if(m_hStartedEvent)
	{
		CloseHandle(m_hStartedEvent);
		m_hStartedEvent = NULL;
	}

	if(m_hStopEvent)
	{
		CloseHandle(m_hStopEvent);
		m_hStopEvent = NULL;
	}
}