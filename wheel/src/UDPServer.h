#pragma once

#include <unordered_map>

#include "wustd.h"
#include "NetUnit.h"
#include "UDPClient.h"
#include "NetDispatch.h"

namespace udp_wheel
{
	class UDPServer : public NetUnit
	{
	public:

		UDPServer(const char* addr, uint16 uPort);

		~UDPServer();

		virtual net_dispatch::NetUnitType GetUnitType() { return net_dispatch::ENUT_LISTEN; }
		virtual int32 Send(const char * data, int32 len, const sockaddr_in& addr);
		virtual int32 Recv(char * data, int32 len, const sockaddr_in& addr);

		bool TryListen();
		void Update(uint32 dlt);
		UDPClient* NewNetClient(const struct sockaddr_in& raddr);
		bool AttachNetClient(UDPClient* poNewClient);
		bool IsRun() { return m_bRun; }
		void StopRun() { m_bRun = false; }
		virtual uint32 GetUniqueID() { return 0xffffffff; }

	private:
		uint16 GetPort() {
			return m_wPort;
		}
		uint32 GetIntIp() {
			return 0;
		}

		void UpdateClient(uint32 dlt);
		void SetRun(bool bflag) { m_bRun = bflag; }
		bool CtlUDPEvent(net_dispatch::NetEventOp op, uint16 ev, NetUnit * userdata, net_dispatch::NetProtocolType ptType = net_dispatch::ENPT_UDP);
		UDPClient* OnRecvByAddr(char * data, int32 len, const sockaddr_in& addr);

		void FreeAllClient();

	private:
		net_dispatch::NetEventInfo m_EventInfo[MAX_REQ_PER_TIME];
		net_dispatch::NetDispatch m_kDispatch;
		char m_ip[32];
		uint32 m_uIp;
		uint16 m_wPort;
		bool m_bRun;
		std::unordered_map<uint32, UDPClient* > m_AllClients;
	};


};











