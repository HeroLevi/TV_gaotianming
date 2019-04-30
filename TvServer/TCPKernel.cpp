#include "TCPKernel.h"


void VideoItask::RunItask()
{
	//等着向当前观众推流
	HANDLE hary[] = {m_pAuthor->m_hSemaphore,m_pAudience->m_hEvent};
	DWORD  dwIndex;
	while(1)
	{
		//等信号、事件
		 dwIndex = WaitForMultipleObjects(2,hary,FALSE,INFINITE);
		 dwIndex -=  WAIT_OBJECT_0;

		 if(dwIndex == 0 )
		 {
			 //将流推向观众
			 m_pKernel->m_pTcpNet->SendData(m_pAudience->m_sock,m_pAuthor->m_szAuthorBuffer+m_pAudience->m_noffset*_DEF_VIDEOSIZE,_DEF_VIDEOSIZE);

			 m_pAudience->m_noffset = (m_pAudience->m_noffset+1)%_DEF_BUFFERNUM;

		 }
		 else  if(dwIndex == 1)
			 break;
		


	}
}



TCPKernel::TCPKernel()
{
	m_pTcpNet = new TCPNet(this);
}

TCPKernel::~TCPKernel()
{
	if(m_pTcpNet)
	{
		delete m_pTcpNet;
		m_pTcpNet = NULL;
	}
}

BEGIN_PROTOCOL_MAP
	PM(_DEF_PROTOCOL_REGISTER_RQ,&TCPKernel::RegisterRq)
	PM(_DEF_PROTOCOL_LOGIN_RQ,&TCPKernel::LoginRq)
	PM(_DEF_PROTOCOL_GETROOMINFO_RQ,&TCPKernel::GetRoomInfoRq)
	PM(_DEF_PROTOCOL_SETROOMINFO_RQ,&TCPKernel::SetRoomInfoRq)
	PM(_DEF_PROTOCOL_STARTTRANSFER_RQ,&TCPKernel::StartTransferRq)
	PM(_DEF_PROTOCOL_VIDEOSTREAMINFO_RQ,&TCPKernel::VideoStreamInfoRq)
	PM(_DEF_PROTOCOL_VIDEOSTREAM_RQ,&TCPKernel::VideoStreamInfoRq)
	PM(_DEF_PROTOCOL_GETAUTHORLIST_RQ,&TCPKernel::GetAuthorlistRq)
	PM(_DEF_PROTOCOL_SELECTAUTHOR_RQ,&TCPKernel::SelectAuthorRq)
END_PROTOCOL_MAP

//观众
void TCPKernel::SelectAuthorRq(SOCKET sock,char* szbuf)
{
	STRU_SELECTAUTHOR_RQ *pssr = (STRU_SELECTAUTHOR_RQ*)szbuf;
	Audience_Info* pAudienceInfo =  m_mapSocketToAudienceInfo[sock];
	//选择主播
	auto ite = m_lstAuthor.begin();
	while( ite != m_lstAuthor.end())
	{
		if(0 == strcmp( pssr->m_szAuthorName,(*ite)->m_szName))
		{
			pAudienceInfo->m_noffset = (*ite)->m_noffset;
			break;
		}
			
		ite++;
	}
	Author_Info *pInfo = *ite;
	m_mapAuthorToAudienceList[pInfo].push_back(pAudienceInfo);
	//向线程池中投递任务(观众信息,主播，)
	Itask *pNew = new VideoItask(pAudienceInfo,pInfo,this);
	m_threadpool.Push(pNew);


}


