/// \file
/// \brief SocketLayer class implementation
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "SocketLayer.h"
#include "RakAssert.h"
#include "RakNetTypes.h"
#include "RakPeer.h"
#include "GetTime.h"
#include "LinuxStrings.h"
#include "SocketDefines.h"

#ifndef _WIN32
#include <netdb.h>
#endif

using namespace RakNet;

/*
#if defined(__native_client__)
using namespace pp;
#endif
*/

#if USE_SLIDING_WINDOW_CONGESTION_CONTROL!=1
#include "CCRakNetUDT.h"
#else
#include "CCRakNetSlidingWindow.h"
#endif

//SocketLayerOverride *SocketLayer::slo=0;

#ifdef _WIN32
#else
#include <string.h> // memcpy
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <errno.h>  // error numbers
#include <stdio.h> // RAKNET_DEBUG_PRINTF
#if !defined(ANDROID)
#include <ifaddrs.h>
#endif
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#endif













#if   defined(_WIN32)
#include "WSAStartupSingleton.h"
#include "WindowsIncludes.h"

#else
#include <unistd.h>
#endif

#include "RakSleep.h"
#include <stdio.h>
#include "Itoa.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

namespace RakNet
{
	extern void ProcessNetworkPacket( const SystemAddress systemAddress, const char *data, const int length, RakPeer *rakPeer, RakNet::TimeUS timeRead );
	//extern void ProcessNetworkPacket( const SystemAddress systemAddress, const char *data, const int length, RakPeer *rakPeer, RakNetSocket* rakNetSocket, RakNet::TimeUS timeRead );
}

#ifdef _DEBUG
#include <stdio.h>
#endif


/*
#ifdef __native_client__

namespace RakNet
{

// Native Client only allows one SendTo call to be in-flight at once, so if a
// send is still in progress and we haven't gotten a callback yet, we queue up
// up to one packet in the socket implementation to be sent immediately as soon
// as the next callback comes in.
struct SocketImpl
{
	SocketImpl();
	~SocketImpl();

	// Chrome socket resource
	PP_Resource s;

	// Flag indicating if a SendTo is currently in progress
	bool sendInProgress;

	// Data for next queued packet to send, if nextSendSize > 0
	char nextSendBuffer[MAXIMUM_MTU_SIZE];

	// Size of next queued packet to send, or 0 if no queued packet
	int nextSendSize;

	// Destination address of queued packet
	PP_NetAddress_Private nextSendAddr;
};

SocketImpl::SocketImpl()
{
	s = 0;
	sendInProgress = false;
	nextSendSize = 0;
}

SocketImpl::~SocketImpl()
{
	if(s != 0)
		((PPB_UDPSocket_Private_0_4*) pp::Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->Close(s);
}

void CloseSocket(RakNetSocket *s)
{
	RakNet::OP_DELETE(s, _FILE_AND_LINE_);
	}

}  // namespace RakNet

#endif  // __native_client__

*/


// http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#ip4to6
// http://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#getaddrinfo

#if RAKNET_SUPPORT_IPV6==1
void PrepareAddrInfoHints(addrinfo *hints)
{
	memset(hints, 0, sizeof (addrinfo)); // make sure the struct is empty
	hints->ai_socktype = SOCK_DGRAM; // UDP sockets
	hints->ai_flags = AI_PASSIVE;     // fill in my IP for me
}
#endif

/*
// Frogwares: Define this
// #define DEBUG_SENDTO_SPIKES
#if !defined(WINDOWS_STORE_RT)
bool SocketLayer::IsPortInUse_Old(unsigned short port, const char *hostAddress)
{
#ifdef __native_client__
	return false;
#else
	__UDPSOCKET__ listenSocket;
	sockaddr_in listenerSocketAddress;
	memset(&listenerSocketAddress,0,sizeof(sockaddr_in));
	// Listen on our designated Port#
	listenerSocketAddress.sin_port = htons( port );

#if defined(SN_TARGET_PSP2)
	listenSocket = socket__( AF_INET, SCE_NET_SOCK_DGRAM_P2P, 0 );
#else
	listenSocket = socket__( AF_INET, SOCK_DGRAM, 0 );
#endif

#if !defined(WINDOWS_STORE_RT)
	if ( listenSocket == 0 )
		return true;
#endif

	// bind our name to the socket
	// Fill in the rest of the address structure
	listenerSocketAddress.sin_family = AF_INET;
	
	if ( hostAddress && hostAddress[0] )
	{
#if defined(SN_TARGET_PSP2)
		inet_addr__( hostAddress, &listenerSocketAddress.sin_addr );
#else
		listenerSocketAddress.sin_addr.s_addr = inet_addr__( hostAddress );
#endif
	}
	else
		listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
	// Binding specific addresses doesn't work with the PS3
	// The functions return success but broadcast messages, possibly more, never arrive
	listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
#endif

	int ret = bind__( listenSocket, ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );
	closesocket__(listenSocket);

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
	return ret == SYS_NET_EADDRINUSE;
#else
	// 	#if defined(_DEBUG)
	// 	if (ret == -1)
	// 		perror("Failed to bind to address:");
	// 	#endif
	return ret <= -1;
#endif
#endif
}
*/
/*
bool SocketLayer::IsSocketFamilySupported(const char *hostAddress, unsigned short socketFamily)
{
	(void) hostAddress;
	(void) socketFamily;

#if RAKNET_SUPPORT_IPV6!=1
	return socketFamily==AF_INET;
#else
	struct addrinfo hints;
#if RAKNET_SUPPORT_IPV6==1
	PrepareAddrInfoHints(&hints);
#endif
	hints.ai_family = socketFamily;
	struct addrinfo *servinfo=0;
	int error;
	// On Ubuntu, "" returns "No address associated with hostname" while 0 works.
	if (hostAddress && 
		(_stricmp(hostAddress,"UNASSIGNED_SYSTEM_ADDRESS")==0 || hostAddress[0]==0))
	{
		getaddrinfo(0, "0", &hints, &servinfo);
	}
	else
	{
		getaddrinfo(hostAddress, "0", &hints, &servinfo);
	}


	(void) error;
	if (servinfo)
	{
		freeaddrinfo(servinfo);
		return true;
	}
	else
	{
#if (defined(__GNUC__) || defined(__GCCXML__)) && !defined(_WIN32)
		printf("IsSocketFamilySupported failed. hostAddress=%s. %s\n", hostAddress, gai_strerror(error));
#endif
	}
	return false;
#endif
}
*/
/*
bool SocketLayer::IsPortInUse(unsigned short port, const char *hostAddress, unsigned short socketFamily)
{
#if RAKNET_SUPPORT_IPV6!=1
	(void) socketFamily;
	return IsPortInUse_Old(port, hostAddress);
#else
	__UDPSOCKET__ listenSocket;
	struct addrinfo hints;
	struct addrinfo *servinfo=0, *aip;  // will point to the results
	PrepareAddrInfoHints(&hints);
	hints.ai_family = socketFamily;
	char portStr[32];
	Itoa(port,portStr,10);

	// On Ubuntu, "" returns "No address associated with hostname" while 0 works.
	if (hostAddress && 
		(_stricmp(hostAddress,"UNASSIGNED_SYSTEM_ADDRESS")==0 || hostAddress[0]==0))
	{
		getaddrinfo(0, portStr, &hints, &servinfo);
	}
	else
	{
		getaddrinfo(hostAddress, portStr, &hints, &servinfo);
	}

	// Try all returned addresses until one works
	for (aip = servinfo; aip != NULL; aip = aip->ai_next)
	{
		// Open socket. The address type depends on what
		// getaddrinfo() gave us.
		listenSocket = socket__(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		#if !defined(WINDOWS_STORE_RT)
		if (listenSocket != 0)
		#endif
		{
			int ret = bind__( listenSocket, aip->ai_addr, (int) aip->ai_addrlen );
			closesocket__(listenSocket);
			if (ret>=0)
			{
				freeaddrinfo(servinfo); // free the linked-list
				return false;
			}
		}

		// If the user didn't specify which host address, then only apply the first
		if (hostAddress==0 || hostAddress[0]==0)
			break;
	}

	freeaddrinfo(servinfo); // free the linked-list
	return true;
#endif // #if RAKNET_SUPPORT_IPV6!=1
}

#endif // #if defined(WINDOWS_STORE_RT)
*/
/*
void SocketLayer::SetDoNotFragment( RakNetSocket* listenSocket, int opt )
{
#if defined( IP_DONTFRAGMENT )

#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(_DEBUG) && !defined(X360)
	// If this assert hit you improperly linked against WSock32.h
	RakAssert(IP_DONTFRAGMENT==14);
#endif

	//if ( setsockopt__( listenSocket, IPPROTO, IP_DONTFRAGMENT, ( char * ) & opt, sizeof ( opt ) ) == -1 )

	if ( listenSocket->SetSockOpt( listenSocket->GetBoundAddress().GetIPPROTO(), IP_DONTFRAGMENT, ( char * ) & opt, sizeof ( opt ) ) == -1 )
	{
#if defined(_WIN32) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8) && !defined(_XBOX_720_COMPILE_AS_WINDOWS)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// I see this hit on XP with IPV6 for some reason
		RAKNET_DEBUG_PRINTF( "Warning: setsockopt__(IP_DONTFRAGMENT) failed:Error code - %d\n%s", dwIOError, messageBuffer );
		LocalFree( messageBuffer );
#endif
	}
#endif
}

void SocketLayer::SetNonBlocking( RakNetSocket* listenSocket)
{
#ifdef __native_client__
#elif defined(_WIN32)
	unsigned long nonBlocking = 1;
	listenSocket->IOCTLSocket( FIONBIO, &nonBlocking );
#elif defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	int sock_opt=1;
	listenSocket->SetSockOpt( SOL_SOCKET, SO_NBIO, ( char * ) & sock_opt, sizeof ( sock_opt ) );
#else
	int flags = listenSocket->Fcntl( F_GETFL, 0);
	listenSocket->Fcntl( F_SETFL, flags | O_NONBLOCK);
#endif
}
*/
void SocketLayer::SetSocketOptions( __UDPSOCKET__ listenSocket, bool blockingSocket, bool setBroadcast)
{
#ifdef __native_client__
	(void) listenSocket;
#else
	int sock_opt = 1;

	// This doubles the max throughput rate
	sock_opt=1024*256;
	setsockopt__(listenSocket, SOL_SOCKET, SO_RCVBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );

	// Immediate hard close. Don't linger the socket, or recreating the socket quickly on Vista fails.
	// Fail with voice and xbox

	sock_opt=0;
	setsockopt__(listenSocket, SOL_SOCKET, SO_LINGER, ( char * ) & sock_opt, sizeof ( sock_opt ) );



	// This doesn't make much difference: 10% maybe
	// Not supported on console 2
	sock_opt=1024*16;
	setsockopt__(listenSocket, SOL_SOCKET, SO_SNDBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );


	if (blockingSocket==false)
	{
#ifdef _WIN32
		unsigned long nonblocking = 1;
		ioctlsocket__(listenSocket, FIONBIO, &nonblocking );



#else
		fcntl( listenSocket, F_SETFL, O_NONBLOCK );
#endif
	}
	if (setBroadcast)
	{
		// Note: Fails with VDP but not xbox
		// Set broadcast capable
		sock_opt=1;
		if ( setsockopt__(listenSocket, SOL_SOCKET, SO_BROADCAST, ( char * ) & sock_opt, sizeof( sock_opt ) ) == -1 )
		{
#if defined(_WIN32) && defined(_DEBUG)
#if  !defined(WINDOWS_PHONE_8)
			DWORD dwIOError = GetLastError();
			// On Vista, can get WSAEACCESS (10013)
			// See http://support.microsoft.com/kb/819124
			// http://blogs.msdn.com/wndp/archive/2007/03/19/winsock-so-exclusiveaddruse-on-vista.aspx
			// http://msdn.microsoft.com/en-us/library/ms740621(VS.85).aspx
			LPVOID messageBuffer;
			FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
				( LPTSTR ) & messageBuffer, 0, NULL );
			// something has gone wrong here...
			RAKNET_DEBUG_PRINTF( "setsockopt__(SO_BROADCAST) failed:Error code - %d\n%s", dwIOError, messageBuffer );
			//Free the buffer.
			LocalFree( messageBuffer );
#endif
#endif

		}

	}

