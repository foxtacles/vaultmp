#include "UDPForwarder.h"

#if _RAKNET_SUPPORT_UDPForwarder==1

#include "GetTime.h"
#include "MTUSize.h"
#include "SocketLayer.h"
#include "WSAStartupSingleton.h"
#include "RakSleep.h"
#include "DS_OrderedList.h"
#include "LinuxStrings.h"

using namespace RakNet;
static const unsigned short DEFAULT_MAX_FORWARD_ENTRIES=64;

RAK_THREAD_DECLARATION(UpdateUDPForwarder);

bool operator<( const DataStructures::MLKeyRef<UDPForwarder::SrcAndDest> &inputKey, const UDPForwarder::ForwardEntry *cls )
{
	return inputKey.Get().source < cls->srcAndDest.source ||
		(inputKey.Get().source == cls->srcAndDest.source && inputKey.Get().dest < cls->srcAndDest.dest);
}
bool operator>( const DataStructures::MLKeyRef<UDPForwarder::SrcAndDest> &inputKey, const UDPForwarder::ForwardEntry *cls )
{
	return inputKey.Get().source > cls->srcAndDest.source ||
		(inputKey.Get().source == cls->srcAndDest.source && inputKey.Get().dest > cls->srcAndDest.dest);
}
bool operator==( const DataStructures::MLKeyRef<UDPForwarder::SrcAndDest> &inputKey, const UDPForwarder::ForwardEntry *cls )
{
	return inputKey.Get().source == cls->srcAndDest.source && inputKey.Get().dest == cls->srcAndDest.dest;
}


UDPForwarder::ForwardEntry::ForwardEntry() {socket=INVALID_SOCKET; timeLastDatagramForwarded=RakNet::GetTimeMS(); updatedSourcePort=false; updatedDestPort=false;}
UDPForwarder::ForwardEntry::~ForwardEntry() {
	if (socket!=INVALID_SOCKET)
		closesocket(socket);
}