void TCPKernel::GetAuthorlistRq(SOCKET sock,char* szbuf)
{

	STRU_GETAUTHORLIST_RQ *psgr= (STRU_GETAUTHORLIST_RQ*)szbuf;
	STRU_GETAUTHORLIST_RS sgr;
	sgr.m_nType = _DEF_PROTOCOL_GETAUTHORLIST_RS;
	memcpy(sgr.m_szName,psgr->m_szName,_DEF_SIZE);
	auto ite = m_lstAuthor.begin();
	int i = 0;
	while(ite != m_lstAuthor.end())
	{
		sgr.m_aryAuthorInfo[i].m_roomid = (*ite)->m_roomid;
		memcpy(sgr.m_aryAuthorInfo[i].m_szName,(*ite)->m_szName,_DEF_SIZE);
		memcpy(sgr.m_aryAuthorInfo[i].m_szRoomName,(*ite)->m_szRoomName,_DEF_SIZE);
		i++;
		ite++;
		if(i == _DEF_AUTHORNUM  || ite == m_lstAuthor.end() )
		{
			sgr.m_nAuthorNum = i;
			m_pTcpNet->SendData(sock,(char*)&sgr,sizeof(sgr));
			i = 0;
		}
	}

}



//主播

void TCPKernel::StartTransferRq(SOCKET sock,char* szbuf)
{
	//开始推送
	STRU_STARTTRANSFER_RQ *pssr = (STRU_STARTTRANSFER_RQ*)szbuf;
	//将正在直播的主播信息记录--链表（主播名、房间号、房间名、sock,信号量，缓冲区，偏移量）
	char szsql[_DEF_SQLLEN] = {0};
	list<string> lststr;
	sprintf_s(szsql,"select r_id,r_name from myview where a_name = '%s'",pssr->m_szName);
	m_sql.SelectMySql(szsql,2,lststr);
	if(lststr.size() >0)
	{
		string strRoomid = lststr.front();
		lststr.pop_front();
		string strRoomName = lststr.front();
		lststr.pop_front();
		Author_Info *pInfo = new Author_Info;
		pInfo->m_hSemaphore = CreateSemaphore(NULL,0,100000,NULL);
		pInfo->m_noffset = 0;
		pInfo->m_roomid = _atoi64(strRoomid.c_str());
		pInfo->m_sock = sock;
		ZeroMemory(pInfo->m_szAuthorBuffer,sizeof(pInfo->m_szAuthorBuffer));
		memcpy(pInfo->m_szName,pssr->m_szName,_DEF_SIZE);
		memcpy(pInfo->m_szRoomName,strRoomName.c_str(),_DEF_SIZE);

		m_lstAuthor.push_back(pInfo);
		m_mapSocketToAuthorInfo[sock] = pInfo;

	}
	
}

void TCPKernel::VideoStreamInfoRq(SOCKET sock,char* szbuf)
{
	Author_Info *pInfo =  m_mapSocketToAuthorInfo[sock];
	//视频流信息
	STRU_VIDEOSTREAM_RQ *psvr = (STRU_VIDEOSTREAM_RQ*)szbuf;

	//主播--观众  map<主播,list<观众>>
	int nSize =  m_mapAuthorToAudienceList[pInfo].size();
	if(nSize >0)
	{
		memcpy(pInfo->m_szAuthorBuffer+pInfo->m_noffset*_DEF_VIDEOSIZE,szbuf,_DEF_VIDEOSIZE);
		pInfo->m_noffset = (pInfo->m_noffset + 1)%_DEF_BUFFERNUM;

		//释放相应个数的信号量
		ReleaseSemaphore(pInfo->m_hSemaphore,nSize,NULL);
	}
}



void TCPKernel::VideoStreamRq(SOCKET sock,char* szbuf)
{
	//视频流内容
	//主播--观众  map<主播,list<观众>>

	//释放相应个数的信号量

}




void TCPKernel::SetRoomInfoRq(SOCKET sock,char* szbuf)
{
	STRU_SETROOMINFO_RQ *pssr = (STRU_SETROOMINFO_RQ*)szbuf;
	STRU_SETROOMINFO_RS ssr;
	ssr.m_nType = _DEF_PROTOCOL_SETROOMINFO_RS;
	memcpy(ssr.m_szName,pssr->m_szName,sizeof(ssr.m_szName));
	ssr.szResult = _setroominfo_fail;
	char szsql[_DEF_SQLLEN] = {0};
	sprintf_s(szsql,"update myview set r_name = '%s' where a_name ='%s' ",pssr->m_szRoomName,pssr->m_szName);
	if(m_sql.UpdateMySql(szsql))
	{
		ssr.szResult = _setroominfo_success;
	}

	m_pTcpNet->SendData(sock,(char*)&ssr,sizeof(ssr));
}