#endif
}
/*
void SocketLayer::SetSocketOptions( RakNetSocket* listenSocket, bool blockingSocket, bool setBroadcast)
{
	SetSocketOptions(listenSocket->s, blockingSocket, setBroadcast);
}
RakNetSocket *SocketLayer::CreateBoundSocket_PS3Lobby( unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned short socketFamily )
{
	(void) port;
	(void) blockingSocket;
	(void) forceHostAddress;
	(void) socketFamily;

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
	(void) blockingSocket;

	int ret;
	RakNetSocket* listenSocket;
	listenSocket = RakNetSocket::Create(AF_INET, SOCK_DGRAM_P2P, 0);
	// Normal version as below

	if ( listenSocket == 0 )
		return 0;

	SetSocketOptions(listenSocket, blockingSocket, true);

	sockaddr_in_p2p listenerSocketAddress;
	memset(&listenerSocketAddress, 0, sizeof(listenerSocketAddress));
	listenerSocketAddress.sin_port = htons(SCE_NP_PORT);
	listenerSocketAddress.sin_vport = htons(port);

	// Fill in the rest of the address structure
	listenerSocketAddress.sin_family = AF_INET;

	// Binding specific addresses doesn't work with the PS3
	// The functions return success but broadcast messages, possibly more, never arrive
	listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;


	// bind our name to the socket
	ret = listenSocket->Bind( ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );

	if ( ret <= -1 )
	{
	switch (ret)
		{
		case EBADF:
			RAKNET_DEBUG_PRINTF("bind__(): sockfd is not a valid descriptor.\n"); break;
		case EINVAL:
			RAKNET_DEBUG_PRINTF("bind__(): The addrlen is wrong, or the socket was not in the AF_UNIX family.\n"); break;
		case EROFS:
			RAKNET_DEBUG_PRINTF("bind__(): The socket inode would reside on a read-only file system.\n"); break;
		case EFAULT:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr points outside the user's accessible address space.\n"); break;
		case ENAMETOOLONG:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr is too long.\n"); break;
		case ENOENT:
			RAKNET_DEBUG_PRINTF("bind__(): The file does not exist.\n"); break;
		case ENOMEM:
			RAKNET_DEBUG_PRINTF("bind__(): Insufficient kernel memory was available.\n"); break;
		case ENOTDIR:
			RAKNET_DEBUG_PRINTF("bind__(): A component of the path prefix is not a directory.\n"); break;
		case EACCES:
			RAKNET_DEBUG_PRINTF("bind__(): Search permission is denied on a component of the path prefix.\n"); break;
		default:
			RAKNET_DEBUG_PRINTF("Unknown bind__() error %i.\n", ret); break;
		}

		return 0;
	}

	return listenSocket;
#else
	return 0;
#endif
}
RakNetSocket *SocketLayer::CreateBoundSocket_PSP2( unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned short socketFamily )
{
	(void) port;
	(void) blockingSocket;
	(void) forceHostAddress;
	(void) socketFamily;

#if defined(SN_TARGET_PSP2)
	(void) blockingSocket;

	int ret;
	RakNetSocket* listenSocket;
	SceNetSockaddrIn listenerSocketAddress;
	memset(&listenerSocketAddress, 0, sizeof(listenerSocketAddress));

	// Lobby version
	listenerSocketAddress.sin_port = htons(SCE_NET_ADHOC_PORT);
	listenerSocketAddress.sin_vport = htons(port);
	listenSocket = RakNetSocket::Create( SCE_NET_AF_INET, SCE_NET_SOCK_DGRAM_P2P, 0 );

	// Normal version as below
	if ( listenSocket < 0 )
	{
		return 0;
	}

	SetSocketOptions(listenSocket, blockingSocket, true);

	// Fill in the rest of the address structure
	listenerSocketAddress.sin_family = AF_INET;

	if (forceHostAddress && forceHostAddress[0])
	{
#if defined(SN_TARGET_PSP2)
		inet_addr__( forceHostAddress, &listenerSocketAddress.sin_addr );
#else
		listenerSocketAddress.sin_addr.s_addr = inet_addr__( forceHostAddress );
#endif
	}
	else
	{
		listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
	}

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	// Binding specific addresses doesn't work with the PS3
	// The functions return success but broadcast messages, possibly more, never arrive
	listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
#endif


	// bind our name to the socket
	ret = listenSocket->Bind( ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );

	if ( ret <= -1 )
	{
		switch (ret)
		{
		case EBADF:
			RAKNET_DEBUG_PRINTF("bind__(): sockfd is not a valid descriptor.\n"); break;
		case EINVAL:
			RAKNET_DEBUG_PRINTF("bind__(): The addrlen is wrong, or the socket was not in the AF_UNIX family.\n"); break;
		case EROFS:
			RAKNET_DEBUG_PRINTF("bind__(): The socket inode would reside on a read-only file system.\n"); break;
		case EFAULT:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr points outside the user's accessible address space.\n"); break;
		case ENAMETOOLONG:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr is too long.\n"); break;
		case ENOENT:
			RAKNET_DEBUG_PRINTF("bind__(): The file does not exist.\n"); break;
		case ENOMEM:
			RAKNET_DEBUG_PRINTF("bind__(): Insufficient kernel memory was available.\n"); break;
		case ENOTDIR:
			RAKNET_DEBUG_PRINTF("bind__(): A component of the path prefix is not a directory.\n"); break;
		case EACCES:
			RAKNET_DEBUG_PRINTF("bind__(): Search permission is denied on a component of the path prefix.\n"); break;
		default:
			RAKNET_DEBUG_PRINTF("Unknown bind__() error %i.\n", ret); break;
		}

		return 0;
	}

	return listenSocket;
#else
	return 0;
#endif
}
*/

