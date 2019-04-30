#ifndef _PACKDEF_H
#define _PACKDEF_H


#define SERVER_IP   "127.0.0.1"
typedef char   PackType;

#define BEGIN_PROTOCOL_MAP static const ProtocolMap m_ProtocolMapEntries[]= \
{

#define END_PROTOCOL_MAP 	{0,0} \
};


#define PM(X,Y)  {X,Y},
//audience消息
#define UM_GETAUTHORLISTMSG   WM_USER + 20

//author消息
#define UM_LOGINMSG            WM_USER + 1
#define UM_ROOMINFOMSG         WM_USER + 2
//边界值
#define _DEF_SIZE				64
#define _DEF_STREAMSIZE			1000
#define _DEF_AUTHORNUM			10
#define _DEF_SQLLEN              300
#define _DEF_AUTHORSIZE          1072*10000
#define _DEF_VIDEOSIZE           1072
#define _DEF_BUFFERNUM           10000
#define _DEF_ROOMNUM             100

#define role_author				0
#define role_audience			1

//
#define _register_telexists      0
#define _register_userexists     1
#define _register_success         2

#define _login_usernoexists      0
#define _login_passworderr       1
#define _login_success           2

#define _setroominfo_fail         0
#define _setroominfo_success      1

 


//协议
#define _DEF_PROTOCOL_BASE		100

#define _DEF_PROTOCOL_REGISTER_RQ			 _DEF_PROTOCOL_BASE +1
#define _DEF_PROTOCOL_REGISTER_RS			 _DEF_PROTOCOL_BASE +2

#define _DEF_PROTOCOL_LOGIN_RQ				 _DEF_PROTOCOL_BASE +3
#define _DEF_PROTOCOL_LOGIN_RS				 _DEF_PROTOCOL_BASE +4

//主播端
#define _DEF_PROTOCOL_GETROOMINFO_RQ		_DEF_PROTOCOL_BASE +5
#define _DEF_PROTOCOL_GETROOMINFO_RS		_DEF_PROTOCOL_BASE +6

#define _DEF_PROTOCOL_SETROOMINFO_RQ		_DEF_PROTOCOL_BASE +7
#define _DEF_PROTOCOL_SETROOMINFO_RS		_DEF_PROTOCOL_BASE +8

#define _DEF_PROTOCOL_STARTTRANSFER_RQ		 _DEF_PROTOCOL_BASE +9
#define _DEF_PROTOCOL_STARTTRANSFER_RS		 _DEF_PROTOCOL_BASE +10

#define _DEF_PROTOCOL_STOPTRANSFER_RQ		_DEF_PROTOCOL_BASE +11
#define _DEF_PROTOCOL_STOPTRANSFER_RS		_DEF_PROTOCOL_BASE +12

#define _DEF_PROTOCOL_VIDEOSTREAMINFO_RQ    _DEF_PROTOCOL_BASE +13
#define _DEF_PROTOCOL_VIDEOSTREAMINFO_RS    _DEF_PROTOCOL_BASE +14

#define _DEF_PROTOCOL_VIDEOSTREAM_RQ		_DEF_PROTOCOL_BASE +15
#define _DEF_PROTOCOL_VIDEOSTREAM_RS		_DEF_PROTOCOL_BASE +16

//观众端
#define _DEF_PROTOCOL_GETAUTHORLIST_RQ		 _DEF_PROTOCOL_BASE +17
#define _DEF_PROTOCOL_GETAUTHORLIST_RS		 _DEF_PROTOCOL_BASE +18

#define _DEF_PROTOCOL_SELECTAUTHOR_RQ		_DEF_PROTOCOL_BASE +19
#define _DEF_PROTOCOL_SELECTAUTHOR_RS		_DEF_PROTOCOL_BASE +20

#define _DEF_PROTOCOL_QUITAUTHOR_RQ			_DEF_PROTOCOL_BASE +21
#define _DEF_PROTOCOL_QUITAUTHOR_RS			_DEF_PROTOCOL_BASE +22

//协议包



struct STRU_REGISTER_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];
	 long long m_tel;
	 char m_szPassword[_DEF_SIZE];
	 char  m_role;
};

typedef  struct STRU_REGISTER_RS
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];
	 char m_szResult;
}STRU_LOGIN_RS;

struct STRU_LOGIN_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];
	 char m_szPassword[_DEF_SIZE];
	 char  m_role;
};

//主播端
struct STRU_GETROOMINFO_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
};

struct STRU_GETROOMINFO_RS
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];
	 char m_szRoomName[_DEF_SIZE];
	 long long m_roomid;
};
struct STRU_SETROOMINFO_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
	 char m_szRoomName[_DEF_SIZE];
};

struct STRU_SETROOMINFO_RS
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
	 char szResult;
};


struct STRU_STARTTRANSFER_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
};

struct STRU_STARTTRANSFER_RS
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
	 char szResult;
};


struct STRU_VIDEOSTREAM_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
	 char m_szStream[_DEF_STREAMSIZE];
	 int  m_nlen;

};

//观众端
struct STRU_GETAUTHORLIST_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
};

struct AuthorInfo
{
	char m_szName[_DEF_SIZE];	
	char m_szRoomName[_DEF_SIZE];
	long long m_roomid;
};

struct STRU_GETAUTHORLIST_RS
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];
	 AuthorInfo m_aryAuthorInfo[_DEF_AUTHORNUM];
	 int        m_nAuthorNum;
};

struct STRU_SELECTAUTHOR_RQ
{
	 PackType m_nType;
	 char m_szName[_DEF_SIZE];	
	 char m_szAuthorName[_DEF_SIZE];
};


#endif