void TCPKernel::GetRoomInfoRq(SOCKET sock,char* szbuf)
{
	STRU_GETROOMINFO_RQ *psgr = (STRU_GETROOMINFO_RQ*)szbuf;
	char szsql[_DEF_SQLLEN] = {0};
	list<string> lststr;
	STRU_GETROOMINFO_RS sgr;
	sgr.m_nType = _DEF_PROTOCOL_GETROOMINFO_RS;
	memcpy(sgr.m_szName,psgr->m_szName,sizeof(sgr.m_szName));
	sprintf_s(szsql,"select r_id,r_name from myview where a_name ='%s' ;",psgr->m_szName);
	m_sql.SelectMySql(szsql,2,lststr);
	if(lststr.size() >0)
	{
		string strRoomId = lststr.front();
		lststr.pop_front();
		string strRoomName = lststr.front();
		lststr.pop_front();

		sgr.m_roomid = _atoi64(strRoomId.c_str());
		memcpy(sgr.m_szRoomName,strRoomName.c_str(),sizeof(sgr.m_szRoomName));

	}
	//
	m_pTcpNet->SendData(sock,(char*)&sgr,sizeof(sgr));
}

void TCPKernel::LoginRq(SOCKET sock,char* szbuf)
{ 
	//1 2  3
	//在数据库中查找
	//检验密码
	STRU_LOGIN_RQ *pslr = (STRU_LOGIN_RQ*)szbuf;
	char szsql[_DEF_SQLLEN] = {0};
	list<string> lststr;
	STRU_LOGIN_RS slr;
	slr.m_nType = _DEF_PROTOCOL_LOGIN_RS;
	memcpy(slr.m_szName,pslr->m_szName,_DEF_SIZE);
	slr.m_szResult = _login_usernoexists;
	if(pslr->m_role == role_author)
	{
		
		sprintf_s(szsql,"SELECT a_password FROM author where  a_name = '%s'" ,pslr->m_szName);
		m_sql.SelectMySql(szsql,1,lststr);
		if(lststr.size() >0)
		{
			slr.m_szResult = _login_passworderr;

			string strpassword = lststr.front();
			lststr.pop_front();

			if(0 == strcmp(strpassword.c_str(),pslr->m_szPassword))
				slr.m_szResult = _login_success;
		}
	}
	else if(pslr->m_role == role_audience)
	{
		sprintf_s(szsql,"SELECT a_password FROM audience where  a_name = '%s'" ,pslr->m_szName);
		m_sql.SelectMySql(szsql,1,lststr);
		if(lststr.size() >0)
		{
			slr.m_szResult = _login_passworderr;

			string strpassword = lststr.front();
			lststr.pop_front();

			if(0 == strcmp(strpassword.c_str(),pslr->m_szPassword))
			{
				slr.m_szResult = _login_success;
				//记录观众信息(sock，事件，偏移量)
				Audience_Info *pInfo = new Audience_Info;
				pInfo->m_hEvent = WSACreateEvent();
				pInfo->m_noffset = 0;
				pInfo->m_sock = sock;
				m_lstAudience.push_back(pInfo);
				m_mapSocketToAudienceInfo[sock] = pInfo;
			}
		}
	}
	

	m_pTcpNet->SendData(sock,(char*)&slr,sizeof(slr));
}