/*
#ifdef __native_client__
struct SocketAndBuffer
{
	RakPeer *peer;
	RakNetSocket* chromeSocket;
	char buffer[MAXIMUM_MTU_SIZE];
	int32_t dataSize;
	SystemAddress recvFromAddress;
	RakNet::TimeUS timeRead;
	SocketAndBuffer *next;
} *sabHead=0, *sabLast=0;
void onRecvFrom(void* pData, int32_t dataSize);
void ChromeRecvFrom(SocketAndBuffer *sab)
{	
	PP_CompletionCallback cc = PP_MakeCompletionCallback(onRecvFrom, sab);
	((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->RecvFrom(sab->chromeSocket->s, sab->buffer, MAXIMUM_MTU_SIZE, cc);
}

void onSendTo(void* pData, int32_t dataSize)
{
	if(dataSize <= 0)
		RAKNET_DEBUG_PRINTF("onSendTo: send failed with error %d\n", dataSize);

	// If the send was aborted due to the socket being destroyed, don't touch
	// the socket pointer since it might have been deallocated
	if(dataSize == PP_ERROR_ABORTED)
		return;

	// If another send was queued up, send it now for minimum latency
	RakNetSocket *s = (RakNetSocket *)pData;
	if (s->nextSendSize > 0)
	{
		int bufSize = s->nextSendSize;
		s->nextSendSize = 0;
		PP_CompletionCallback cc = PP_MakeCompletionCallback(onSendTo, s);
		((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->SendTo(s->s, s->nextSendBuffer, bufSize, &s->nextSendAddr, cc);
	}
	else
	{
		s->sendInProgress = false;
	}
}
void onRecvFrom(void* pData, int32_t dataSize)
{
	SocketAndBuffer *sab = (SocketAndBuffer *) pData;

	//any error codes will be given to us in the dataSize value; see pp_errors.h for a list of errors
	if(dataSize <=0 || !pData )
	{
		// This value indicates failure due to an asynchronous operation being
		// interrupted. The most common cause of this error code is destroying
		// a resource that still has a callback pending. All callbacks are
		// guaranteed to execute, so any callbacks pending on a destroyed
		// resource will be issued with PP_ERROR_ABORTED.
		if(dataSize==PP_ERROR_ABORTED)
		{
			RAKNET_DEBUG_PRINTF("onRecvFrom error PP_ERROR_ABORTED, killing recvfrom loop\n");
			RakNet::OP_DELETE(sab, _FILE_AND_LINE_);
		}
		else
		{
			RAKNET_DEBUG_PRINTF("onRecvFrom error %d\n", dataSize);

			// Reissue call
			PP_CompletionCallback cc = PP_MakeCompletionCallback(onRecvFrom, sab);
			((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->RecvFrom(sab->chromeSocket->s, sab->buffer, MAXIMUM_MTU_SIZE, cc);
		}

		return;
	}

	sab->dataSize=dataSize;
	sab->timeRead=RakNet::GetTimeUS();
	PP_NetAddress_Private addr;
	bool ok=false;
	if(((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->GetRecvFromAddress(sab->chromeSocket->s, &addr) == PP_TRUE)
	{
		PP_NetAddressFamily_Private family = NetAddressPrivate::GetFamily(addr);
		if (family == PP_NETADDRESSFAMILY_IPV4)
		{
			ok = NetAddressPrivate::GetAddress(addr, &sab->recvFromAddress.address.addr4.sin_addr, sizeof(in_addr));
		}
#if RAKNET_SUPPORT_IPV6==1
		else
		{
			ok = NetAddressPrivate::GetAddress(addr, &sab->recvFromAddress.address.addr6.sin6_addr, sizeof(in6_addr));
		}
#endif
	}

	if(ok)
	{
		sab->recvFromAddress.SetPortHostOrder(pp::NetAddressPrivate::GetPort(addr));

		if(sab->peer)
		{
			sab->peer->ProcessChromePacket(sab->chromeSocket, sab->buffer, sab->dataSize, sab->recvFromAddress, sab->timeRead);
		}
		else
		{
			// Add sab to linked list to read out
			if (sabHead==0)
				sabHead=sab;
			else
				sabLast->next=sab;
			sabLast=sab;
		}
	}

	// Call again
	ChromeRecvFrom(sab);
}
struct ChromeSocketContainer
{
	RakPeer *peer;
	RakNetSocket *s;
};
void onSocketBound(void* pData, int32_t dataSize)
{
	RAKNET_DEBUG_PRINTF("onSocketBound ==> %d\n", dataSize);

	//any error codes will be given to us in the dataSize value
	if(dataSize < 0)
	{
		fprintf(stderr,"onSocketBound exiting, dataSize = %d\n", dataSize);
		return;
	}

	ChromeSocketContainer *csc = (ChromeSocketContainer *)pData;
	// Call recvfrom the first time
	SocketAndBuffer *sab = RakNet::OP_NEW<SocketAndBuffer>(_FILE_AND_LINE_);
	sab->peer=csc->peer;
	sab->chromeSocket=csc->s;
	sab->next=0;
	ChromeRecvFrom(sab);
	RakNet::OP_DELETE(csc, _FILE_AND_LINE_);
}
RakNetSocket* CreateChromeSocket(RakPeer *peer, unsigned short port, const char *forceHostAddress, _PP_Instance_ chromeInstance, bool is_ipv6)
{
	if(Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))
	{
		RakNetSocket *s = RakNetSocket::Create(chromeInstance);
		// RakNet::OP_NEW<SocketImpl>(_FILE_AND_LINE_);
		//s->s = ((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->Create(chromeInstance);
		RAKNET_DEBUG_PRINTF("CreateChromeSocket(%d,%s,0x%08x,%d) ==> 0x%08x\n", port, forceHostAddress?forceHostAddress:"(null)",chromeInstance,is_ipv6, s->s);
		
		// Enable the broadcast feature on the socket (must happen before the
		// bind call)
		((PPB_UDPSocket_Private_0_4*) pp::Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->SetSocketFeature(s->s, PP_UDPSOCKETFEATURE_BROADCAST, PP_MakeBool(PP_TRUE));
		
		PP_NetAddress_Private client_addr;
		uint8_t ipv6[16], ipv4[4];
		if (forceHostAddress)
		{
			unsigned int ipIdx=0;
			char * pch;
			pch = strtok ((char*) forceHostAddress,".");
			if (is_ipv6)
			{
				while (pch != NULL && ipIdx<16)
				{
					ipv6[ipIdx++]=atoi(pch);
					pch = strtok (NULL, ".");
				}
				NetAddressPrivate::CreateFromIPv6Address(ipv6,0,port,&client_addr);
			}
			else
			{
				while (pch != NULL && ipIdx<4)
				{
					ipv4[ipIdx++]=atoi(pch);
					pch = strtok (NULL, ".");
				}
				NetAddressPrivate::CreateFromIPv4Address(ipv4,port,&client_addr);
			}
		}
		else
		{
			NetAddressPrivate::GetAnyAddress(is_ipv6, &client_addr);
			NetAddressPrivate::ReplacePort(client_addr, port, &client_addr);
		}
		ChromeSocketContainer *csc = RakNet::OP_NEW<ChromeSocketContainer>(_FILE_AND_LINE_);
		csc->peer=peer;
		csc->s=s;
		RAKNET_DEBUG_PRINTF("attempting to bind to %s\n", NetAddressPrivate::Describe(client_addr, true).c_str());
		PP_CompletionCallback cc = PP_MakeCompletionCallback(onSocketBound, csc);
		((PPB_UDPSocket_Private_0_4*) Module::Get()->GetBrowserInterface(PPB_UDPSOCKET_PRIVATE_INTERFACE_0_4))->Bind(s->s, &client_addr, cc);
		return s;
	}
	return 0;
}
#endif
*/
/*
#if defined(WINDOWS_STORE_RT)
RakNetSocket* SocketLayer::CreateWindowsStore8Socket( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, _PP_Instance_ chromeInstance )
{
	// Create the socket
	Windows::Networking::Sockets::DatagramSocket udpSocket = ref new DatagramSocket();

	// Set socket options

	// Bind the socket

	return true;
}
#endif
// void onSocketBound(void* pData, int32_t dataSize)
RakNetSocket *SocketLayer::CreateBoundSocket_IPV4( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, _PP_Instance_ chromeInstance )
{
#ifdef __native_client__
//	return CreateChromeSocket(peer,port,forceHostAddress,chromeInstance,false);
	return 0;
#else
	(void) peer;
	(void) chromeInstance;
	(void) sleepOn10048;

	int ret;
	RakNetSocket* listenSocket;
	sockaddr_in listenerSocketAddress;
	memset(&listenerSocketAddress,0,sizeof(sockaddr_in));
	// Listen on our designated Port#
	listenerSocketAddress.sin_port = htons( port );

//#if defined(SN_TARGET_PSP2)
//	listenSocket = socket__( AF_INET, SCE_NET_SOCK_DGRAM_P2P, extraSocketOptions );
//#else
	listenSocket = RakNetSocket::Create( AF_INET, SOCK_DGRAM, extraSocketOptions );
//#endif

	if ( listenSocket == 0 )
	{
#if defined(_WIN32) && !defined(_XBOX) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8) && !defined(_XBOX_720_COMPILE_AS_WINDOWS)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "socket__(...) failed:Error code - %d\n%s", dwIOError, messageBuffer );
		char str[512];
		strcpy(str,(const char*) messageBuffer);
		//Free the buffer.
		LocalFree( messageBuffer );
#endif

		return 0;
	}

	listenSocket->SetBlockingSocket(blockingSocket);
	listenSocket->SetExtraSocketOptions(extraSocketOptions);
	listenSocket->SetChromeInstance(chromeInstance);

	SetSocketOptions(listenSocket, blockingSocket, true);

	// Fill in the rest of the address structure
	listenerSocketAddress.sin_family = AF_INET;

	if (forceHostAddress && forceHostAddress[0])
	{
		//		RAKNET_DEBUG_PRINTF("Force binding %s:%i\n", forceHostAddress, port);
#if defined(SN_TARGET_PSP2)
		inet_addr__( forceHostAddress, &listenerSocketAddress.sin_addr );
#else
		listenerSocketAddress.sin_addr.s_addr = inet_addr__( forceHostAddress );
#endif
	}
	else
	{
		//		RAKNET_DEBUG_PRINTF("Binding any on port %i\n", port);
		listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
	}

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	// Binding specific addresses doesn't work with the PS3
	// The functions return success but broadcast messages, possibly more, never arrive
	listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
#endif


	// bind our name to the socket
	ret = listenSocket->Bind( ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );

	if ( ret <= -1 )
	{
#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
		ret = sys_net_errno;
		// See Error Codes for Socket Functions
		// Error codes of socket functions obtained with sys_net_errno

#endif

		RakNet::OP_DELETE(listenSocket,_FILE_AND_LINE_);

#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_WITH_XBOX_LIVE) && !defined(X360)
		DWORD dwIOError = GetLastError();
		if (dwIOError==10048)
		{
			if (sleepOn10048==0)
				return 0;
			// Vista has a bug where it returns WSAEADDRINUSE (10048) if you create, shutdown, then rebind the socket port unless you wait a while first.
			// Wait, then rebind
			RakSleep(100);

			listenerSocketAddress.sin_port = htons( port );
			listenSocket = RakNetSocket::Create( AF_INET, SOCK_DGRAM, 0 );
			if ( listenSocket == 0 )
				return false;

			listenSocket->SetBlockingSocket(blockingSocket);
			listenSocket->SetExtraSocketOptions(extraSocketOptions);
			listenSocket->SetChromeInstance(chromeInstance);

			SetSocketOptions(listenSocket, blockingSocket, true);

			// Fill in the rest of the address structure
			listenerSocketAddress.sin_family = AF_INET;
			if (forceHostAddress && forceHostAddress[0])
			{
#if defined(SN_TARGET_PSP2)
				inet_addr__( forceHostAddress, &listenerSocketAddress.sin_addr );
#else
				listenerSocketAddress.sin_addr.s_addr = inet_addr__( forceHostAddress );
#endif
			}
			else
				listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;

			// bind our name to the socket
			ret = listenSocket->Bind( ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );

			if ( ret >= 0 )
				return listenSocket;
		}

		#if !defined(WINDOWS_PHONE_8) && !defined(_XBOX_720_COMPILE_AS_WINDOWS)
		dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "bind__(...) failed:Error code - %d\n%s", (unsigned int) dwIOError, (char*) messageBuffer );
		//Free the buffer.
		LocalFree( messageBuffer );
		#endif
#elif (defined(__GNUC__) || defined(__GCCXML__) || defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)  ) && !defined(_WIN32)
		switch (ret)
		{
		case EBADF:
			RAKNET_DEBUG_PRINTF("bind__(): sockfd is not a valid descriptor.\n"); break;
#if !defined(_PS3) && !defined(__PS3__) && !defined(SN_TARGET_PS3) && !defined(_PS4) && !defined(SN_TARGET_PSP2)
		case ENOTSOCK:
			RAKNET_DEBUG_PRINTF("bind__(): Argument is a descriptor for a file, not a socket.\n"); break;
#endif
		case EINVAL:
			RAKNET_DEBUG_PRINTF("bind__(): The addrlen is wrong, or the socket was not in the AF_UNIX family.\n"); break;
		case EROFS:
			RAKNET_DEBUG_PRINTF("bind__(): The socket inode would reside on a read-only file system.\n"); break;
		case EFAULT:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr points outside the user's accessible address space.\n"); break;
		case ENAMETOOLONG:
			RAKNET_DEBUG_PRINTF("bind__(): my_addr is too long.\n"); break;
		case ENOENT:
			RAKNET_DEBUG_PRINTF("bind__(): The file does not exist.\n"); break;
		case ENOMEM:
			RAKNET_DEBUG_PRINTF("bind__(): Insufficient kernel memory was available.\n"); break;
		case ENOTDIR:
			RAKNET_DEBUG_PRINTF("bind__(): A component of the path prefix is not a directory.\n"); break;
		case EACCES:
			RAKNET_DEBUG_PRINTF("bind__(): Search permission is denied on a component of the path prefix.\n"); break;
#if !defined(_PS3) && !defined(__PS3__) && !defined(SN_TARGET_PS3) && !defined(_PS4) && !defined(SN_TARGET_PSP2)
		case ELOOP:
			RAKNET_DEBUG_PRINTF("bind__(): Too many symbolic links were encountered in resolving my_addr.\n"); break;
#endif
		default:
			RAKNET_DEBUG_PRINTF("Unknown bind__() error %i.\n", ret); break;
		}
#endif

		return 0;
	}

	return listenSocket;


#endif
}
#if RAKNET_SUPPORT_IPV6==1
RakNetSocket *SocketLayer::CreateBoundSocket_SupportIPV4And6( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, unsigned short socketFamily, _PP_Instance_ chromeInstance )
{
#ifdef _WIN32
	// Vista has a bug where it returns WSAEADDRINUSE (10048) if you create, shutdown, then rebind the socket port unless you wait a while first.
	if (sleepOn10048==0)
		RakSleep(100);
#endif

	(void) chromeInstance;
	(void) extraSocketOptions;
	(void) peer;

	int ret=0;
	RakNetSocket* listenSocket;
	struct addrinfo hints;
	struct addrinfo *servinfo=0, *aip;  // will point to the results
	PrepareAddrInfoHints(&hints);
	hints.ai_family=socketFamily;
	char portStr[32];
	Itoa(port,portStr,10);

	// On Ubuntu, "" returns "No address associated with hostname" while 0 works.
	if (forceHostAddress && 
		(_stricmp(forceHostAddress,"UNASSIGNED_SYSTEM_ADDRESS")==0 || forceHostAddress[0]==0))
	{
		getaddrinfo(0, portStr, &hints, &servinfo);
	}
	else
	{
		getaddrinfo(forceHostAddress, portStr, &hints, &servinfo);
	}

	// Try all returned addresses until one works
	for (aip = servinfo; aip != NULL; aip = aip->ai_next)
	{
		// Open socket. The address type depends on what
		// getaddrinfo() gave us.
		listenSocket = RakNetSocket::Create(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
		if (listenSocket )
		{
			ret = listenSocket->Bind( aip->ai_addr, (int) aip->ai_addrlen );
			if (ret>=0)
			{
				// Is this valid?
				sockaddr_in6 addr6;
				memcpy(&addr6, aip->ai_addr, sizeof(addr6));

				freeaddrinfo(servinfo); // free the linked-list

				SetSocketOptions(listenSocket, blockingSocket, true);
				return listenSocket;
			}
			else
			{
				RakNet::OP_DELETE(listenSocket,_FILE_AND_LINE_);
			}
		}
	}

#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(X360)
	DWORD dwIOError = GetLastError();
	LPVOID messageBuffer;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
		( LPTSTR ) & messageBuffer, 0, NULL );
	// something has gone wrong here...

	RAKNET_DEBUG_PRINTF( "bind__(...) failed:Error code - %d\n%s", (unsigned int) dwIOError, (char*) messageBuffer );
	//Free the buffer.
	LocalFree( messageBuffer );
#elif defined(__GNUC__)  || defined(__GCCXML__) && !defined(_WIN32)
	switch (ret)
	{
	case EBADF:
		RAKNET_DEBUG_PRINTF("bind__(): sockfd is not a valid descriptor.\n"); break;
#if !defined(_PS3) && !defined(__PS3__) && !defined(SN_TARGET_PS3) && !defined(_PS4) && !defined(SN_TARGET_PSP2)
	case ENOTSOCK:
		RAKNET_DEBUG_PRINTF("bind__(): Argument is a descriptor for a file, not a socket.\n"); break;
#endif
	case EINVAL:
		RAKNET_DEBUG_PRINTF("bind__(): The addrlen is wrong, or the socket was not in the AF_UNIX family.\n"); break;
	case EROFS:
		RAKNET_DEBUG_PRINTF("bind__(): The socket inode would reside on a read-only file system.\n"); break;
	case EFAULT:
		RAKNET_DEBUG_PRINTF("bind__(): my_addr points outside the user's accessible address space.\n"); break;
	case ENAMETOOLONG:
		RAKNET_DEBUG_PRINTF("bind__(): my_addr is too long.\n"); break;
	case ENOENT:
		RAKNET_DEBUG_PRINTF("bind__(): The file does not exist.\n"); break;
	case ENOMEM:
		RAKNET_DEBUG_PRINTF("bind__(): Insufficient kernel memory was available.\n"); break;
	case ENOTDIR:
		RAKNET_DEBUG_PRINTF("bind__(): A component of the path prefix is not a directory.\n"); break;
	case EACCES:
		RAKNET_DEBUG_PRINTF("bind__(): Search permission is denied on a component of the path prefix.\n"); break;
#if !defined(_PS3) && !defined(__PS3__) && !defined(SN_TARGET_PS3) && !defined(_PS4) && !defined(SN_TARGET_PSP2)
	case ELOOP:
		RAKNET_DEBUG_PRINTF("bind__(): Too many symbolic links were encountered in resolving my_addr.\n"); break;
#endif
	default:
		RAKNET_DEBUG_PRINTF("Unknown bind__() error %i.\n", ret); break;
	}
#endif

	return 0;
}
#endif // RAKNET_SUPPORT_IPV6==1
RakNetSocket *SocketLayer::CreateBoundSocket( RakPeer *peer, unsigned short port, bool blockingSocket, const char *forceHostAddress, unsigned int sleepOn10048, unsigned int extraSocketOptions, unsigned short socketFamily, _PP_Instance_ chromeInstance )
{
	(void) peer;
	(void) blockingSocket;
	(void) extraSocketOptions;
	(void) socketFamily;
	(void) chromeInstance;

#ifdef __native_client__
	// eturn CreateChromeSocket(peer,port,forceHostAddress,chromeInstance,true);
	return 0;
#elif defined(WINDOWS_STORE_RT)
	return CreateWindowsStore8Socket(peer,port,blockingSocket,forceHostAddress,sleepOn10048,extraSocketOptions, chromeInstance);
#else
	(void) blockingSocket;

	#if RAKNET_SUPPORT_IPV6!=1
		return CreateBoundSocket_IPV4(peer,port,blockingSocket,forceHostAddress,sleepOn10048,extraSocketOptions, chromeInstance);
	#else
		return CreateBoundSocket_SupportIPV4And6(peer,port,blockingSocket,forceHostAddress,sleepOn10048,extraSocketOptions, socketFamily, chromeInstance);
	#endif

#endif
}
*/
/*
const char* SocketLayer::DomainNameToIP_Old( const char *domainName )
{
	static struct in_addr addr;
	memset(&addr,0,sizeof(in_addr));

#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360)
	WSAEVENT hEvent = WSACreateEvent();
	XNDNS* pDns = NULL;

	HRESULT err = XNetDnsLookup( domainName, hEvent, &pDns );
	WaitForSingleObject( hEvent, 1000 );
	WSACloseEvent( hEvent );

	if( pDns )
	{
		const unsigned int ADDR_BUFF_SIZE = 16;		//Enough room for "xxx.xxx.xxx.xxx\0"
		static char addrBuff[ADDR_BUFF_SIZE] = {0};

		if( pDns->iStatus == 0 )
		{
			memcpy( &addr, &pDns->aina[0].s_addr, sizeof( struct in_addr ) );
			XNetInAddrToString(addr, addrBuff, ADDR_BUFF_SIZE);
		}
		else
		{
			//error "DNS lookup failed: %u", pDns->iStatus
		}
		XNetDnsRelease( pDns );

		return addrBuff;
	}
	else
	{
		//error "DNS lookup failed: %u", err;
	}
#elif defined(WINDOWS_STORE_RT)
	// Do I use http://msdn.microsoft.com/en-US/library/windows/apps/windows.networking.sockets.datagramsocketinformation for this?
	// Perhaps DatagramSocket., followed by GetEndpointParisAsync
	RakAssert("Not yet supported" && 0);
#elif defined(SN_TARGET_PSP2)
	inet_addr__(domainName, &addr);
	return inet_ntoa( addr );
#else
	// Use inet_addr instead? What is the difference?
	struct hostent * phe = gethostbyname( domainName );

	if ( phe == 0 || phe->h_addr_list[ 0 ] == 0 )
	{
		//cerr << "Yow! Bad host lookup." << endl;
		return 0;
	}

	if (phe->h_addr_list[ 0 ]==0)
		return 0;

	memcpy( &addr, phe->h_addr_list[ 0 ], sizeof( struct in_addr ) );
	return inet_ntoa( addr );
#endif

	return "";
}
*/
/*
const char* SocketLayer::DomainNameToIP( const char *domainName )
{
#if RAKNET_SUPPORT_IPV6!=1
	return DomainNameToIP_Old(domainName);
#else

#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360)
	struct in_addr addr;
	WSAEVENT hEvent = WSACreateEvent();
	XNDNS* pDns = NULL;

	HRESULT err = XNetDnsLookup( domainName, hEvent, &pDns );
	WaitForSingleObject( hEvent, 1000 );
	WSACloseEvent( hEvent );

	if( pDns )
	{
		const unsigned int ADDR_BUFF_SIZE = 16;		//Enough room for "xxx.xxx.xxx.xxx\0"
		static char addrBuff[ADDR_BUFF_SIZE] = {0};

		if( pDns->iStatus == 0 )
		{
			memcpy( &addr, &pDns->aina[0].s_addr, sizeof( struct in_addr ) );
			XNetInAddrToString(addr, addrBuff, ADDR_BUFF_SIZE);
		}
		else
		{
			//error "DNS lookup failed: %u", pDns->iStatus
		}
		XNetDnsRelease( pDns );

		return addrBuff;
	}
	else
	{
		//error "DNS lookup failed: %u", err;
	}
#else

	struct addrinfo hints, *res, *p;
	int status;
	static char ipstr[INET6_ADDRSTRLEN];
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // AF_INET or AF_INET6 to force version
	hints.ai_socktype = SOCK_DGRAM;

	if ((status = getaddrinfo(domainName, NULL, &hints, &res)) != 0) {
		return 0;
	}

	p=res;
// 	for(p = res;p != NULL; p = p->ai_next) {
		void *addr;
//		char *ipver;

		// get the pointer to the address itself,
		// different fields in IPv4 and IPv6:
		if (p->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
			addr = &(ipv4->sin_addr);
			strcpy(ipstr, inet_ntoa( ipv4->sin_addr ));
		} 
		else
		{
			// TODO - test
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
			addr = &(ipv6->sin6_addr);
			// inet_ntop function does not exist on windows
			// http://www.mail-archive.com/users@ipv6.org/msg02107.html
			getnameinfo((struct sockaddr *)ipv6, sizeof(struct sockaddr_in6), ipstr, 1, NULL, 0, NI_NUMERICHOST);
		}
		freeaddrinfo(res); // free the linked list
		return ipstr;
//	}

#endif

	return "";

#endif // #if RAKNET_SUPPORT_IPV6!=1
}
*/

