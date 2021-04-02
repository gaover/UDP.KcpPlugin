#include "wucomm.h"
#include "NetDispatch.h"

namespace net_dispatch
{

#if defined(__LINUX__)

	NetDispatch::NetDispatch()
	{


	}

	NetDispatch::~NetDispatch()
	{
		UnInit();
	}

	bool NetDispatch::Init()
	{
		DISPATCH_HANDLE _dispatch = epoll_create1(EPOLL_CLOEXEC);
		if (_dispatch == INVALID_DISPATCH)
		{
			perror("epoll_create1 fail");
			return false;
		}

		m_kDispatch = _dispatch;
		return true;
	}

	bool NetDispatch::UnInit()
	{
		if (m_kDispatch != INVALID_DISPATCH)
		{
			close(m_kDispatch);
			m_kDispatch = INVALID_DISPATCH;
		}
		return true;
	}

	bool NetDispatch::AttachHandleToDispatch(SOCKET sock, void * userdata)
	{
		int ret = 0;
		epoll_event ev;
		ev.events = 0;
		ev.data.ptr = userdata;

		ret = ::epoll_ctl(m_kDispatch, ENEO_ADD, sock, &ev);
		if (ret) {
			printf("et [%d]ID add error %s\n", sock, strerror(errno));
		}

		return !ret;
	}

	bool NetDispatch::CtlEvent(NetEventOp op, SOCKET sock, uint16 ee, void * userdata, NetProtocolType ptType)
	{
		int ret = 0;
		epoll_event ev;
		ev.events = ee;
		ev.data.ptr = userdata;

		ret = ::epoll_ctl(m_kDispatch, ENEO_MOD, sock, &ev);
		if (ret) {
			printf("et [%d]ID ctl error %s\n", sock, strerror(errno));
		}

		return !ret;
	}

	int32 NetDispatch::Update(NetEventInfo* pEvtInfo, int32 nMaxNum)
	{
		int32 uMaxSize = nMaxNum > MAX_REQ_PER_TIME ? MAX_REQ_PER_TIME : nMaxNum;

		int sz = 0;
		sz = ::epoll_wait(m_kDispatch, m_kEvents, uMaxSize, 5);
		if (sz < 0)
		{
			return 0;
		}
		// 这里需要对 NetEventInfo 里的信息加工
		for (int i = 0; i != sz; ++i)
		{
			EVENT_HANDLE& rkEvent = m_kEvents[i];
			NetEventInfo* pkDestEvent = pEvtInfo + i;

			pkDestEvent->userdata = (void*)rkEvent.data.ptr;
			pkDestEvent->evt = 0;
			if (rkEvent.events & EPOLLERR || rkEvent.events & EPOLLHUP)
			{
				pkDestEvent->evt |= ENE_CLOSE;
			}
			if (rkEvent.events & EPOLLIN)
			{
				pkDestEvent->evt |= ENE_READ;
			}
			if (rkEvent.events & EPOLLOUT)
			{
				pkDestEvent->evt |= ENE_WRITE;
			}
		}
		return sz;
	}

#else

	NetDispatch::NetDispatch()
	{


	}

	NetDispatch::~NetDispatch()
	{
		UnInit();
	}

	bool NetDispatch::Init()
	{
		DISPATCH_HANDLE _dispatch = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
		if (_dispatch == INVALID_DISPATCH)
		{
			perror("CreateIoCompletionPort fail");
			return false;
		}

		m_kDispatch = _dispatch;
		return true;
	}

	bool NetDispatch::UnInit()
	{
		if (m_kDispatch != INVALID_DISPATCH)
		{
			udp_wheel::closedispatch(m_kDispatch);
			m_kDispatch = INVALID_DISPATCH;
		}
		return true;
	}

	bool NetDispatch::AttachHandleToDispatch(SOCKET sock, void * pdata)
	{
		return !!CreateIoCompletionPort((HANDLE)sock, m_kDispatch, (ULONG_PTR)pdata, 0);
	}

