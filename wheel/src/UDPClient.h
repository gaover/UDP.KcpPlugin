#pragma once

#include "wustd.h"
#include "NetUnit.h"
#include "KCPPlug.h"

namespace udp_wheel
{
#define MAX_CLIENT_BUFF_LEN	512

	enum UDPClientType
	{
		EUCT_ACCEPT,
		EUCT_CONNECT,
	};

	int OutputCallback(const char *buf, int len, void *kcp, void *user);

	class UDPClient : public NetUnit
	{
	public:
		UDPClient(const sockaddr_in& addinfo, SOCKET soc);
		UDPClient();
		~UDPClient();

		virtual void Update(uint32 dlt);
		static UDPClient* TryConnect(const char* ip, uint16 uport);
		virtual net_dispatch::NetUnitType GetUnitType() { return net_dispatch::ENUT_CLIENT; }

		virtual int32 Send(const char * data, int32 len, const sockaddr_in& addr);
		virtual int32 Recv(char * data, int32 len, const sockaddr_in& addr);

		bool CheckKcpPlug(void* data, int32 len);
		void OnRecvData(const char* data, int32 len);

		void * GetKcpPlugControlBlock() { return m_PlugUserData; }

		void SetSomeInfoByAddr(const sockaddr_in& rAddr);
		int32 RealSend(const char * data, int32 len);
		int32 RealRecv();
		const sockaddr_in& GetAddrInfo() { return m_kAddrInfo; }

		void ReSetBuff() { m_buffOffset = 0; m_buff[m_buffOffset] = 0; }

		void CopyBuff(void * data, uint32 len) 
		{
			if (len > 0 && data)
			{
				memcpy(m_buff, data, len);
				m_buffOffset += len;
			}
		}
		char * GetBuffPtr() { return m_buff; }
		int32 GetBuffLen() { return m_buffOffset; }
		int32 GetBuffLeftLen() { return MAX_CLIENT_BUFF_LEN - m_buffOffset;	}
		virtual uint32 GetUniqueID() { return m_BigID; }
		const uint32 GetKcpConvID();
		void SetKcpConvID(uint32 id);
		UDPClientType GetClientType() { return m_eClientType; }
		void SetClientType(UDPClientType tt) { m_eClientType = tt; }
	private:

		//---------------------------member
		struct sockaddr_in m_kAddrInfo;
		void * m_PlugUserData;
		char  m_buff[MAX_CLIENT_BUFF_LEN];
		int32 m_buffOffset;
		uint32 m_BigID;
		uint32 m_ConvID;
		UDPClientType m_eClientType;
	};



};