/*
void SocketLayer::Write( RakNetSocket*writeSocket, const char* data, const int length )
{
#ifdef __native_client__
#else

#ifdef _DEBUG
	RakAssert( writeSocket != 0 );
#endif

	writeSocket->Send( data, length, 0 );
#endif
}
#if defined(WINDOWS_STORE_RT)
void SocketLayer::RecvFromBlocking_WindowsStore8( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead )
{
}
#endif
void SocketLayer::RecvFromBlocking_IPV4( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead )
{
	(void) rakPeer;

#ifndef __native_client__

	sockaddr* sockAddrPtr;
	socklen_t sockLen;
	socklen_t* socketlenPtr=(socklen_t*) &sockLen;
	sockaddr_in sa;
	memset(&sa,0,sizeof(sockaddr_in));
	int dataOutSize;
	const int flag=0;

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
	sockaddr_in_p2p sap2p;
	if (s->GetRemotePortRakNetWasStartedOn()!=0)
	{
		sockLen=sizeof( sap2p );
		sap2p.sin_family = AF_INET;
		sockAddrPtr=(sockaddr*) &sap2p;
	}
	else
#elif defined(SN_TARGET_PSP2)
	SceNetSockaddrIn sap2p;
	if (s->GetRemotePortRakNetWasStartedOn()!=0)
	{
		sockLen=sizeof( sap2p );
		sap2p.sin_family = AF_INET;
		sockAddrPtr=(sockaddr*) &sap2p;
	}
	else
#endif
	{
		sockLen=sizeof(sa);
		sa.sin_family = AF_INET;
		sa.sin_port=0;
		sockAddrPtr=(sockaddr*) &sa;
	}

#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
	dataOutSize=MAXIMUM_MTU_SIZE*2;
#else
	dataOutSize=MAXIMUM_MTU_SIZE;
#endif

	*bytesReadOut = s->RecvFrom( dataOut, dataOutSize, flag, sockAddrPtr, socketlenPtr );
#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
	if (s->GetExtraSocketOptions()==IPPROTO_VDP)
	{
		if (*bytesReadOut<2)
			return;
		*bytesReadOut=*bytesReadOut-2;
		memmove(dataOut,dataOut+2,*bytesReadOut);
	}
#endif
	if (*bytesReadOut<=0)
		return;
	*timeRead=RakNet::GetTimeUS();

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	if (s->GetRemotePortRakNetWasStartedOn()!=0)
	{
		systemAddressOut->SetPortNetworkOrder( sap2p.sin_port );
		systemAddressOut->address.addr4.sin_addr.s_addr = sap2p.sin_addr.s_addr;
	}
	else
#endif
	{
		systemAddressOut->SetPortNetworkOrder( sa.sin_port );
		systemAddressOut->address.addr4.sin_addr.s_addr=sa.sin_addr.s_addr;
	}

#endif // __native_client__
}
#if RAKNET_SUPPORT_IPV6==1
void SocketLayer::RecvFromBlockingIPV4And6( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead )
{
	(void) rakPeer;
	sockaddr_storage their_addr;
	sockaddr* sockAddrPtr;
	socklen_t sockLen;
	socklen_t* socketlenPtr=(socklen_t*) &sockLen;
	memset(&their_addr,0,sizeof(their_addr));
	int dataOutSize;
	const int flag=0;

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	sockaddr_in_p2p sap2p;
	if (remotePortRakNetWasStartedOn_PS3!=0)
	{
		sockLen=sizeof( sap2p );
		sap2p.sin_family = AF_INET;
		sockAddrPtr=(sockaddr*) &sap2p;
	}
	else
#endif
	{
		sockLen=sizeof(their_addr);
		sockAddrPtr=(sockaddr*) &their_addr;
	}

#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
	dataOutSize=MAXIMUM_MTU_SIZE*2;
#else
	dataOutSize=MAXIMUM_MTU_SIZE;
#endif

	*bytesReadOut = s->RecvFrom( dataOut, dataOutSize, flag, sockAddrPtr, socketlenPtr );
#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
	if (extraSocketOptions==IPPROTO_VDP)
	{
		if (*bytesReadOut<2)
			return;
		*bytesReadOut=*bytesReadOut-2;
		memmove(dataOut,dataOut+2,*bytesReadOut);
	}
#endif
	if (*bytesReadOut<=0)
		return;
	*timeRead=RakNet::GetTimeUS();

#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
	if (remotePortRakNetWasStartedOn_PS3!=0)
	{
		systemAddressOut->port = ntohs( sap2p.sin_port );
		systemAddressOut->binaryAddress = sap2p.sin_addr.s_addr;
	}
	else
#endif
	{
		if (their_addr.ss_family==AF_INET)
		{
			memcpy(&systemAddressOut->address.addr4,(sockaddr_in *)&their_addr,sizeof(sockaddr_in));
			systemAddressOut->debugPort=ntohs(systemAddressOut->address.addr4.sin_port);
			//	systemAddressOut->address.addr4.sin_port=ntohs( systemAddressOut->address.addr4.sin_port );
		}
		else
		{
			memcpy(&systemAddressOut->address.addr6,(sockaddr_in6 *)&their_addr,sizeof(sockaddr_in6));
			systemAddressOut->debugPort=ntohs(systemAddressOut->address.addr6.sin6_port);
			//	systemAddressOut->address.addr6.sin6_port=ntohs( systemAddressOut->address.addr6.sin6_port );
		}
	}
}
#endif
void SocketLayer::RecvFromBlocking( RakNetSocket *s, RakPeer *rakPeer, char *dataOut, int *bytesReadOut, SystemAddress *systemAddressOut, RakNet::TimeUS *timeRead )
{

#if defined(WINDOWS_STORE_RT)
	return RecvFromBlocking_WindowsStore8(s, rakPeer, dataOut, bytesReadOut, systemAddressOut, timeRead);
#else

	#if RAKNET_SUPPORT_IPV6!=1
		RecvFromBlocking_IPV4(s,rakPeer,dataOut,bytesReadOut,systemAddressOut,timeRead);
	#else
		RecvFromBlockingIPV4And6(s,rakPeer,dataOut,bytesReadOut,systemAddressOut,timeRead);
	#endif // defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4) || defined(SN_TARGET_PSP2)
#endif

}

int SocketLayer::SendTo_PS3Lobby( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress )
{
	(void) s;
	(void) data;
	(void) length;
	(void) systemAddress;

	int len=0;
#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
	sockaddr_in_p2p sa;
	memset(&sa, 0, sizeof(sa));
	// LAME!!!! You have to know the behind-nat port on the recipient! Just guessing it is the same as our own
	sa.sin_vport = htons(s->GetRemotePortRakNetWasStartedOn());
	sa.sin_port = systemAddress.GetPortNetworkOrder(); // Port returned from signaling
	sa.sin_addr.s_addr = systemAddress.address.addr4.sin_addr.s_addr;
	sa.sin_family = AF_INET;
	do
	{
		len = s->SendTo( data, length, 0, ( const sockaddr* ) & sa, sizeof( sa ) );
	}
	while ( len == 0 );
#endif
	return len;
}
int SocketLayer::SendTo_PSP2( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress )
{
	(void) s;
	(void) data;
	(void) length;
	(void) systemAddress;

	int len=0;
#if defined(SN_TARGET_PSP2)
	SceNetSockaddrIn sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_vport = htons(s->GetRemotePortRakNetWasStartedOn());
	sa.sin_port = systemAddress.GetPortNetworkOrder();
	sa.sin_addr.s_addr = systemAddress.address.addr4.sin_addr.s_addr;
	sa.sin_family = SCE_NET_AF_INET;
	do
	{
		len = s->SendTo( data, length, 0, ( const sockaddr* ) & sa, sizeof( sa ) );
	}
	while ( len == 0 );
#endif
	return len;
}
int SocketLayer::SendTo_360( RakNetSocket *s, const char *data, int length, const char *voiceData, int voiceLength, const SystemAddress &systemAddress )
{
	(void) s;
	(void) data;
	(void) length;
	(void) voiceData;
	(void) voiceLength;
	(void) systemAddress;

	int len=0;
#if defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
	if (s->GetExtraSocketOptions()!=IPPROTO_VDP)
	{
		return SendTo_PC(s,data,length,systemAddress,_FILE_AND_LINE_);
	}

	unsigned short payloadLength=(unsigned short) length;

	WSABUF buffers[3];
	buffers[0].buf=(char*) &payloadLength;
	buffers[0].len=sizeof(payloadLength);
	buffers[1].buf=(char*) data;
	buffers[1].len=length;
	buffers[2].buf=(char*) voiceData;
	buffers[2].len=voiceLength;
	DWORD size = buffers[0].len + buffers[1].len + buffers[2].len;

	sockaddr_in sa;
	memset(&sa,0,sizeof(sockaddr_in));
	sa.sin_port = systemAddress.GetPortNetworkOrder();
	sa.sin_addr.s_addr = systemAddress.address.addr4.sin_addr.s_addr;
	sa.sin_family = AF_INET;

	// TODO - might have to nuke this for GFWL
	int result = s->_WSASendTo((LPWSABUF)buffers, 3, &size, 0, ( const sockaddr* ) & sa, sizeof( sa ), NULL, NULL);
	if (result==-1)
	{
//		DWORD dwIOError = GetLastError();
//		int a=5;
	}
	len=size;


#endif
	return len;
}
#if defined(WINDOWS_STORE_RT)
int SocketLayer::SendTo_WindowsStore8( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress, const char *file, const long line )
{

	return 1;
}
#endif
int SocketLayer::SendTo_PC( RakNetSocket *s, const char *data, int length, const SystemAddress &systemAddress, const char *file, const long line )
{
#ifdef __native_client__
	return -1;
#else
	// TODO
	// http://www.linuxquestions.org/questions/linux-software-2/ipv6-linux-sendto-problems-519485/

// #if RAKNET_SUPPORT_IPV6==1
// 	RakAssert(
// 		systemAddress.address.addr4.sin_family!=AF_MAX &&
// 		(systemAddress.address.addr4.sin_family==AF_INET || (systemAddress.address.addr6.sin6_scope_id!=0))
// 		);
// #endif


	int len=0;
	do
	{
#ifdef DEBUG_SENDTO_SPIKES
		RakNetTime start = RakNet::GetTime();
#else
		(void) file;
		(void) line;
#endif
		if (systemAddress.address.addr4.sin_family==AF_INET)
		{
			//systemAddress.address.addr4.sin_port=htons(systemAddress.address.addr4.sin_port);
			len = s->SendTo( data, length, 0, ( const sockaddr* ) & systemAddress.address.addr4, sizeof( sockaddr_in ) );
			//systemAddress.address.addr4.sin_port=ntohs(systemAddress.address.addr4.sin_port);
		}
		else
		{
#if RAKNET_SUPPORT_IPV6==1
		//	systemAddress.address.addr6.sin6_port=htons(systemAddress.address.addr6.sin6_port);
			len = s->SendTo( data, length, 0, ( const sockaddr* ) & systemAddress.address.addr6, sizeof( sockaddr_in6 ) );
			//systemAddress.address.addr6.sin6_port=ntohs(systemAddress.address.addr6.sin6_port);
#endif
		}

#ifdef DEBUG_SENDTO_SPIKES
		RakNetTime end = RakNet::GetTime();
		static unsigned int callCount=1;
		RAKNET_DEBUG_PRINTF("%i. SendTo_PC, time=%" PRINTF_64_BIT_MODIFIER "u, elapsed=%" PRINTF_64_BIT_MODIFIER "u, length=%i, returned=%i, binaryAddress=%i, port=%i, file=%s, line=%i\n", callCount++, end, end-start, length, len, binaryAddress, port, file, line);
#endif
		if (len<0)
		{
			RAKNET_DEBUG_PRINTF("sendto failed with code %i for char %i and length %i.\n", len, data[0], length);
		}
	}
	while ( len == 0 );
	return len;
#endif  // __native_client__
}

#ifdef _MSC_VER
#pragma warning( disable : 4702 ) // warning C4702: unreachable code
#endif
int SocketLayer::SendTo( RakNetSocket *s, const char *data, int length, SystemAddress systemAddress, const char *file, const long line )
{
	int len=0;
	RakAssert(length<=MAXIMUM_MTU_SIZE-UDP_HEADER_SIZE);
#if !defined(SN_TARGET_PSP2) && !defined(__S3E__)
	RakAssert(systemAddress.address.addr4.sin_family!=AF_MAX);
#endif
// 
// 	if (slo)
// 	{
// 		len = slo->RakNetSendTo(data,length,systemAddress);
// 		if ( len != -1 )
// 			return 0;
// 		return 1;
// 	}

	if ( s == 0 )
	{
		return -1;
	}


	if (s->GetRemotePortRakNetWasStartedOn()!=0)
	{
	#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3) || defined(_PS4)
		len = SendTo_PS3Lobby(s,data,length,systemAddress);
	#elif defined(SN_TARGET_PSP2)
		len = SendTo_PSP2(s,data,length,systemAddress);
	#endif
	}
	else
	{


#if defined(WINDOWS_STORE_RT)
		len = SendTo_WindowsStore8(s,data,length,systemAddress,file,line);
#elif defined(_XBOX) || defined(_XBOX_720_WITH_XBOX_LIVE) || defined(X360) || defined(GFWL)
		if (s->GetExtraSocketOptions()==IPPROTO_VDP)
			len = SendTo_360(s,data,length,0,0,systemAddress);
		else
#endif
		len = SendTo_PC(s,data,length,systemAddress,file,line);

	}

	if ( len != -1 )
		return 0;

#if defined(_WIN32) && !defined(_WIN32_WCE) && !defined(WINDOWS_STORE_RT)

	DWORD dwIOError = WSAGetLastError();

	if ( dwIOError == WSAECONNRESET )
	{
#if defined(_DEBUG)
		RAKNET_DEBUG_PRINTF( "A previous send operation resulted in an ICMP Port Unreachable message.\n" );
#endif

	}
	else if ( dwIOError != WSAEWOULDBLOCK && dwIOError != WSAEADDRNOTAVAIL)
	{
#if defined(_WIN32) && !defined(_XBOX) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(X360) && defined(_DEBUG) && !defined(_XBOX_720_COMPILE_AS_WINDOWS) && !defined(WINDOWS_PHONE_8)
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "sendto failed:Error code - %d\n%s", dwIOError, messageBuffer );

		//Free the buffer.
		LocalFree( messageBuffer );
#endif

	}

	return dwIOError;
#endif

	return 1; // error
}
// Not enough info for IPV6
// int SocketLayer::SendTo( UDPSOCKET s, const char *data, int length, const char ip[ 16 ], unsigned short port, unsigned short remotePortRakNetWasStartedOn_PS3, unsigned int extraSocketOptions, const char *file, const long line )
// {
// 	SystemAddress systemAddress;
// 	systemAddress.FromStringAndPort(ip,port);
// 	return SendTo( s, data, length, systemAddress,remotePortRakNetWasStartedOn_PS3, extraSocketOptions, file, line );
// }
int SocketLayer::SendToTTL( RakNetSocket *s, const char *data, int length, SystemAddress &systemAddress, int ttl )
{
// 	if (slo)
// 		return slo->RakNetSendTo(data,length,systemAddress);

#if !defined(_XBOX) && !defined(_XBOX_720_WITH_XBOX_LIVE) && !defined(X360) && !defined(__native_client__)
	int oldTTL;
	socklen_t opLen=sizeof(oldTTL);
	// Get the current TTL
	if (s->GetSockOpt(systemAddress.GetIPPROTO(), IP_TTL, ( char * ) & oldTTL, &opLen ) == -1)
	{
#if defined(_WIN32) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8) && !defined(_XBOX_720_COMPILE_AS_WINDOWS)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "getsockopt__(IPPROTO_IP,IP_TTL) failed:Error code - %d\n%s", dwIOError, messageBuffer );
		//Free the buffer.
		LocalFree( messageBuffer );
#endif
	}

	// Set to TTL
	int newTTL=ttl;
	if (s->SetSockOpt(systemAddress.GetIPPROTO(), IP_TTL, ( char * ) & newTTL, sizeof ( newTTL ) ) == -1)
	{

#if defined(_WIN32) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8) && !defined(_XBOX_720_COMPILE_AS_WINDOWS)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "setsockopt__(IPPROTO_IP,IP_TTL) failed:Error code - %d\n%s", dwIOError, messageBuffer );
		//Free the buffer.
		LocalFree( messageBuffer );
#endif
	}

	// Send
	int res = SendTo(s,data,length,systemAddress, __FILE__, __LINE__ );

	// Restore the old TTL
	s->SetSockOpt(systemAddress.GetIPPROTO(), IP_TTL, ( char * ) & oldTTL, opLen );

	return res;
#else
	return 0;
#endif
}
*/

