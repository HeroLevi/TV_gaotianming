#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H

#include "IKernel.h"
#include "TCPNet.h"
#include "CMySql.h"
#include "MyThreadPool.h"
#include "Packdef.h"
#include <list>
#include <map>

#pragma comment(lib,"ThreadPool.lib")
class TCPKernel;
typedef void (TCPKernel:: *PFUN)(SOCKET ,char*);
struct ProtocolMap
{
    PackType m_nType;
	PFUN     m_pfun;
};

//主播 （主播名、房间号、房间名、sock,信号量，缓冲区，偏移量）
struct Author_Info
{
	char m_szName[_DEF_SIZE];
	char m_szRoomName[_DEF_SIZE];
	long long m_roomid;
	SOCKET m_sock;
	HANDLE m_hSemaphore;
	char   m_szAuthorBuffer[_DEF_AUTHORSIZE];
	int    m_noffset;


};
//观众信息(sock，事件，偏移量)
struct Audience_Info
{
	SOCKET m_sock;
	HANDLE m_hEvent;
	int    m_noffset;
};
class TCPKernel :public IKernel
{
public:
	TCPKernel();
	virtual ~TCPKernel();
public:
	bool Open();
	void Close();
	void DealData(SOCKET sock,char* szbuf);
public:
	void RegisterRq(SOCKET sock,char* szbuf);
	void LoginRq(SOCKET sock,char* szbuf);
	void GetRoomInfoRq(SOCKET sock,char* szbuf);
	void SetRoomInfoRq(SOCKET sock,char* szbuf);
	void StartTransferRq(SOCKET sock,char* szbuf);
	void VideoStreamInfoRq(SOCKET sock,char* szbuf);
	void VideoStreamRq(SOCKET sock,char* szbuf);
public:
	void SelectAuthorRq(SOCKET sock,char* szbuf);
	void GetAuthorlistRq(SOCKET sock,char* szbuf);
public:
	INet *m_pTcpNet;
private:
	CMySql m_sql;
	CMyThreadPool m_threadpool;
	std::list<Author_Info*> m_lstAuthor;
	std::list<Audience_Info*> m_lstAudience;
	std::map<SOCKET,Audience_Info*> m_mapSocketToAudienceInfo;
	std::map<SOCKET,Author_Info*> m_mapSocketToAuthorInfo;
	std::map<Author_Info*,std::list<Audience_Info*>> m_mapAuthorToAudienceList;

};


class VideoItask :public Itask
{
public:
	VideoItask(Audience_Info* pAudience,Author_Info*   pAuthor,TCPKernel*     pKernel)
	{
		m_pAudience = pAudience;
		m_pAuthor = pAuthor;
		m_pKernel = pKernel;
	}
	~VideoItask(){}
	void RunItask();
private:
Audience_Info* m_pAudience;
Author_Info*   m_pAuthor;
TCPKernel*     m_pKernel;

};
#endif