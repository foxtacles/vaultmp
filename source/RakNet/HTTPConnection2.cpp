#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_HTTPConnection2==1 && _RAKNET_SUPPORT_TCPInterface==1

#include "HttpConnection2.h"
#include "TCPInterface.h"

using namespace RakNet;

STATIC_FACTORY_DEFINITIONS(HTTPConnection2,HTTPConnection2);

HTTPConnection2::HTTPConnection2()
{
}
HTTPConnection2::~HTTPConnection2()
{

}
bool HTTPConnection2::TransmitRequest(RakString stringToTransmit, RakString host, unsigned short port, int ipVersion, SystemAddress useAddress)
{
	Request request;
	request.host=host;
	if (useAddress!=UNASSIGNED_SYSTEM_ADDRESS)
	{
		request.hostEstimatedAddress=useAddress;
		if (IsConnected(request.hostEstimatedAddress)==false)
			return false;
	}
	else
	{
		if (request.hostEstimatedAddress.FromString(host.C_String(), '|', ipVersion)==false)
			return false;
	}
	request.hostEstimatedAddress.SetPort(port);
	request.port=port;
	request.stringToTransmit=stringToTransmit;
	request.contentLength=-1;
	request.contentOffset=0;
	request.ipVersion=ipVersion;

	if (IsConnected(request.hostEstimatedAddress))
	{
		if (sentRequests.Size()==0)
		{
			request.hostCompletedAddress=request.hostEstimatedAddress;
			SendRequest(request);
		}
		else
		{
			// Request pending, push it
			pendingRequests.Push(request, _FILE_AND_LINE_);
		}
	}
	else
	{
		pendingRequests.Push(request, _FILE_AND_LINE_);

		if (ipVersion!=6)
		{
			tcpInterface->Connect(host.C_String(), port, false, AF_INET);
		}
		else
		{
			#if RAKNET_SUPPORT_IPV6
				tcpInterface->Connect(host.C_String(), port, false, AF_INET6);
			#else
				RakAssert("HTTPConnection2::TransmitRequest needs define  RAKNET_SUPPORT_IPV6" && 0);
			#endif
		}
	}
	return true;
}
bool HTTPConnection2::GetResponse( RakString &stringTransmitted, RakString &hostTransmitted, RakString &responseReceived, SystemAddress &hostReceived, int &contentOffset )
{
	if (completedRequests.Size()>0)
	{
		responseReceived = completedRequests[0].stringReceived;
		hostReceived = completedRequests[0].hostCompletedAddress;
		stringTransmitted = completedRequests[0].stringToTransmit;
		hostTransmitted = completedRequests[0].host;
		contentOffset = completedRequests[0].contentOffset;
		completedRequests.RemoveAtIndexFast(0);
		return true;
	}
	return false;
}
PluginReceiveResult HTTPConnection2::OnReceive(Packet *packet)
{
	unsigned int i;
	for (i=0; i < sentRequests.Size(); i++)
	{
		if (sentRequests[i].hostCompletedAddress==packet->systemAddress)
		{
			sentRequests[i].stringReceived+=packet->data;

			if (sentRequests[i].contentLength==-1)
			{
				const char *length_header = strstr(sentRequests[i].stringReceived.C_String(), "Content-Length: ");
				if(length_header)
				{
					length_header += 16; // strlen("Content-Length: ");

					unsigned int clLength;
					for (clLength=0; length_header[clLength] && length_header[clLength] >= '0' && length_header[clLength] <= '9'; clLength++)
						;
					if (clLength>0 && (length_header[clLength]=='\r' || length_header[clLength]=='\n'))
					{
						sentRequests[i].contentLength = RakString::ReadIntFromSubstring(length_header, 0, clLength);
					}
				}
			}

			// If we know the content length, find \r\n\r\n
			if (sentRequests[i].contentLength != -1)
			{
				if (sentRequests[i].contentLength > 0)
				{
					const char *body_header = strstr(sentRequests[i].stringReceived.C_String(), "\r\n\r\n");
					if (body_header)
					{
						body_header += 4; // strlen("\r\n\r\n");
						size_t slen = strlen(body_header);
						RakAssert(slen <= (size_t) sentRequests[i].contentLength);
						if (slen == (size_t) sentRequests[i].contentLength)
						{
							sentRequests[i].contentOffset = body_header - sentRequests[i].stringReceived.C_String();
							completedRequests.Push(sentRequests[i], _FILE_AND_LINE_);
							sentRequests.RemoveAtIndexFast(i);

							// If there is another command waiting for this server, send it
							SendPendingRequestToConnectedSystem(packet->systemAddress);
						}
					}
				}
				else
				{
					completedRequests.Push(sentRequests[i], _FILE_AND_LINE_);
					sentRequests.RemoveAtIndexFast(i);

					// If there is another command waiting for this server, send it
					SendPendingRequestToConnectedSystem(packet->systemAddress);
				}
			}
			else
			{
				const char *firstNewlineSet = strstr(sentRequests[i].stringReceived.C_String(), "\r\n\r\n");
				if (firstNewlineSet!=0)
				{
					int offset = firstNewlineSet - sentRequests[i].stringReceived.C_String();
					if (sentRequests[i].stringReceived.C_String()[offset+4]==0)
						sentRequests[i].contentOffset=-1;
					else
						sentRequests[i].contentOffset=offset+4;
					completedRequests.Push(sentRequests[i], _FILE_AND_LINE_);
					sentRequests.RemoveAtIndexFast(i);

					// If there is another command waiting for this server, send it
					SendPendingRequestToConnectedSystem(packet->systemAddress);
				}
			}

			break;
		}
	}
	return RR_CONTINUE_PROCESSING;
}

