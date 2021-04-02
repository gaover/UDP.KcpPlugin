#pragma once

#include "wustd.h"


namespace net_dispatch
{
	enum NetEvent
	{
		ENE_READ = 0x01,
		ENE_WRITE = 0x02,
		ENE_CLOSE = 0x04,
	};

#if defined(__LINUX__)
	enum NetEventOp
	{
		ENEO_ADD = EPOLL_CTL_ADD,
		ENEO_MOD = EPOLL_CTL_MOD,
		ENEO_DEL = EPOLL_CTL_DEL,
	};

	struct NetEventInfo
	{
		NetEventInfo()
		{
			memset(this, 0, sizeof(NetEventInfo));
		}
		uint16 evt;
		void* userdata;
};


#else
	enum NetEventOp
	{
		ENEO_ADD,
		ENEO_MOD,
		ENEO_DEL,
	};



	struct NetEventInfo : public OVERLAPPED
	{
		NetEventInfo()
		{
			memset(this, 0, sizeof(NetEventInfo));
			addr_len = sizeof(addr);
		}
		uint16 evt;
		void* userdata;

		int32 len;
		char buf[512];
		struct sockaddr_in addr;
		int32 addr_len;
	};

#endif
	enum NetUnitType
	{
		ENUT_LISTEN,
		ENUT_CLIENT,
	};

	enum NetProtocolType
	{
		ENPT_UDP,
		ENPT_TCP,
	};


#define MAX_REQ_PER_TIME 1024

	class NetDispatch
	{
	public:
		NetDispatch();
		~NetDispatch();

		bool Init();
		bool UnInit();
		
		int32 Update(NetEventInfo* pEvtInfo, int32 nMaxNum);

		bool CtlEvent(NetEventOp op, SOCKET sock, uint16 ev, void * userdata, NetProtocolType ptType);

		bool AttachHandleToDispatch(SOCKET sock, void * pdata);

	private:
		EVENT_HANDLE m_kEvents[MAX_REQ_PER_TIME];
		DISPATCH_HANDLE m_kDispatch;
	};



};
