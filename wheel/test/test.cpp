#include "UDPServer.h"
#include <iostream>
#include <string>

using namespace udp_wheel;

int main()
{
	UDPServer stServer("0.0.0.0", 60000);

	uint32 dlt = 0;

	if (stServer.TryListen())
	{
		//UDPClient* poNew = UDPClient::TryConnect("127.0.0.1", 60000);
		//if (!poNew)
		//{
		//	printf("UDPClient::TryConnect(\"127.0.0.1\", 60000) fail\n");
		//	return 0;
		//}

		//stServer.AttachNetClient(poNew);
		//dlt = iclock();

		//std::string aaaa = std::to_string(poNew->GetUniqueID());

		//int32 ret = poNew->Send(aaaa.c_str(), aaaa.length(), poNew->GetAddrInfo());
		//if (!ret)
		//{
		//	std::cout << "Send err" << std::endl;
		//	return -999;
		//}
		bool bOnce = false;
		while (stServer.IsRun())
		{
			//if (!bOnce && poNew->GetKcpConvID())
			//{
			//	poNew->Send(aaaa.c_str(), aaaa.length(), poNew->GetAddrInfo());
			//	bOnce = true;
			//}

			dlt = iclock();
			stServer.Update(dlt);

			udp_wheel::sleep(100);
		}
	}


	return 0;

}