RakNet::RakString SocketLayer::GetSubNetForSocketAndIp(__UDPSOCKET__ inSock, RakNet::RakString inIpString)
{
	RakNet::RakString netMaskString;
	RakNet::RakString ipString;





#if   defined(WINDOWS_STORE_RT)
	RakAssert("Not yet supported" && 0);
	return "";
#elif defined(_WIN32)
	INTERFACE_INFO InterfaceList[20];
	unsigned long nBytesReturned;
	if (WSAIoctl(inSock, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList,
		sizeof(InterfaceList), &nBytesReturned, 0, 0) == SOCKET_ERROR) {
			return "";
	}

	int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);

	for (int i = 0; i < nNumInterfaces; ++i)
	{
		sockaddr_in *pAddress;
		pAddress = (sockaddr_in *) & (InterfaceList[i].iiAddress);
		ipString=inet_ntoa(pAddress->sin_addr);

		if (inIpString==ipString)
		{
			pAddress = (sockaddr_in *) & (InterfaceList[i].iiNetmask);
			netMaskString=inet_ntoa(pAddress->sin_addr);
			return netMaskString;
		}
	}
	return "";
#else

	int fd,fd2;
	fd2 = socket__(AF_INET, SOCK_DGRAM, 0);

	if(fd2 < 0)
	{
		return "";
	}

	struct ifconf ifc;
	char          buf[1999];
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;
	if(ioctl(fd2, SIOCGIFCONF, &ifc) < 0)
	{
		return "";
	}

	struct ifreq *ifr;
	ifr         = ifc.ifc_req;
	int intNum = ifc.ifc_len / sizeof(struct ifreq);
	for(int i = 0; i < intNum; i++)
	{
		ipString=inet_ntoa(((struct sockaddr_in *)&ifr[i].ifr_addr)->sin_addr);

		if (inIpString==ipString)
		{
			struct ifreq ifr2;
			fd = socket__(AF_INET, SOCK_DGRAM, 0);
			if(fd < 0)
			{
				return "";
			}
			ifr2.ifr_addr.sa_family = AF_INET;

			strncpy(ifr2.ifr_name, ifr[i].ifr_name, IFNAMSIZ-1);

			ioctl(fd, SIOCGIFNETMASK, &ifr2);

			close(fd);
			close(fd2);
			netMaskString=inet_ntoa(((struct sockaddr_in *)&ifr2.ifr_addr)->sin_addr);

			return netMaskString;
		}
	}

	close(fd2);
	return "";

