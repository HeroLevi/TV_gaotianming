#pragma once
#include "MyCollectDesktop.h"
#include "CollectAudio.h"
#include "CollectCamera.h"
class Myffmpeg
{
public:
	Myffmpeg(void);
	~Myffmpeg(void);

public:
	void Unitffmpeg();
	void Factory(bool desk =false,bool camera=false,bool audio=false,bool microphone=false);
	void SetStart(bool desk =false,bool camera =false,bool audio =false,bool microphone =false);
	void SetStop(bool desk =false,bool camera =false,bool audio =false,bool microphone =false);
public:
	MyCollectDesktop *m_pDesktop;
	CCollectAudio    *m_pAudio;
	CCollectCamera   *m_pCamera;
};

