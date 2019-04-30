
#include "TCPNet.h"

bool  TCPNet::m_bFlagQuit = true;
IKernel *TCPNet::m_pKernel = NULL;
TCPNet::TCPNet(IKernel *pKernel)
{
	m_sockListen = NULL;
	m_pKernel = pKernel;
	

}


TCPNet::~TCPNet(void)
{
}
bool TCPNet::InitNetWork()
{
	//1.选择种类 -- 
	 WORD wVersionRequested;
    WSADATA wsaData;
    int err;

/* Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h */
    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {
        /* Tell the user that we could not find a usable */
        /* Winsock DLL.                                  */
       // printf("WSAStartup failed with error: %d\n", err);
        return false;
    }


    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
       // printf("Could not find a usable version of Winsock.dll\n");
        UnInitNetWork();
        return false;
    }
   
        

	//2.雇佣店长 -- socket 
	m_sockListen = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP );
	if(m_sockListen == INVALID_SOCKET )
	{
		UnInitNetWork();
		 return false;
	}
	//3.选址 --- 
	sockaddr_in  addrServer;
	addrServer.sin_family = AF_INET;
	addrServer.sin_addr.S_un.S_addr = /*INADDR_ANY/**/inet_addr("127.0.0.1");
	addrServer.sin_port = htons(_DEFPORT);
	if(SOCKET_ERROR ==bind(m_sockListen,(const sockaddr*)&addrServer,sizeof(addrServer)))
	{
		 UnInitNetWork();
		 return false;
	}
	//4.店长发宣传 -- 
	if(SOCKET_ERROR  == listen(m_sockListen,20))
	{
		 UnInitNetWork();
		 return false;

	}

	//5.创建线程
	m_hThreadAccept = (HANDLE)_beginthreadex(NULL,0,&ThreadAccept,this,0,0);
	
	return true;
}

unsigned _stdcall TCPNet::ThreadAccept( void * lpvoid)
{
	TCPNet *pthis = (TCPNet*)lpvoid;
	
	while(pthis->m_bFlagQuit)
	{

		  SOCKET sockWaiter =  accept(pthis->m_sockListen,NULL,NULL);

		  if(sockWaiter == INVALID_SOCKET)
			  continue;
		  HANDLE m_hThreadRecv = (HANDLE)_beginthreadex(NULL,0,&ThreadRecv,(void * )sockWaiter,0,0);
		  if(m_hThreadRecv)
		  {
			   pthis->m_lsthThreadRecv.push_back(m_hThreadRecv);
		  }
		//接收数据 
	}
	
	return 0;
}
unsigned _stdcall TCPNet::ThreadRecv( void * lpvoid)
{
	SOCKET sockWaiter = (SOCKET)lpvoid;
	char* pszbuf = NULL;
	int nRelReadNum;
	int nPackSize;
	while(TCPNet::m_bFlagQuit)
	{
		
		//接收包大小
		nRelReadNum =  recv(sockWaiter,(char*)&nPackSize,sizeof(int),0);
		if(nRelReadNum <=0)
		{
			//判断是不是客户端下线
			if(WSAECONNRESET == GetLastError())
			{
				closesocket(sockWaiter);
				sockWaiter = NULL;
				break;
			}
			continue;
		}
			
		//接收完整数据包
		 pszbuf = new char[nPackSize];
		 int noffset = 0;
		 while(nPackSize)
		 {
			nRelReadNum =  recv(sockWaiter,pszbuf+noffset,nPackSize,0);
			noffset += nRelReadNum;
			nPackSize -= nRelReadNum;
	 
		 }
	 
		//交给核心处理类处理
		 m_pKernel->DealData(sockWaiter,pszbuf);

		 delete []pszbuf;
		 pszbuf =  NULL;


	}
	return 0;
}

void TCPNet::UnInitNetWork()
{
	m_bFlagQuit = false;
	if(m_hThreadAccept )
	{
		if(WAIT_TIMEOUT == WaitForSingleObject(m_hThreadAccept,100))
			TerminateThread(m_hThreadAccept,-1);
		
		CloseHandle(m_hThreadAccept);
		m_hThreadAccept = NULL;

	
	}
	auto ite = m_lsthThreadRecv.begin();
	while(ite != m_lsthThreadRecv.end())
	{
		if(WAIT_TIMEOUT == WaitForSingleObject(*ite,100))
		TerminateThread(*ite,-1);
		
		CloseHandle(*ite);
		*ite = NULL;
		ite++;
	}
	m_lsthThreadRecv.clear();
	WSACleanup();
	if(m_sockListen)
	{
		closesocket(m_sockListen);
		m_sockListen = NULL;
	}
}

bool TCPNet::SendData(SOCKET sock,char* szbuf,int nlen)
{
	if(!sock || !szbuf || nlen <=0)
		return false;

	//发送包大小
	if(send(sock,(const char*)&nlen,sizeof(int),0)<0)
	return false;

	//包内容
	if(send(sock,szbuf,nlen,0)<0)
		return false;
	
	return true;
}