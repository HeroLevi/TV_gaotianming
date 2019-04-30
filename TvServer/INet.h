#ifndef _INET_H
#define _INET_H

#include <Winsock2.h>
#pragma comment(lib,"Ws2_32.lib")

class INet 
{
public:
	INet(){}
	virtual ~INet(){}
public:
	virtual bool InitNetWork() = 0;
	virtual void UnInitNetWork() = 0;
	virtual bool SendData(SOCKET sock,char* szbuf,int nlen) = 0;


};

#endif