	bool NetDispatch::CtlEvent(NetEventOp op, SOCKET sock, uint16 ev, void * userdata, NetProtocolType ptType)
	{
		switch (op)
		{
		case net_dispatch::ENEO_ADD:
		case net_dispatch::ENEO_MOD:
		{
			if (ev & ENE_READ)
			{
				NetEventInfo* pEvtInfo = new NetEventInfo();
				pEvtInfo->evt = ENE_READ;
				pEvtInfo->userdata = userdata;
				WSABUF buf;

				buf.buf = pEvtInfo->buf;
				buf.len = sizeof(pEvtInfo->buf);

				DWORD flags = 0;
				DWORD dwSz = 0;
				DWORD dwLen = 0;

				switch (ptType)
				{
				case net_dispatch::ENPT_UDP:
				{
					//_In_ SOCKET s,
					//	_In_reads_(dwBufferCount) __out_data_source(NETWORK) LPWSABUF lpBuffers,
					//	_In_ DWORD dwBufferCount,
					//	_Out_opt_ LPDWORD lpNumberOfBytesRecvd,
					//	_Inout_ LPDWORD lpFlags,
					//	_Out_writes_bytes_to_opt_(*lpFromlen, *lpFromlen) struct sockaddr_in FAR * lpFrom,
					//	_Inout_opt_ LPINT lpFromlen,
					//	_Inout_opt_ LPWSAOVERLAPPED lpOverlapped,
					//	_In_opt_ LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine

					int32 iRet = WSARecvFrom(sock,
						&buf,
						1, &dwLen,
						&flags,
						(sockaddr*)&pEvtInfo->addr, &pEvtInfo->addr_len,
						pEvtInfo,
						NULL
					);
					int32 eno = WSAGetLastError();
					if (iRet == SOCKET_ERROR && eno != ERROR_IO_PENDING)
					{
						return false;
					}
				}break;
				case net_dispatch::ENPT_TCP:
				{
					if (WSARecv(sock, &buf, 1, &dwSz, &flags, (LPWSAOVERLAPPED)pEvtInfo, 0) == SOCKET_ERROR)
					{
						int iSendError = WSAGetLastError();
						if (iSendError != ERROR_IO_PENDING)
						{
							// 返回false则删除该elem,由外层删除
							return false;
						}
					}
				}break;
				default:
					break;
				}
			}

			if (ev & ENE_WRITE)
			{
				NetEventInfo* pEvtInfo = new NetEventInfo();
				pEvtInfo->evt = ENE_WRITE;
				pEvtInfo->userdata = userdata;

				WSABUF buf;

				buf.buf = pEvtInfo->buf;
				buf.len = sizeof(pEvtInfo->buf);

				DWORD flags = 0;
				DWORD dwSz = 0;

				pEvtInfo->addr_len = sizeof(sockaddr_in);

				switch (ptType)
				{
				case net_dispatch::ENPT_UDP:
				{
					// UDP直接发送
					//int32 iRet = WSASendTo(sock,
					//	&buf,
					//	1, &dwSz,
					//	flags,
					//	(const sockaddr_in*)&pEvtInfo->addr, pEvtInfo->addr_len,
					//	pEvtInfo,
					//	NULL
					//);

					//if (iRet == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING)
					//{
					//	return false;
					//}
				}break;
				case net_dispatch::ENPT_TCP:
				{
					if (WSASend(sock, &buf, 1, &dwSz, 0, pEvtInfo, NULL) == SOCKET_ERROR)
					{
						int iSendError = WSAGetLastError();
						if (iSendError != ERROR_IO_PENDING)
						{
							// 返回false则删除该elem,由外层删除
							return false;
						}
					}
				}break;
				default:
					break;
				}
			}
		}break;
		case net_dispatch::ENEO_DEL:
			break;
		default:
			break;
		}

		return true;
	}

	int32 NetDispatch::Update(NetEventInfo* pEvtInfo, int32 nMaxNum)
	{
		ULONG ulSz = 0;
		ULONG uMaxSize = nMaxNum > MAX_REQ_PER_TIME ? MAX_REQ_PER_TIME : nMaxNum;

		if (!::GetQueuedCompletionStatusEx(m_kDispatch, m_kEvents, uMaxSize, &ulSz, 5, true))
		{
			int code = GetLastError();
			if (code != 258)
			{
				DebugBreak();
			}
			ulSz = 0;
		}
		int32 sz = ulSz;
		// 这里需要对 NetEventInfo 里的信息加工
		for (int32 i = 0; i != sz; ++i)
		{
			NetEventInfo* pkDestInfo = pEvtInfo + i;
			EVENT_HANDLE* pkEntry = m_kEvents + i;
			NetEventInfo* pkTempEvent2 = (NetEventInfo*)pkEntry->lpOverlapped;

			if (pkEntry->dwNumberOfBytesTransferred == 0)
			{
				pkDestInfo->evt = ENE_CLOSE;
				pkDestInfo->userdata = pkTempEvent2;
				continue;
			}

			switch (pkTempEvent2->evt)
			{
			case ENE_READ:
			case ENE_CLOSE:
			{
				pkDestInfo->evt = ENE_READ;
				pkDestInfo->userdata = pkTempEvent2;
				pkTempEvent2->len = pkEntry->dwNumberOfBytesTransferred;
			}break;
			}
		}
		return sz;
	}


#endif

}