void TCPKernel::RegisterRq(SOCKET sock,char* szbuf)
{
	STRU_REGISTER_RQ *psrr = (STRU_REGISTER_RQ *)szbuf;
	char szsql[_DEF_SQLLEN] = {0};
	STRU_REGISTER_RS srr;
	srr.m_nType = _DEF_PROTOCOL_REGISTER_RS;
	memcpy(srr.m_szName,psrr->m_szName,_DEF_SIZE);
	list<string> lststr;
	if(psrr->m_role == role_author)
	{
		//检查数据库中是否存在此人
		sprintf_s(szsql,"SELECT a_id FROM author where a_id = %lld or a_name = '%s'" ,psrr->m_tel,psrr->m_szName);
		m_sql.SelectMySql(szsql,1,lststr);


		if(lststr.size() ==0)
		{
			srr.m_szResult = _register_success;
			 //将此人插入到数据库中
			sprintf_s(szsql,"insert into author values(%lld,'%s','%s')",psrr->m_tel,psrr->m_szName,psrr->m_szPassword);
			m_sql.UpdateMySql(szsql);
		     //创建新的房间
			sprintf_s(szsql,"insert into room(r_name,a_id) values('%s',%lld)","YY房间",psrr->m_tel);
			m_sql.UpdateMySql(szsql);
	
		}
		else 
		{
			  //
			string strId = lststr.front();
			lststr.pop_front();

			if(psrr->m_tel == _atoi64(strId.c_str()))
			{
				srr.m_szResult = _register_telexists;
			}
			else
			{
				srr.m_szResult = _register_userexists;
			}

		}
	
	}	
	else if(psrr->m_role == role_audience)
	{
		//检查数据库中是否存在此人
		sprintf_s(szsql,"SELECT a_id FROM audience where a_id = %lld or a_name = '%s'" ,psrr->m_tel,psrr->m_szName);
		m_sql.SelectMySql(szsql,1,lststr);

		if(lststr.size() ==0)
		{
			srr.m_szResult = _register_success;
			 //将此人插入到数据库中
			sprintf_s(szsql,"insert into audience values(%lld,'%s','%s')",psrr->m_tel,psrr->m_szName,psrr->m_szPassword);
			m_sql.UpdateMySql(szsql);
		   
	
		}
		else
		{
			string strId = lststr.front();
			lststr.pop_front();

			if(psrr->m_tel == _atoi64(strId.c_str()))
			{
				srr.m_szResult = _register_telexists;
			}
			else
			{
				srr.m_szResult = _register_userexists;
			}

		}
	   
	}
		
	//回复
	m_pTcpNet->SendData(sock,(char*)&srr,sizeof(srr));
}


bool TCPKernel::Open()
{
	if(!m_sql.ConnectMySql("localhost","root","colin123","live"))
		return false;
	
	if(!m_pTcpNet->InitNetWork())
		return false;


	if(!m_threadpool.CreateThreadPool(1,1000))
		return false;
	
	return true;
}

void TCPKernel::Close()
{
	m_pTcpNet->UnInitNetWork();
	m_threadpool.DestroyThreadPool();

	//释放链表
	auto ite = m_lstAuthor.begin();
	while(ite != m_lstAuthor.end())
	{
		CloseHandle((*ite)->m_hSemaphore);
		closesocket((*ite)->m_sock);
		delete *ite;
		*ite = NULL;
		ite++;
	}

	m_lstAuthor.clear();
	auto iteAudience = m_lstAudience.begin();
	while(iteAudience != m_lstAudience.end())
	{
		CloseHandle((*iteAudience)->m_hEvent);
		closesocket((*iteAudience)->m_sock);
		delete *iteAudience;
		*iteAudience = NULL;
		iteAudience++;
	}

	m_lstAudience.clear();

}

void TCPKernel::DealData(SOCKET sock,char* szbuf)
{
	 //处理数据---协议映射表
	PackType *pType = (PackType *)szbuf;
	int i = 0;
	while(1)
	{
		if(m_ProtocolMapEntries[i].m_nType ==  *pType)
		{
			(this->*m_ProtocolMapEntries[i].m_pfun)(sock,szbuf);
			break;
		}
		else if(m_ProtocolMapEntries[i].m_nType ==0 || m_ProtocolMapEntries[i].m_pfun ==0)
			break;

		i++;
	}

}
