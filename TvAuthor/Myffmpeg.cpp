#include "stdafx.h"
#include "Myffmpeg.h"


Myffmpeg::Myffmpeg(void)
{
	m_pDesktop = NULL;
	m_pAudio = NULL;
	m_pCamera = NULL;
}


Myffmpeg::~Myffmpeg(void)
{
	if(m_pDesktop)
	{
		delete m_pDesktop;
		m_pDesktop = NULL;
	}
	if(m_pAudio)
	{
		delete m_pAudio;
		m_pAudio = NULL;
	}
	if(m_pCamera)
	{
		delete m_pCamera;
		m_pCamera = NULL;
	}
}


void Myffmpeg::Factory(bool desk,bool camera,bool audio,bool microphone)
{
	if(desk == true && m_pDesktop == NULL)
	{
		m_pDesktop = new MyCollectDesktop(this);
		m_pDesktop->Initial();
	}
	if(audio == true && m_pAudio == NULL)
	{
		m_pAudio = new CCollectAudio(this);
		m_pAudio->Initial();
	}
	if(camera == true && m_pCamera == NULL)
	{
		m_pCamera = new CCollectCamera(_T("video=USB2.0 VGA UVC WebCam"));
		m_pCamera->Start();
	}

}

void Myffmpeg::SetStart(bool desk,bool camera,bool audio,bool microphone)
{
	if(desk == true && m_pDesktop)
	{
		m_pDesktop->Run();
	}
	if(audio == true && m_pAudio)
	{
		m_pAudio->Run();
	}
	

}
void Myffmpeg::SetStop(bool desk,bool camera,bool audio,bool microphone)
{
	if(desk == true)
	{
		m_pDesktop->Close();
	}
	if(audio == true)
	{
		m_pAudio->Close();
	}
	if(camera == true)
	{
		m_pCamera->Stop();
	}

}



void Myffmpeg::Unitffmpeg()
{
	m_pDesktop->UnInitial();
	
}