#endif

}












































































#if   defined(WINDOWS_STORE_RT)
void GetMyIP_WinRT( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{
	// Perhaps DatagramSocket.BindEndpointAsynch, use localHostName as an empty string, then query what it bound to?
	RakAssert("Not yet supported" && 0);
}
#else
void GetMyIP_Win32( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{
	int idx=0;
	idx=0;
	char ac[ 80 ];
	if ( gethostname( ac, sizeof( ac ) ) == -1 )
	{
 #if defined(_WIN32) && !defined(WINDOWS_PHONE_8)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "gethostname failed:Error code - %d\n%s", dwIOError, messageBuffer );
		//Free the buffer.
		LocalFree( messageBuffer );
		#endif
		return ;
	}


#if RAKNET_SUPPORT_IPV6==1
	struct addrinfo hints;
	struct addrinfo *servinfo=0, *aip;  // will point to the results
	PrepareAddrInfoHints(&hints);
	getaddrinfo(ac, "", &hints, &servinfo);

	for (idx=0, aip = servinfo; aip != NULL && idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS; aip = aip->ai_next, idx++)
	{
		if (aip->ai_family == AF_INET)
		{
			struct sockaddr_in *ipv4 = (struct sockaddr_in *)aip->ai_addr;
			memcpy(&addresses[idx].address.addr4,ipv4,sizeof(sockaddr_in));
		}
		else
		{
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)aip->ai_addr;
			memcpy(&addresses[idx].address.addr4,ipv6,sizeof(sockaddr_in6));
		}

	}

	freeaddrinfo(servinfo); // free the linked-list
#else
	struct hostent *phe = gethostbyname( ac );

	if ( phe == 0 )
	{
 #if defined(_WIN32) && !defined(WINDOWS_PHONE_8)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "gethostbyname failed:Error code - %d\n%s", dwIOError, messageBuffer );

		//Free the buffer.
		LocalFree( messageBuffer );
	#endif
		return ;
	}
	for ( idx = 0; idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS; ++idx )
	{
		if (phe->h_addr_list[ idx ] == 0)
			break;

		memcpy(&addresses[idx].address.addr4.sin_addr,phe->h_addr_list[ idx ],sizeof(struct in_addr));

	}
#endif // else RAKNET_SUPPORT_IPV6==1

	while (idx < MAXIMUM_NUMBER_OF_INTERNAL_IDS)
	{
		addresses[idx]=UNASSIGNED_SYSTEM_ADDRESS;
		idx++;
	}
}

#endif


void SocketLayer::GetMyIP( SystemAddress addresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS] )
{






#if   defined(WINDOWS_STORE_RT)
	GetMyIP_WinRT(addresses);
#elif defined(_WIN32)
	GetMyIP_Win32(addresses);
#else
//	GetMyIP_Linux(addresses);
	GetMyIP_Win32(addresses);
#endif
}


