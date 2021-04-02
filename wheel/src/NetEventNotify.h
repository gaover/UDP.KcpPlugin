#pragma once

class NetEventNotify
{
public:
	NetEventNotify() {}
	virtual ~NetEventNotify() {}

	virtual void OnConnectNotify(void * userdata) {}
	virtual void OnAcceptNotify(void * userdata) {}
	virtual void OnCloseNotify(void * userdata) {}
	virtual void OnRecv(void * userdata) {}
};