#pragma once


#include <iostream>

#include "wucomm.h"
#include "NetDispatch.h"

namespace udp_wheel
{


	class NetUnit
	{
	public:
		NetUnit()
			:
			m_sock(INVALID_SOCKET)
		{
		}

		virtual ~NetUnit()
		{
			if (IsValid())
			{
				close(m_sock);
				m_sock = INVALID_SOCKET;
			}
		}

		virtual uint32 GetUniqueID() { return 0; }

		void SetHandle(SOCKET sock)
		{
			if (m_sock == INVALID_SOCKET)
			{
				m_sock = sock;
			}
		}

		SOCKET GetHandle() {
			return m_sock;
		}

		bool IsValid()
		{
			return m_sock != INVALID_SOCKET;
		}

		virtual net_dispatch::NetUnitType GetUnitType() = 0;
		virtual int32 Send(const char * data, int32 len, const sockaddr_in& addr) = 0;
		virtual int32 Recv(char * data, int32 len, const sockaddr_in& addr) = 0;

		virtual void Update(uint32 dlt) = 0;

	private:
		SOCKET m_sock;
	};

};
