
#include "wucomm.h"
#include "UDPClient.h"
#include "UDPServer.h"

namespace udp_wheel
{
	void UDPServer::UpdateClient(uint32 dlt)
	{
		for (auto& ptr : m_AllClients)
		{
			ptr.second->Update(dlt);
		}
	}

	bool UDPServer::TryListen()
	{
		SOCKET soc;
		struct sockaddr_in my_addr;

		if (GetHandle() != INVALID_SOCKET)
		{
			perror("Server has been started\n");
			return false;
		}

		/* 开启 socket 监听 */
		if ((soc = NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
		{
			perror("socket create failed ！");
			return false;
		}

		int opt = 1;
		setsockopt(soc, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

		setnonblocking(soc);

		memset(&my_addr,0, sizeof(my_addr));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(m_wPort);
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(soc, (struct sockaddr *) &my_addr, sizeof(sockaddr_in)) == -1)
		{
			perror("bind error");
			return false;
		}
		else
		{
			printf("UDP server start success, info:0.0.0.0:%d\n", m_wPort);
		}

		SetHandle(soc);

		NetUnit* poUnit = this;

		m_kDispatch.AttachHandleToDispatch(soc, poUnit);
		if (!CtlUDPEvent(net_dispatch::ENEO_ADD, net_dispatch::ENE_READ, poUnit))
		{
			perror("AddEvent read fail");
			return false;
		}
		SetRun(true);
		return true;
	}

	bool UDPServer::CtlUDPEvent(net_dispatch::NetEventOp op, uint16 ev, NetUnit * userdata, net_dispatch::NetProtocolType ptType)
	{
		return m_kDispatch.CtlEvent(op, userdata->GetHandle(), net_dispatch::ENE_READ, userdata, ptType);
	}

	UDPClient* UDPServer::OnRecvByAddr(char * data, int32 len, const sockaddr_in& addr)
	{
		UDPClient* poUnit;
		auto id = MakeUniqueID(addr);

		auto it = m_AllClients.find(id);
		if (it != m_AllClients.end())
		{
			poUnit = it->second;
			poUnit->Recv(data, len, addr);
		}
		else
		{
			poUnit = NewNetClient(addr);
			if (poUnit)
			{
				poUnit->Recv(data, len, addr);
			}
		}

		return poUnit;
	}

	void UDPServer::FreeAllClient()
	{
		for (auto& ptr : m_AllClients)
		{
			delete ptr.second;
		}
		m_AllClients.clear();
	}

	UDPClient* UDPServer::NewNetClient(const sockaddr_in& raddr)
	{
		UDPClient* poUnit = NULL;
		socklen_t addr_size = sizeof(sockaddr_in);
		char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
		getnameinfo((struct sockaddr *)&raddr, addr_size, hbuf, sizeof(hbuf),
			sbuf, sizeof(sbuf), NI_NUMERICHOST | NI_NUMERICSERV);


		struct sockaddr_in my_addr;
		SOCKET fd = NewSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		/*设置socket属性，端口可以重用*/
		int opt = 1;
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

		setnonblocking(fd);

		memset(&my_addr, 0, sizeof(my_addr));
		my_addr.sin_family = AF_INET;
		my_addr.sin_port = htons(this->GetPort());
		my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
		if (bind(fd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == SOCKET_ERROR)
		{
			perror("bind fail");
			return NULL;
		}

		if (fd == -1)
			return  NULL;

		if (::connect(fd, (struct sockaddr*)&raddr, sizeof(raddr)) == 0)
		{
			poUnit = new UDPClient(raddr, fd);
			if (AttachNetClient(poUnit))
			{
				printf("new udp client [%s:%s:%u]\n", hbuf, sbuf, poUnit->GetUniqueID());
				poUnit->SetClientType(EUCT_ACCEPT);
				return poUnit;
			}
		}

		close(fd);
		delete poUnit;
		perror("new udp client fail");
		return  NULL;
	}

	bool UDPServer::AttachNetClient(UDPClient* poUnit)
	{
		auto it = m_AllClients.find(poUnit->GetUniqueID());
		if (it == m_AllClients.end())
		{
			if (m_kDispatch.AttachHandleToDispatch(poUnit->GetHandle(), poUnit))
			{
				if (CtlUDPEvent(net_dispatch::ENEO_ADD, net_dispatch::ENE_READ, poUnit))
				{
					m_AllClients.emplace(poUnit->GetUniqueID(), poUnit);
					return true;
				}
			}
		}

		printf("AttachNetClient fail\n");
		return false;
	}

	int32 UDPServer::Send(const char * data, int32 len, const sockaddr_in& addr)
	{
		exit(99);
		return 0;
	}

	void UDPServer::Update(uint32 dlt)
	{
		UpdateClient(dlt);

		int32 num = m_kDispatch.Update(m_EventInfo, MAX_REQ_PER_TIME);

		for (int32 index = 0; index < num; ++index)
		{
			net_dispatch::NetEventInfo& rInfo = m_EventInfo[index];

			switch (rInfo.evt)
			{
			case net_dispatch::ENE_READ:
			{

#ifndef __LINUX__
				net_dispatch::NetEventInfo* poReal = (net_dispatch::NetEventInfo*)rInfo.userdata;
				NetUnit* pUnit = (NetUnit*)poReal->userdata;
				pUnit->Recv(poReal->buf, poReal->len, poReal->addr);
				CtlUDPEvent(net_dispatch::ENEO_MOD, net_dispatch::ENE_READ, pUnit);
				delete poReal;
#else
				static struct sockaddr_in addr;
				NetUnit* pUnit = (NetUnit*)rInfo.userdata;
				pUnit->Recv(NULL, 0, addr);
#endif
			}break;
			case net_dispatch::ENE_WRITE:
			{

			}break;
			case net_dispatch::ENE_CLOSE:
			{

			}break;
			default:
				break;
			}
		}

	}



#ifndef __LINUX__

	UDPServer::~UDPServer()
	{
		WSACleanup();
		FreeAllClient();
		m_kDispatch.UnInit();
	}

	UDPServer::UDPServer(const char* addr, uint16 uPort)
	{
		strncpy(m_ip, addr, sizeof(m_ip));
		m_wPort = uPort;

		WSADATA wsd;
		WORD wVersionRequested = MAKEWORD(2, 2);
		int nResult = WSAStartup(wVersionRequested, &wsd);
		if (nResult == SOCKET_ERROR)
		{
			WSACleanup();
			exit(-11);
		}

		if (LOBYTE(wsd.wVersion) != 2 || HIBYTE(wsd.wVersion) != 2)
		{
			WSACleanup();
			exit(-11);
		}

		m_kDispatch.Init();

		SetRun(false);
	}

	int32 UDPServer::Recv(char * data, int32 len, const sockaddr_in& addr)
	{
		UDPClient* poUnit = OnRecvByAddr(data, len, addr);
		if (poUnit)
		{
			CtlUDPEvent(net_dispatch::ENEO_MOD, net_dispatch::ENE_READ, poUnit);
		}
		return 0;
	}

#else

	UDPServer::~UDPServer()
	{
		FreeAllClient();
		m_kDispatch.UnInit();
	}

	UDPServer::UDPServer(const char* addr, uint16 uPort)
	{
		strncpy(m_ip, addr, sizeof(m_ip));
		m_wPort = uPort;

		m_kDispatch.Init();

		SetRun(false);
	}

	int32 UDPServer::Recv(char * data, int32 len, const sockaddr_in& addr)
	{
		struct sockaddr_in client_addr;
		socklen_t addr_size = sizeof(client_addr);
		char buf[MAX_CLIENT_BUFF_LEN];
		int ret = recvfrom(GetHandle(), buf, MAX_CLIENT_BUFF_LEN, 0, (struct sockaddr *)&client_addr, &addr_size);

		if (ret < 0)
		{
			printf("recvfrom %d", GetErrNo());
			return 0;
		}

		OnRecvByAddr(buf, ret, client_addr);
		//UDPClient* poNewClient = NewNetClient(client_addr);
		//if (poNewClient)
		//{
		//	return poNewClient->Recv(buf, ret, client_addr);
		//}
		return 0;
	}

#endif

};