void HTTPConnection2::OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming)
{
	(void) rakNetGUID;
	(void) isIncoming; // unknown

	SendPendingRequestToConnectedSystem(systemAddress);
}
void HTTPConnection2::SendPendingRequestToConnectedSystem(SystemAddress sa)
{
	if (sa==UNASSIGNED_SYSTEM_ADDRESS)
		return;

	unsigned int requestsSent=0;

	// Search through requests to find a match for this instance of TCPInterface and SystemAddress
	unsigned int i;
	i=0;
	while (i < pendingRequests.Size())
	{
		if (pendingRequests[i].hostEstimatedAddress==sa)
		{
			// Send this request
			pendingRequests[i].hostCompletedAddress=sa;

#if OPEN_SSL_CLIENT_SUPPORT==1
			tcpInterface->StartSSLClient(sa);
#endif
			SendRequest(pendingRequests[i]);
			pendingRequests.RemoveAtIndex(i);
			requestsSent++;
			break;
		}
		else
		{
			i++;
		}
	}

	if (requestsSent==0)
	{
		i=0;
		while (i < pendingRequests.Size())
		{
			// Just assign
			pendingRequests[i].hostCompletedAddress=sa;

			// Send

#if OPEN_SSL_CLIENT_SUPPORT==1
			tcpInterface->StartSSLClient(sa);
#endif

			SendRequest(pendingRequests[i]);
			pendingRequests.RemoveAtIndex(i);
			break;
		}
	}
}
void HTTPConnection2::RemovePendingRequest(SystemAddress sa)
{
	unsigned int i;
	i=0;
	for (i=0; i < pendingRequests.Size(); i++)
	{
		if (pendingRequests[i].hostEstimatedAddress==sa)
			pendingRequests.RemoveAtIndex(i);
		else
			i++;
	}
}
void HTTPConnection2::SendNextPendingRequest(void)
{
	// Send a pending request
	if (pendingRequests.Size()>0)
	{
		Request pendingRequest = pendingRequests.Peek();
		if (pendingRequest.ipVersion!=6)
		{
			tcpInterface->Connect(pendingRequest.host.C_String(), pendingRequest.port, false, AF_INET);
		}
		else
		{
#if RAKNET_SUPPORT_IPV6
			tcpInterface->Connect(pendingRequest.host.C_String(), pendingRequest.port, false, AF_INET6);
#else
			RakAssert("HTTPConnection2::TransmitRequest needs define  RAKNET_SUPPORT_IPV6" && 0);
#endif
		}
	}
}

void HTTPConnection2::OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason)
{
	(void) failedConnectionAttemptReason;
	if (packet->systemAddress==UNASSIGNED_SYSTEM_ADDRESS)
		return;

	RemovePendingRequest(packet->systemAddress);

	SendNextPendingRequest();
}
void HTTPConnection2::OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	(void) lostConnectionReason;
	(void) rakNetGUID;

	if (systemAddress==UNASSIGNED_SYSTEM_ADDRESS)
		return;

	// Update sent requests to completed requests
	unsigned int i;
	i=0;
	while (i < sentRequests.Size())
	{
		if (sentRequests[i].hostCompletedAddress==systemAddress)
		{
			completedRequests.Push(sentRequests[i], _FILE_AND_LINE_);
			sentRequests.RemoveAtIndexFast(i);
		}
		else
		{
			i++;
		}
	}

	SendNextPendingRequest();

	
}
bool HTTPConnection2::IsConnected(SystemAddress sa)
{
	SystemAddress remoteSystems[64];
	unsigned short numberOfSystems=64;
	tcpInterface->GetConnectionList(remoteSystems, &numberOfSystems);
	for (unsigned int i=0; i < numberOfSystems; i++)
	{
		if (remoteSystems[i]==sa)
		{
			return true;
		}
	}
	return false;
}
void HTTPConnection2::SendRequest(Request &request)
{
	tcpInterface->Send(request.stringToTransmit.C_String(), request.stringToTransmit.GetLength(), request.hostCompletedAddress, false);
	sentRequests.Push(request, _FILE_AND_LINE_);
}

#endif // #if _RAKNET_SUPPORT_HTTPConnection2==1 && _RAKNET_SUPPORT_TCPInterface==1
