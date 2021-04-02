#pragma once

#include "wustd.h"

namespace udp_wheel
{
#ifndef __LINUX__

	static void itimeofday(long *sec, long *usec)
	{
		static long mode = 0, addsec = 0;
		BOOL retval;
		static int64 freq = 1;
		int64 qpc;
		if (mode == 0) {
			retval = QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
			freq = (freq == 0) ? 1 : freq;
			retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
			addsec = (long)time(NULL);
			addsec = addsec - (long)((qpc / freq) & 0x7fffffff);
			mode = 1;
		}
		retval = QueryPerformanceCounter((LARGE_INTEGER*)&qpc);
		retval = retval * 2;
		if (sec) *sec = (long)(qpc / freq) + addsec;
		if (usec) *usec = (long)((qpc % freq) * 1000000 / freq);
	}

	static inline SOCKET NewSocket(int32 af, int32 type, int32 protocol)
	{
		return WSASocket(af, type, protocol, NULL, 0, WSA_FLAG_OVERLAPPED);
	}

	static inline int32 GetErrNo()
	{
		return WSAGetLastError();
	}

	static inline bool setnonblocking(SOCKET fd)
	{
		u_long arg = 1;
		return (::ioctlsocket(fd, FIONBIO, &arg) == 0);
	}

	static inline bool closedispatch(DISPATCH_HANDLE soc)
	{
		return ::CloseHandle(soc);
	}

	static inline bool close(SOCKET soc)
	{
		return ::closesocket(soc);
	}

#else

	/* get system time */
	static void itimeofday(long *sec, long *usec)
	{
		struct timeval time;
		gettimeofday(&time, NULL);
		if (sec) *sec = time.tv_sec;
		if (usec) *usec = time.tv_usec;
	}

	static inline SOCKET NewSocket(int32 af, int32 type, int32 protocol)
	{
		return socket(af, type, protocol);
	}

	static inline int32 GetErrNo()
	{
		return errno;
	}

	static inline bool setnonblocking(SOCKET fd)
	{
		int arg = 1;
		::ioctl(fd, FIONBIO, &arg);
		return true;
	}

#endif

	/* get clock in millisecond 64 */
	static inline int64 iclock64(void)
	{
		long s, u;
		int64 value;
		itimeofday(&s, &u);
		value = ((int64)s) * 1000 + (u / 1000);
		return value;
	}

	static inline uint32 iclock()
	{
		return (uint32)(iclock64() & 0xfffffffful);
	}

	static inline const char* ErrMessage(int32 eno)
	{
		return strerror(eno);
	}

	static inline uint32 MakeUniqueID(const sockaddr_in& address)
	{
		return uint32(address.sin_addr.s_addr) + (uint32(address.sin_port) << (sizeof(address.sin_port) * 8));
	}

	inline void sleep(uint32 ms)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(ms));
	}


};




