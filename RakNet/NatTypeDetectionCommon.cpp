#include "NatTypeDetectionCommon.h"
#include "SocketLayer.h"
#include "SocketIncludes.h"

using namespace RakNet;

bool RakNet::CanConnect(NATTypeDetectionResult type1, NATTypeDetectionResult type2)
{
	/// If one system is NAT_TYPE_SYMMETRIC, the other must be NAT_TYPE_ADDRESS_RESTRICTED or less
	/// If one system is NAT_TYPE_PORT_RESTRICTED, the other must be NAT_TYPE_PORT_RESTRICTED or less
	bool connectionGraph[5][5] =
	{
		// None,	Full Cone,	Address Restricted,		Port Restricted,	Symmetric
		true, 		true, 		true, 					true, 				true,		// None
		true, 		true, 		true, 					true, 				true,		// Full Cone
		true, 		true, 		true, 					true, 				true,		// Address restricted
		true, 		true, 		true, 					true, 				false,		// Port restricted
		true, 		true, 		true, 					false, 				false,		// Symmetric
	};

	return connectionGraph[(int) type1][(int) type2];
}

const char *RakNet::NATTypeDetectionResultToString(NATTypeDetectionResult type)
{
	switch (type)
	{
	case NAT_TYPE_NONE:
		return "None";
	case NAT_TYPE_FULL_CONE:
		return "Full cone";
	case NAT_TYPE_ADDRESS_RESTRICTED:
		return "Address restricted";
	case NAT_TYPE_PORT_RESTRICTED:
		return "Port restricted";
	case NAT_TYPE_SYMMETRIC:
		return "Symmetric";
	}
	return "Error, unknown enum in NATTypeDetectionResult";
}

// None and relaxed can connect to anything
// Moderate can connect to moderate or less
// Strict can connect to relaxed or less
const char *RakNet::NATTypeDetectionResultToStringFriendly(NATTypeDetectionResult type)
{
	switch (type)
	{
	case NAT_TYPE_NONE:
		return "Open";
	case NAT_TYPE_FULL_CONE:
		return "Relaxed";
	case NAT_TYPE_ADDRESS_RESTRICTED:
		return "Relaxed";
	case NAT_TYPE_PORT_RESTRICTED:
		return "Moderate";
	case NAT_TYPE_SYMMETRIC:
		return "Strict";
	}
	return "Error, unknown enum in NATTypeDetectionResult";
}


SOCKET RakNet::CreateNonblockingBoundSocket(const char *bindAddr)
{
	SOCKET s = SocketLayer::Instance()->CreateBoundSocket( 0, false, bindAddr, true );
	#ifdef _WIN32
		unsigned long nonblocking = 1;
		ioctlsocket( s, FIONBIO, &nonblocking );
	#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3)
                                                                                                     
	#else
		fcntl( s, F_SETFL, O_NONBLOCK );
	#endif
	return s;
}

int RakNet::NatTypeRecvFrom(char *data, SOCKET socket, SystemAddress &sender)
{
	sockaddr_in sa;
	socklen_t len2;
	const int flag=0;
	len2 = sizeof( sa );
	sa.sin_family = AF_INET;
	sa.sin_port=0;
	int len = recvfrom( socket, data, MAXIMUM_MTU_SIZE, flag, ( sockaddr* ) & sa, ( socklen_t* ) & len2 );
	if (len>0)
	{
		sender.binaryAddress = sa.sin_addr.s_addr;
		sender.port = ntohs( sa.sin_port );
	}
	return len;
}
