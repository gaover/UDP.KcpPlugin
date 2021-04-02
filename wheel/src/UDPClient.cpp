
#include "KCPPlug.h"
#include "UDPClient.h"

namespace udp_wheel
{

	UDPClient::UDPClient(const sockaddr_in& addinfo, SOCKET soc)
		: m_PlugUserData(NULL)
		, m_buffOffset(0)
		, m_ConvID(0)
	{
		SetHandle(soc);
		SetSomeInfoByAddr(addinfo);
	}

	UDPClient::UDPClient()
		: m_PlugUserData(NULL)
		, m_buffOffset(0)
		, m_ConvID(0)
	{

	}

	UDPClient::~UDPClient()
	{
		kcp_plug::KcpRelease(GetKcpPlugControlBlock());
	}

	void UDPClient::SetSomeInfoByAddr(const sockaddr_in& rAddr) 
	{
			m_kAddrInfo = rAddr;
			m_BigID = MakeUniqueID(rAddr);
	}

	const uint32 UDPClient::GetKcpConvID()
	{
		if (kcp_plug::UseKcpPlug())
		{
			return m_ConvID;
		}

		return 1;
	}


	void UDPClient::SetKcpConvID(uint32 id)
	{
		if (!m_ConvID)
		{
			m_ConvID = id;
			if (!m_PlugUserData)
			{
				m_PlugUserData = kcp_plug::NewKcpPlugControlBlock(m_ConvID, this, (void*)&OutputCallback);
			}
		}
	}

	void UDPClient::Update(uint32 dlt)
	{
		return kcp_plug::KcpUpdate(GetKcpPlugControlBlock(), dlt);
	}

	int32 UDPClient::Send(const char * data, int32 len, const sockaddr_in& addr)
	{
		if (!this->GetKcpConvID())
		{
			return this->RealSend(data, len);
		}

		return kcp_plug::KcpSend(GetKcpPlugControlBlock(), this->GetHandle(), data, len);
	}

	bool UDPClient::CheckKcpPlug(void* data, int32 len)
	{
		if (!kcp_plug::UseKcpPlug()) return false;

		UDPClientType tt = this->GetClientType();
		switch (tt)
		{
		case EUCT_ACCEPT:
		{
			if (!this->GetKcpConvID())
			{
				uint32 id = this->GetUniqueID();
				// 交换会话id
				this->SetKcpConvID(kcp_plug::GetKcpConvID(&id, sizeof(uint32)));
				this->RealSend((const char*)&id, sizeof(uint32));
				return true;
			}
		}break;
		case EUCT_CONNECT:
		{
			if (!this->GetKcpConvID())
			{
				// 交换会话id
#ifdef __LINUX__
				len = this->RealRecv();
				data = (void*)GetBuffPtr();
#endif
				this->SetKcpConvID(kcp_plug::GetKcpConvID(data, len));
				return true;
			}
		}break;
		default:
			return false;
			break;
		}

		return false;
	}

	int32 UDPClient::Recv(char * data, int32 len, const sockaddr_in& addr)
	{
		if (!this->CheckKcpPlug(data, len))
		{
			int32 n = kcp_plug::KcpRecv(GetKcpPlugControlBlock(), this->GetHandle(), data, len, GetBuffPtr(), GetBuffLeftLen());
			if (n > 0)
			{
				this->OnRecvData(GetBuffPtr(), n);
			}
		}

		return 0;
	}

	void UDPClient::OnRecvData(const char* data, int32 len)
	{
		char * pstr = (char*)data;
		pstr[len] = 0;
		printf("[%u] recv data %s\n", GetUniqueID(), pstr);
		char buf[100];
		len = snprintf(buf, sizeof(buf), "%u", GetUniqueID());
		Send(buf, len, GetAddrInfo());
	}

	int32 UDPClient::RealSend(const char * data, int32 len)
	{
		return send(GetHandle(), data, len, 0);
	}

	int32 UDPClient::RealRecv()
	{
		return recv(GetHandle(), GetBuffPtr(), GetBuffLeftLen(), 0);
	}

	UDPClient* UDPClient::TryConnect(const char* ip, uint16 uport)
	{
		struct sockaddr_in my_addr;
		struct sockaddr_in remote_addr;
		UDPClient* poUnit = NULL;
		SOCKET fd = NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		/*设置socket属性，端口可以重用*/
		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

		setnonblocking(fd);

		memset(&my_addr, 0, sizeof(my_addr));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(0);
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == SOCKET_ERROR)
		{
			perror("bind fail");
			return NULL;
		}
		else
		{
			//printf("IP and port bind success \n");
		}

		if (fd == -1)
			return  NULL;

		socklen_t addr_len = sizeof(struct sockaddr);
		if (0 != getsockname(fd, (struct sockaddr*)&my_addr, &addr_len))
		{
			printf("get sock name fail\n");
			close(fd);
			return NULL;
		}

		memset(&remote_addr, 0, sizeof(remote_addr));
		//设置服务器ip、port
		remote_addr.sin_family = AF_INET;
		remote_addr.sin_addr.s_addr = inet_addr((char*)ip);
		remote_addr.sin_port = htons(uport);

		if (::connect(fd, (struct sockaddr*)&remote_addr, sizeof(remote_addr)) == 0)
		{
			poUnit = new UDPClient();
			poUnit->SetHandle(fd);
			poUnit->SetSomeInfoByAddr(my_addr);
			poUnit->SetClientType(EUCT_CONNECT);
			printf("connect %s %u success.local info [0.0.0.0:%u:%u]\n", 
				ip, uport, ntohs(my_addr.sin_port), poUnit->GetUniqueID());
			return poUnit;
		}
		else
		{
			printf("connect %s %u fail [%d][%s]\n", ip, uport, GetErrNo(), strerror(GetErrNo()));
		}

		close(fd);
		delete poUnit;
		return NULL;
	}

	int OutputCallback(const char *buf, int len, void *kcp, void *user)
	{
		if (user)
		{
			UDPClient* poClient = (UDPClient*)user;
			//发送信息
			return send(poClient->GetHandle(), buf, len, 0);
		}
		return 0;
	}


};