/*
unsigned short SocketLayer::GetLocalPort(RakNetSocket *s)
{
	SystemAddress sa;
	GetSystemAddress(s,&sa);
	return sa.GetPort();
}
*/
unsigned short SocketLayer::GetLocalPort(__UDPSOCKET__ s)
{
	SystemAddress sa;
	GetSystemAddress(s,&sa);
	return sa.GetPort();
}
void SocketLayer::GetSystemAddress_Old ( __UDPSOCKET__ s, SystemAddress *systemAddressOut )
{
#if defined(__native_client__)
	*systemAddressOut = UNASSIGNED_SYSTEM_ADDRESS;
#else
	sockaddr_in sa;
	memset(&sa,0,sizeof(sockaddr_in));
	socklen_t len = sizeof(sa);
	if (getsockname__(s, (sockaddr*)&sa, &len)!=0)
	{
#if defined(_WIN32) && defined(_DEBUG) && !defined(WINDOWS_PHONE_8)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "getsockname failed:Error code - %d\n%s", dwIOError, messageBuffer );

		//Free the buffer.
		LocalFree( messageBuffer );
#endif
		*systemAddressOut = UNASSIGNED_SYSTEM_ADDRESS;
		return;
	}

	systemAddressOut->SetPortNetworkOrder(sa.sin_port);
	systemAddressOut->address.addr4.sin_addr.s_addr=sa.sin_addr.s_addr;
#endif
}
/*
void SocketLayer::GetSystemAddress_Old ( RakNetSocket *s, SystemAddress *systemAddressOut )
{
	return GetSystemAddress_Old(s->s, systemAddressOut);
}
*/
void SocketLayer::GetSystemAddress ( __UDPSOCKET__ s, SystemAddress *systemAddressOut )
{
#if RAKNET_SUPPORT_IPV6!=1
	GetSystemAddress_Old(s, systemAddressOut);
#else
	socklen_t slen;
	sockaddr_storage ss;
	slen = sizeof(ss);

	if (getsockname__(s, (struct sockaddr *)&ss, &slen)!=0)
	{
#if defined(_WIN32) && defined(_DEBUG)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "getsockname failed:Error code - %d\n%s", dwIOError, messageBuffer );

		//Free the buffer.
		LocalFree( messageBuffer );
#endif
		systemAddressOut->FromString(0);
		return;
	}

	if (ss.ss_family==AF_INET)
	{
		memcpy(&systemAddressOut->address.addr4,(sockaddr_in *)&ss,sizeof(sockaddr_in));
		systemAddressOut->debugPort=ntohs(systemAddressOut->address.addr4.sin_port);

		uint32_t zero = 0;		
		if (memcmp(&systemAddressOut->address.addr4.sin_addr.s_addr, &zero, sizeof(zero))==0)
			systemAddressOut->SetToLoopback(4);
		//	systemAddressOut->address.addr4.sin_port=ntohs(systemAddressOut->address.addr4.sin_port);
	}
	else
	{
		memcpy(&systemAddressOut->address.addr6,(sockaddr_in6 *)&ss,sizeof(sockaddr_in6));
		systemAddressOut->debugPort=ntohs(systemAddressOut->address.addr6.sin6_port);

		char zero[16];
		memset(zero,0,sizeof(zero));
		if (memcmp(&systemAddressOut->address.addr4.sin_addr.s_addr, &zero, sizeof(zero))==0)
			systemAddressOut->SetToLoopback(6);

		//	systemAddressOut->address.addr6.sin6_port=ntohs(systemAddressOut->address.addr6.sin6_port);
	}
#endif // #if RAKNET_SUPPORT_IPV6!=1
}
/*
void SocketLayer::GetSystemAddress ( RakNetSocket *s, SystemAddress *systemAddressOut )
{
	return GetSystemAddress(s->s, systemAddressOut);
}
*/

