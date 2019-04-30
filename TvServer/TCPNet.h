#ifndef  _TCPNET_H
#define _TCPNET_H


#include <process.h>
#include <list>
#include "INet.h"
#include "IKernel.h"
#define _DEFPORT    1234
#define _DEFSIZE     1024

class TCPNet :public INet
{
public:
	TCPNet(IKernel *pKernel);
	virtual ~TCPNet();
public:
	bool InitNetWork();
	void UnInitNetWork();
	bool SendData(SOCKET sock,char* szbuf,int nlen);
public:
	static  unsigned _stdcall ThreadAccept( void * );
	static  unsigned _stdcall ThreadRecv( void * );
private:
	SOCKET m_sockListen;
	std::list<HANDLE> m_lsthThreadRecv;
	HANDLE m_hThreadAccept;
	static bool   m_bFlagQuit;
	static IKernel *m_pKernel;
	
	
};





#endif