UDPForwarder::UDPForwarder()
{
#ifdef _WIN32
	WSAStartupSingleton::AddRef();
#endif

	maxForwardEntries=DEFAULT_MAX_FORWARD_ENTRIES;
	isRunning=false;
	threadRunning=false;




}
UDPForwarder::~UDPForwarder()
{
	Shutdown();

#ifdef _WIN32
	WSAStartupSingleton::Deref();
#endif
}
void UDPForwarder::Startup(void)
{
	if (isRunning==true)
		return;

	isRunning=true;
	threadRunning=false;

#ifdef UDP_FORWARDER_EXECUTE_THREADED





	int errorCode;



	errorCode = RakNet::RakThread::Create(UpdateUDPForwarder, this);

	if ( errorCode != 0 )
	{
		RakAssert(0);
		return;
	}

	while (threadRunning==false)
		RakSleep(30);
#endif
}
void UDPForwarder::Shutdown(void)
{
	if (isRunning==false)
		return;

	isRunning=false;

#ifdef UDP_FORWARDER_EXECUTE_THREADED
	while (threadRunning==true)
		RakSleep(30);
#endif

	forwardList.ClearPointers(true,_FILE_AND_LINE_);






}
void UDPForwarder::Update(void)
{
#ifndef UDP_FORWARDER_EXECUTE_THREADED
	#if RAKNET_SUPPORT_IPV6!=1
	UpdateThreaded_Old();
	#else
	UpdateThreaded();
	#endif
#endif
}
void UDPForwarder::UpdateThreaded_Old(void)
{
	fd_set      readFD;
	//fd_set exceptionFD;
	FD_ZERO(&readFD);
	//	FD_ZERO(&exceptionFD);
	timeval tv;
	int selectResult;
	tv.tv_sec=0;
	tv.tv_usec=0;

	RakNet::TimeMS curTime = RakNet::GetTimeMS();

	SOCKET largestDescriptor=0;
	DataStructures::DefaultIndexType i;

	// Remove unused entries
	i=0;
	while (i < forwardList.GetSize())
	{
		if (curTime > forwardList[i]->timeLastDatagramForwarded && // Account for timestamp wrap
			curTime > forwardList[i]->timeLastDatagramForwarded+forwardList[i]->timeoutOnNoDataMS)
		{
			RakNet::OP_DELETE(forwardList[i],_FILE_AND_LINE_);
			forwardList.RemoveAtIndex(i,_FILE_AND_LINE_);
		}
		else
			i++;
	}

	if (forwardList.GetSize()==0)
		return;

	for (i=0; i < forwardList.GetSize(); i++)
	{
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		FD_SET(forwardList[i]->socket, &readFD);
		//		FD_SET(forwardList[i]->readSocket, &exceptionFD);

		if (forwardList[i]->socket > largestDescriptor)
			largestDescriptor = forwardList[i]->socket;
	}




	selectResult=(int) select((int) largestDescriptor+1, &readFD, 0, 0, &tv);


	char data[ MAXIMUM_MTU_SIZE ];
	sockaddr_in sa;
	socklen_t len2;

	if (selectResult > 0)
	{
		DataStructures::Queue<ForwardEntry*> entriesToRead;
		ForwardEntry *forwardEntry;

		for (i=0; i < forwardList.GetSize(); i++)
		{
			forwardEntry = forwardList[i];
			// I do this because I'm updating the forwardList, and don't want to lose FD_ISSET as the list is no longer in order
			if (FD_ISSET(forwardEntry->socket, &readFD))
				entriesToRead.Push(forwardEntry,_FILE_AND_LINE_);
		}

		while (entriesToRead.IsEmpty()==false)
		{
			forwardEntry=entriesToRead.Pop();

			const int flag=0;
			int receivedDataLen, len=0;
			unsigned short portnum=0;
			len2 = sizeof( sa );
			sa.sin_family = AF_INET;
			receivedDataLen = recvfrom( forwardEntry->socket, data, MAXIMUM_MTU_SIZE, flag, ( sockaddr* ) & sa, ( socklen_t* ) & len2 );

			if (receivedDataLen<0)
			{
#if defined(_WIN32) && defined(_DEBUG) 
				DWORD dwIOError = WSAGetLastError();

				if (dwIOError!=WSAECONNRESET && dwIOError!=WSAEINTR && dwIOError!=WSAETIMEDOUT)
				{
					LPVOID messageBuffer;
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
						( LPTSTR ) & messageBuffer, 0, NULL );
					// something has gone wrong here...
					RAKNET_DEBUG_PRINTF( "recvfrom failed:Error code - %d\n%s", dwIOError, messageBuffer );

					//Free the buffer.
					LocalFree( messageBuffer );
				}
#endif
				continue;
			}

			portnum = ntohs( sa.sin_port );
			if (forwardEntry->srcAndDest.source.address.addr4.sin_addr.s_addr==sa.sin_addr.s_addr && forwardEntry->updatedSourcePort==false && forwardEntry->srcAndDest.dest.GetPort()!=portnum)
			{
				forwardEntry->updatedSourcePort=true;

				if (forwardEntry->srcAndDest.source.GetPort()!=portnum)
				{
					DataStructures::DefaultIndexType index;
					SrcAndDest srcAndDest(forwardEntry->srcAndDest.dest, forwardEntry->srcAndDest.source);
					index=forwardList.GetIndexOf(srcAndDest);
					forwardList.RemoveAtIndex(index,_FILE_AND_LINE_);
					forwardEntry->srcAndDest.source.SetPort(portnum);
					forwardList.Push(forwardEntry,forwardEntry->srcAndDest,_FILE_AND_LINE_);
				}
			}

			if (forwardEntry->srcAndDest.source.address.addr4.sin_addr.s_addr==sa.sin_addr.s_addr && forwardEntry->srcAndDest.source.GetPort()==portnum)
			{
				// Forward to dest
				len=0;
				sockaddr_in saOut;
				saOut.sin_port = forwardEntry->srcAndDest.dest.GetPortNetworkOrder(); // User port
				saOut.sin_addr.s_addr = forwardEntry->srcAndDest.dest.address.addr4.sin_addr.s_addr;
				saOut.sin_family = AF_INET;
				do
				{
					len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & saOut, sizeof( saOut ) );
				}
				while ( len == 0 );

				// printf("1. Forwarding after %i ms\n", curTime-forwardEntry->timeLastDatagramForwarded);

				forwardEntry->timeLastDatagramForwarded=curTime;
			}

			if (forwardEntry->srcAndDest.dest.address.addr4.sin_addr.s_addr==sa.sin_addr.s_addr && forwardEntry->updatedDestPort==false && forwardEntry->srcAndDest.source.GetPort()!=portnum)
			{
				forwardEntry->updatedDestPort=true;

				if (forwardEntry->srcAndDest.dest.GetPort()!=portnum)
				{
					DataStructures::DefaultIndexType index;
					SrcAndDest srcAndDest(forwardEntry->srcAndDest.source, forwardEntry->srcAndDest.dest);
					index=forwardList.GetIndexOf(srcAndDest);
					forwardList.RemoveAtIndex(index,_FILE_AND_LINE_);
					forwardEntry->srcAndDest.dest.SetPort(portnum);
					forwardList.Push(forwardEntry,forwardEntry->srcAndDest,_FILE_AND_LINE_);
				}
			}

			if (forwardEntry->srcAndDest.dest.address.addr4.sin_addr.s_addr==sa.sin_addr.s_addr && forwardEntry->srcAndDest.dest.GetPort()==portnum)
			{
				// Forward to source
				len=0;
				sockaddr_in saOut;
				saOut.sin_port = forwardEntry->srcAndDest.source.GetPortNetworkOrder(); // User port
				saOut.sin_addr.s_addr = forwardEntry->srcAndDest.source.address.addr4.sin_addr.s_addr;
				saOut.sin_family = AF_INET;
				do
				{
					len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & saOut, sizeof( saOut ) );
				}
				while ( len == 0 );

				// printf("2. Forwarding after %i ms\n", curTime-forwardEntry->timeLastDatagramForwarded);

				forwardEntry->timeLastDatagramForwarded=curTime;
			}
		}
	}
}
#if RAKNET_SUPPORT_IPV6==1
void UDPForwarder::UpdateThreaded(void)
{
	fd_set      readFD;
	//fd_set exceptionFD;
	FD_ZERO(&readFD);
//	FD_ZERO(&exceptionFD);
	timeval tv;
	int selectResult;
	tv.tv_sec=0;
	tv.tv_usec=0;

	RakNet::TimeMS curTime = RakNet::GetTimeMS();

	SOCKET largestDescriptor=0;
	DataStructures::DefaultIndexType i;

	// Remove unused entries
	i=0;
	while (i < forwardList.GetSize())
	{
		if (curTime > forwardList[i]->timeLastDatagramForwarded && // Account for timestamp wrap
			curTime > forwardList[i]->timeLastDatagramForwarded+forwardList[i]->timeoutOnNoDataMS)
		{
			RakNet::OP_DELETE(forwardList[i],_FILE_AND_LINE_);
			forwardList.RemoveAtIndex(i,_FILE_AND_LINE_);
		}
		else
			i++;
	}

	if (forwardList.GetSize()==0)
		return;

	for (i=0; i < forwardList.GetSize(); i++)
	{
#ifdef _MSC_VER
#pragma warning( disable : 4127 ) // warning C4127: conditional expression is constant
#endif
		FD_SET(forwardList[i]->socket, &readFD);
//		FD_SET(forwardList[i]->readSocket, &exceptionFD);

		if (forwardList[i]->socket > largestDescriptor)
			largestDescriptor = forwardList[i]->socket;
	}

	selectResult=(int) select((int) largestDescriptor+1, &readFD, 0, 0, &tv);

	char data[ MAXIMUM_MTU_SIZE ];
	sockaddr_storage their_addr;
	sockaddr* sockAddrPtr;
	socklen_t sockLen;
	socklen_t* socketlenPtr=(socklen_t*) &sockLen;
	sockaddr_in *sockAddrIn;
	sockaddr_in6 *sockAddrIn6;
	SystemAddress receivedAddr;
	sockLen=sizeof(their_addr);
	sockAddrPtr=(sockaddr*) &their_addr;

	if (selectResult > 0)
	{
		DataStructures::Queue<ForwardEntry*> entriesToRead;
		ForwardEntry *forwardEntry;

		for (i=0; i < forwardList.GetSize(); i++)
		{
			forwardEntry = forwardList[i];
			// I do this because I'm updating the forwardList, and don't want to lose FD_ISSET as the list is no longer in order
			if (FD_ISSET(forwardEntry->socket, &readFD))
				entriesToRead.Push(forwardEntry,_FILE_AND_LINE_);
		}

		while (entriesToRead.IsEmpty()==false)
		{
			forwardEntry=entriesToRead.Pop();

			const int flag=0;
			int receivedDataLen, len=0;
			receivedDataLen = recvfrom( forwardEntry->socket, data, MAXIMUM_MTU_SIZE, flag, sockAddrPtr, socketlenPtr );

			if (receivedDataLen<0)
			{
#if defined(_WIN32) && defined(_DEBUG) 
				DWORD dwIOError = WSAGetLastError();

				if (dwIOError!=WSAECONNRESET && dwIOError!=WSAEINTR && dwIOError!=WSAETIMEDOUT)
				{
					LPVOID messageBuffer;
					FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
						( LPTSTR ) & messageBuffer, 0, NULL );
					// something has gone wrong here...
					RAKNET_DEBUG_PRINTF( "recvfrom failed:Error code - %d\n%s", dwIOError, messageBuffer );

					//Free the buffer.
					LocalFree( messageBuffer );
				}
#endif
				continue;
			}

			if (their_addr.ss_family==AF_INET)
			{
				sockAddrIn=(sockaddr_in *)&their_addr;
				sockAddrIn6=0;
				memcpy(&receivedAddr.address.addr4,sockAddrIn,sizeof(sockaddr_in));
			//	receivedAddr.address.addr4.sin_port=ntohs( sockAddrIn->sin_port );
			}
			else
			{
				sockAddrIn=0;
				sockAddrIn6=(sockaddr_in6 *)&their_addr;
				memcpy(&receivedAddr.address.addr6,sockAddrIn6,sizeof(sockaddr_in6));
			//	receivedAddr.address.addr6.sin6_port=ntohs( sockAddrIn6->sin6_port );
			}
			
			if (forwardEntry->srcAndDest.source.EqualsExcludingPort(receivedAddr) && forwardEntry->updatedSourcePort==false && forwardEntry->srcAndDest.dest.GetPort()!=receivedAddr.GetPort())
			{
				forwardEntry->updatedSourcePort=true;

				if (forwardEntry->srcAndDest.source.GetPort()!=receivedAddr.GetPort())
				{
					DataStructures::DefaultIndexType index;
					SrcAndDest srcAndDest(forwardEntry->srcAndDest.dest, forwardEntry->srcAndDest.source);
					index=forwardList.GetIndexOf(srcAndDest);
					forwardList.RemoveAtIndex(index,_FILE_AND_LINE_);
					forwardEntry->srcAndDest.source.SetPort(receivedAddr.GetPort());
					forwardList.Push(forwardEntry,forwardEntry->srcAndDest,_FILE_AND_LINE_);
				}
			}
			
			if (forwardEntry->srcAndDest.source.EqualsExcludingPort(receivedAddr) && forwardEntry->srcAndDest.source.GetPort()==receivedAddr.GetPort())
			{
				// Forward to dest
				len=0;

				if (forwardEntry->srcAndDest.dest.address.addr4.sin_family==AF_INET)
				{
					do
					{
						len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & forwardEntry->srcAndDest.dest.address.addr4, sizeof( sockaddr_in ) );
					}
					while ( len == 0 );
				}
				else
				{
					do
					{
						len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & forwardEntry->srcAndDest.dest.address.addr6, sizeof( sockaddr_in ) );
					}
					while ( len == 0 );
				}


				// printf("1. Forwarding after %i ms\n", curTime-forwardEntry->timeLastDatagramForwarded);

				forwardEntry->timeLastDatagramForwarded=curTime;
			}

			if (forwardEntry->srcAndDest.dest.EqualsExcludingPort(receivedAddr) && forwardEntry->updatedDestPort==false && forwardEntry->srcAndDest.source.GetPort()!=receivedAddr.GetPort())
			{
				forwardEntry->updatedDestPort=true;

				if (forwardEntry->srcAndDest.dest.GetPort()!=receivedAddr.GetPort())
				{
					DataStructures::DefaultIndexType index;
					SrcAndDest srcAndDest(forwardEntry->srcAndDest.source, forwardEntry->srcAndDest.dest);
					index=forwardList.GetIndexOf(srcAndDest);
					forwardList.RemoveAtIndex(index,_FILE_AND_LINE_);
					forwardEntry->srcAndDest.dest.SetPort(receivedAddr.GetPort());
					forwardList.Push(forwardEntry,forwardEntry->srcAndDest,_FILE_AND_LINE_);
				}
			}

			if (forwardEntry->srcAndDest.dest.EqualsExcludingPort(receivedAddr) && forwardEntry->srcAndDest.dest.GetPort()==receivedAddr.GetPort())
			{
				// Forward to source
				len=0;
				if (forwardEntry->srcAndDest.source.address.addr4.sin_family==AF_INET)
				{
					do
					{
						len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & forwardEntry->srcAndDest.source.address.addr4, sizeof( sockaddr_in ) );
					}
					while ( len == 0 );
				}
				else
				{
					do
					{
						len = sendto( forwardEntry->socket, data, receivedDataLen, 0, ( const sockaddr* ) & forwardEntry->srcAndDest.source.address.addr6, sizeof( sockaddr_in ) );
					}
					while ( len == 0 );
				}

				// printf("2. Forwarding after %i ms\n", curTime-forwardEntry->timeLastDatagramForwarded);

				forwardEntry->timeLastDatagramForwarded=curTime;
			}
		}
	}
}
#endif // #if RAKNET_SUPPORT_IPV6!=1
void UDPForwarder::SetMaxForwardEntries(unsigned short maxEntries)
{
	RakAssert(maxEntries>0 && maxEntries<65535/2);
	maxForwardEntries=maxEntries;
}
int UDPForwarder::GetMaxForwardEntries(void) const
{
	return maxForwardEntries;
}
int UDPForwarder::GetUsedForwardEntries(void) const
{
	return (int) forwardList.GetSize();
}
UDPForwarderResult UDPForwarder::AddForwardingEntry(SrcAndDest srcAndDest, RakNet::TimeMS timeoutOnNoDataMS, unsigned short *port, const char *forceHostAddress, short socketFamily)
{
	(void) socketFamily;

	DataStructures::DefaultIndexType insertionIndex;
	insertionIndex = forwardList.GetInsertionIndex(srcAndDest);
	if (insertionIndex!=(DataStructures::DefaultIndexType)-1)
	{
#if RAKNET_SUPPORT_IPV6!=1
		int sock_opt;
		sockaddr_in listenerSocketAddress;
		listenerSocketAddress.sin_port = 0;
		ForwardEntry *fe = RakNet::OP_NEW<UDPForwarder::ForwardEntry>(_FILE_AND_LINE_);
		fe->srcAndDest=srcAndDest;
		fe->timeoutOnNoDataMS=timeoutOnNoDataMS;
		fe->socket = socket( AF_INET, SOCK_DGRAM, 0 );

		//printf("Made socket %i\n", fe->readSocket);

		// This doubles the max throughput rate
		sock_opt=1024*256;
		setsockopt(fe->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );

		// Immediate hard close. Don't linger the readSocket, or recreating the readSocket quickly on Vista fails.
		sock_opt=0;
		setsockopt(fe->socket, SOL_SOCKET, SO_LINGER, ( char * ) & sock_opt, sizeof ( sock_opt ) );

		listenerSocketAddress.sin_family = AF_INET;

		if (forceHostAddress && forceHostAddress[0])
		{
			listenerSocketAddress.sin_addr.s_addr = inet_addr( forceHostAddress );
		}
		else
		{
			listenerSocketAddress.sin_addr.s_addr = INADDR_ANY;
		}

		int ret = bind( fe->socket, ( struct sockaddr * ) & listenerSocketAddress, sizeof( listenerSocketAddress ) );
		if (ret==-1)
		{
			RakNet::OP_DELETE(fe,_FILE_AND_LINE_);
			return UDPFORWARDER_BIND_FAILED;
		}
#else
		ForwardEntry *fe = RakNet::OP_NEW<UDPForwarder::ForwardEntry>(_FILE_AND_LINE_);
		fe->srcAndDest=srcAndDest;
		fe->timeoutOnNoDataMS=timeoutOnNoDataMS;
		fe->socket=INVALID_SOCKET;

		struct addrinfo hints;
		memset(&hints, 0, sizeof (addrinfo)); // make sure the struct is empty
		hints.ai_family = socketFamily;
		hints.ai_socktype = SOCK_DGRAM; // UDP sockets
		hints.ai_flags = AI_PASSIVE;     // fill in my IP for me
		struct addrinfo *servinfo=0, *aip;  // will point to the results
		
		
		RakAssert(forceHostAddress==0 || forceHostAddress[0]!=0);
		if (_stricmp(forceHostAddress,"UNASSIGNED_SYSTEM_ADDRESS")==0)
		{
			getaddrinfo(0, "0", &hints, &servinfo);
		}
		else
		{
			getaddrinfo(forceHostAddress, "0", &hints, &servinfo);
		}
		
		for (aip = servinfo; aip != NULL; aip = aip->ai_next)
		{
			// Open socket. The address type depends on what
			// getaddrinfo() gave us.
			fe->socket = socket(aip->ai_family, aip->ai_socktype, aip->ai_protocol);
			if (fe->socket != INVALID_SOCKET)
			{
				int ret = bind( fe->socket, aip->ai_addr, (int) aip->ai_addrlen );
				if (ret>=0)
				{
					break;
				}
				else
				{
					closesocket(fe->socket);
					fe->socket=INVALID_SOCKET;
				}
			}
		}

		if (fe->socket==INVALID_SOCKET)
			return UDPFORWARDER_BIND_FAILED;

		//printf("Made socket %i\n", fe->readSocket);

		// This doubles the max throughput rate
		int sock_opt;
		sock_opt=1024*256;
		setsockopt(fe->socket, SOL_SOCKET, SO_RCVBUF, ( char * ) & sock_opt, sizeof ( sock_opt ) );

		// Immediate hard close. Don't linger the readSocket, or recreating the readSocket quickly on Vista fails.
		sock_opt=0;
		setsockopt(fe->socket, SOL_SOCKET, SO_LINGER, ( char * ) & sock_opt, sizeof ( sock_opt ) );
#endif // #if RAKNET_SUPPORT_IPV6!=1

//		DataStructures::DefaultIndexType oldSize = forwardList.GetSize();
		forwardList.InsertAtIndex(fe,insertionIndex,_FILE_AND_LINE_);
		RakAssert(forwardList.GetIndexOf(fe->srcAndDest)!=(unsigned int) -1);
		*port = SocketLayer::GetLocalPort ( fe->socket );
		return UDPFORWARDER_SUCCESS;
	}

	return UDPFORWARDER_FORWARDING_ALREADY_EXISTS;
}
UDPForwarderResult UDPForwarder::StartForwarding(SystemAddress source, SystemAddress destination, RakNet::TimeMS timeoutOnNoDataMS, const char *forceHostAddress, unsigned short socketFamily,
								  unsigned short *forwardingPort, SOCKET *forwardingSocket)
{
	// Invalid parameters?
	if (timeoutOnNoDataMS == 0 || timeoutOnNoDataMS > UDP_FORWARDER_MAXIMUM_TIMEOUT || source==UNASSIGNED_SYSTEM_ADDRESS || destination==UNASSIGNED_SYSTEM_ADDRESS)
		return UDPFORWARDER_INVALID_PARAMETERS;

#ifdef UDP_FORWARDER_EXECUTE_THREADED
	ThreadOperation threadOperation;
	threadOperation.source=source;
	threadOperation.destination=destination;
	threadOperation.timeoutOnNoDataMS=timeoutOnNoDataMS;
	threadOperation.forceHostAddress=forceHostAddress;
	threadOperation.socketFamily=socketFamily;
	threadOperation.operation=ThreadOperation::TO_START_FORWARDING;
	threadOperationIncomingMutex.Lock();
	threadOperationIncomingQueue.Push(threadOperation, _FILE_AND_LINE_ );
	threadOperationIncomingMutex.Unlock();

	while (1)
	{
		RakSleep(0);
		threadOperationOutgoingMutex.Lock();
		if (threadOperationOutgoingQueue.Size()!=0)
		{
			threadOperation=threadOperationOutgoingQueue.Pop();
			threadOperationOutgoingMutex.Unlock();
			if (forwardingPort)
				*forwardingPort=threadOperation.forwardingPort;
			if (forwardingSocket)
				*forwardingSocket=threadOperation.forwardingSocket;
			return threadOperation.result;
		}
		threadOperationOutgoingMutex.Unlock();

	}
#else
	return StartForwardingThreaded(source, destination, timeoutOnNoDataMS, forceHostAddress, socketFamily, srcToDestPort, destToSourcePort, srcToDestSocket, destToSourceSocket);
#endif

}
UDPForwarderResult UDPForwarder::StartForwardingThreaded(SystemAddress source, SystemAddress destination, RakNet::TimeMS timeoutOnNoDataMS, const char *forceHostAddress, unsigned short socketFamily,
								  unsigned short *forwardingPort, SOCKET *forwardingSocket)
{
	SrcAndDest srcAndDest(source, destination);
	
	UDPForwarderResult result = AddForwardingEntry(srcAndDest, timeoutOnNoDataMS, forwardingPort, forceHostAddress, socketFamily);

	if (result!=UDPFORWARDER_SUCCESS)
		return result;

	if (*forwardingSocket)
	{
		DataStructures::DefaultIndexType idx;
		idx = forwardList.GetIndexOf(srcAndDest);

		*forwardingSocket=forwardList[idx]->socket;
	}

	return UDPFORWARDER_SUCCESS;
}
void UDPForwarder::StopForwarding(SystemAddress source, SystemAddress destination)
{
#ifdef UDP_FORWARDER_EXECUTE_THREADED
	ThreadOperation threadOperation;
	threadOperation.source=source;
	threadOperation.destination=destination;
	threadOperation.operation=ThreadOperation::TO_STOP_FORWARDING;
	threadOperationIncomingMutex.Lock();
	threadOperationIncomingQueue.Push(threadOperation, _FILE_AND_LINE_ );
	threadOperationIncomingMutex.Unlock();
#else
	StopForwardingThreaded(source, destination);
#endif
}
void UDPForwarder::StopForwardingThreaded(SystemAddress source, SystemAddress destination)
{
	SrcAndDest srcAndDest(destination,source);

	DataStructures::DefaultIndexType idx = forwardList.GetIndexOf(srcAndDest);
	if (idx!=(DataStructures::DefaultIndexType)-1)
	{
		RakNet::OP_DELETE(forwardList[idx],_FILE_AND_LINE_);
		forwardList.RemoveAtIndex(idx,_FILE_AND_LINE_);
	}
}
#ifdef UDP_FORWARDER_EXECUTE_THREADED
RAK_THREAD_DECLARATION(UpdateUDPForwarder)
{
	UDPForwarder * udpForwarder = ( UDPForwarder * ) arguments;
	udpForwarder->threadRunning=true;
	UDPForwarder::ThreadOperation threadOperation;
	while (udpForwarder->isRunning)
	{
		udpForwarder->threadOperationIncomingMutex.Lock();
		while (udpForwarder->threadOperationIncomingQueue.Size())
		{
			threadOperation=udpForwarder->threadOperationIncomingQueue.Pop();
			udpForwarder->threadOperationIncomingMutex.Unlock();
			if (threadOperation.operation==UDPForwarder::ThreadOperation::TO_START_FORWARDING)
			{
				threadOperation.result=udpForwarder->StartForwardingThreaded(threadOperation.source, threadOperation.destination, threadOperation.timeoutOnNoDataMS,
					threadOperation.forceHostAddress, threadOperation.socketFamily, &threadOperation.forwardingPort, &threadOperation.forwardingSocket);
				udpForwarder->threadOperationOutgoingMutex.Lock();
				udpForwarder->threadOperationOutgoingQueue.Push(threadOperation, _FILE_AND_LINE_ );
				udpForwarder->threadOperationOutgoingMutex.Unlock();
			}
			else
			{
				udpForwarder->StopForwardingThreaded(threadOperation.source, threadOperation.destination);
			}


			udpForwarder->threadOperationIncomingMutex.Lock();
		}
		udpForwarder->threadOperationIncomingMutex.Unlock();

#if RAKNET_SUPPORT_IPV6!=1
		udpForwarder->UpdateThreaded_Old();
#else
		udpForwarder->UpdateThreaded();
#endif


		// 12/1/2010 Do not change from 0
		// See http://www.jenkinssoftware.com/forum/index.php?topic=4033.0;topicseen
		// Avoid 100% reported CPU usage
		RakSleep(0);
	}
	udpForwarder->threadRunning=false;
	return 0;
}
#endif

#endif // #if _RAKNET_SUPPORT_FileOperations==1