// void SocketLayer::SetSocketLayerOverride(SocketLayerOverride *_slo)
// {
// 	slo=_slo;
// }

bool SocketLayer::GetFirstBindableIP(char firstBindable[128], int ipProto)
{
	SystemAddress ipList[ MAXIMUM_NUMBER_OF_INTERNAL_IDS ];
	SocketLayer::GetMyIP( ipList );


	if (ipProto==AF_UNSPEC)

	{
		ipList[0].ToString(false,firstBindable);
		return true;
	}		

	// Find the first valid host address
	unsigned int l;
	for (l=0; l < MAXIMUM_NUMBER_OF_INTERNAL_IDS; l++)
	{
		if (ipList[l]==UNASSIGNED_SYSTEM_ADDRESS)
			break;
		if (ipList[l].GetIPVersion()==4 && ipProto==AF_INET)
			break;
		if (ipList[l].GetIPVersion()==6 && ipProto==AF_INET6)
			break;
	}

	if (ipList[l]==UNASSIGNED_SYSTEM_ADDRESS || l==MAXIMUM_NUMBER_OF_INTERNAL_IDS)
		return false;
// 	RAKNET_DEBUG_PRINTF("%i %i %i %i\n",
// 		((char*)(&ipList[l].address.addr4.sin_addr.s_addr))[0],
// 		((char*)(&ipList[l].address.addr4.sin_addr.s_addr))[1],
// 		((char*)(&ipList[l].address.addr4.sin_addr.s_addr))[2],
// 		((char*)(&ipList[l].address.addr4.sin_addr.s_addr))[3]
// 	);
	ipList[l].ToString(false,firstBindable);
	return true;

}


#ifdef _MSC_VER
#pragma warning( pop )
#endif
