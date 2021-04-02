#pragma once

#include "wustd.h"

namespace kcp_plug
{
#ifdef USE_KCP_CONTROL_TRANSFORM
	#include "ikcp.h"
#endif

#define MAX_UDP_MTU	576

	typedef int(*kcpOutputCallback)(const char *buf, int len, struct IKCPCB *kcp, void *user);

	inline bool UseKcpPlug()
	{
#ifdef USE_KCP_CONTROL_TRANSFORM
		return true;
#else
		return false;
#endif
	}

	inline uint32 GetKcpConvID(const void* data, int32 len)
	{
#ifdef USE_KCP_CONTROL_TRANSFORM
		uint32 id = 0;
		memcpy(&id, data, len);
		return id;
#else
		return 1;
#endif

	}

	inline void KcpRelease(void* block)
	{
#ifdef USE_KCP_CONTROL_TRANSFORM
		if (!block) return;
		ikcp_release((ikcpcb *)block);
#else

#endif
	}

	inline void * NewKcpPlugControlBlock(uint32 uconv, void * userdata, void* call)
	{
#ifdef USE_KCP_CONTROL_TRANSFORM

		ikcpcb *kcp = ikcp_create(uconv, userdata);
		kcp->output = (kcpOutputCallback)call;//设置kcp对象的回调函数
		ikcp_nodelay(kcp, 0, 10, 0, 0);
		ikcp_wndsize(kcp, 128, 128);
		ikcp_setmtu(kcp, MAX_UDP_MTU);
		return kcp;
#else
		return nullptr;
#endif
	}

	inline void KcpUpdate(void *block, uint32 dlt)
	{
#ifdef USE_KCP_CONTROL_TRANSFORM
		if (!block) return;
		ikcp_update((ikcpcb *)block, dlt);
#else

#endif
	}

	inline int32 KcpSend(void * block, SOCKET soc, const char * data, int32 len)
	{
#ifdef USE_KCP_CONTROL_TRANSFORM
		if (!block) return 0;
		return ikcp_send((ikcpcb *)block, (const char*)data, len);
#else
		return send(soc, (const char*)data, len, 0);
#endif
	}

	inline int32 KcpRecv(void * block, SOCKET soc, void * input, int32 currLen, void* outbuff, int32 maxlen)
	{
		char* pBuf = (char*)outbuff;
		int32 n;
#ifdef USE_KCP_CONTROL_TRANSFORM
		if (!block) return 0;
		char TmpBuff[MAX_UDP_MTU];
		ikcpcb * pcb = (ikcpcb *)block;
		if (currLen > 0 && input)
		{
			ikcp_input(pcb, (const char*)input, currLen);
		}

		while (1)
		{
			if ((n = recv(soc, TmpBuff, MAX_UDP_MTU, 0)) > 0)
			{
				ikcp_input(pcb, TmpBuff, n);
			}
			else
			{
				break;
			}
		}

		currLen = 0;
		while (maxlen > 0)
		{
			n = ikcp_recv(pcb, pBuf + currLen, maxlen);
			if (n < 0)
			{
				break;
			}
			currLen += n;
			maxlen -= n;
		}

		return currLen;

#else
		if (currLen > 0 && input)
		{
			memcpy(pBuf, (const char*)input, currLen);
		}

		while (maxlen > 0)
		{
			if ((n = recv(soc, pBuf + currLen, maxlen, 0)) > 0)
			{
				currLen += n;
				maxlen -= n;
			}
			else
			{
				break;
			}
		}

		return currLen;
#endif
	}

};

