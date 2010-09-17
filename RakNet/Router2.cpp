#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_Router2==1

#include "Router2.h"
#include "RakPeerInterface.h"
#include "BitStream.h"
#include "RakNetTime.h"
#include "GetTime.h"
#include "DS_OrderedList.h"
#include "SocketLayer.h"
#include "FormatString.h"

using namespace RakNet;

/*
Algorithm:

1. Sender calls ConnectInternal(). A ConnnectRequest structure is allocated and stored in the connectionRequests list, containing a list of every system we are connected to. ID_ROUTER_2_QUERY_FORWARDING is sent to all connected systems.

2. Upon the router getting ID_ROUTER_2_QUERY_FORWARDING, ID_ROUTER_2_REPLY_FORWARDING is sent to the sender indicating if that router is connected to the endpoint, along with the ping from the router to the endpoint.

3. Upon the sender getting ID_ROUTER_2_REPLY_FORWARDING, the connection request structure is looked up in Router2::UpdateForwarding. The ping is stored in that structure. Once all systems have replied, the system continues to the next state. If every system in step 1 has been exhausted, and routing has occured at least once, then ID_CONNECTION_LOST is returned. If every system in step 1 has been exhausted and routing has never occured, then ID_ROUTER_2_FORWARDING_NO_PATH is returned. Otherwise, the router with the lowest ping is chosen, and RequestForwarding() is called with that system, which sends ID_ROUTER_2_REQUEST_FORWARDING to the router.

4. When the router gets ID_ROUTER_2_REQUEST_FORWARDING, a MiniPunchRequest structure is allocated and stored in the miniPunchesInProgress list. The function SendOOBMessages() sends ID_ROUTER_2_REPLY_TO_SENDER_PORT from the routing sockets to both the sender and endpoint. It also sends ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT through the regular RakNet connection.

5. The sender and endpoint should get ID_ROUTER_2_REPLY_TO_SENDER_PORT and/or ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT depending on what type of router they have. If ID_ROUTER_2_REPLY_TO_SENDER_PORT arrives, then this will reply back to the routing socket directly. If ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT arrives, then the reply port is modified to be the port specified by the router system. In both cases, ID_ROUTER_2_MINI_PUNCH_REPLY is sent. As the router has already setup the forwarding, ID_ROUTER_2_MINI_PUNCH_REPLY will actually arrive to the endpoint from the sender, and from the sender to the endpoint.

6. When ID_ROUTER_2_MINI_PUNCH_REPLY arrives, ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE will be sent to the router. This is to tell the router that the forwarding has succeeded.

7. When ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE arrives on the router, the router will find the two systems in the miniPunchesInProgress list, which was added in step 4 (See OnMiniPunchReplyBounce()). gotReplyFromSource or gotReplyFromEndpoint will be set to true, depending on the sender. When both gotReplyFromSource and gotReplyFromEndpoint have replied, then ID_ROUTER_2_REROUTE is sent to the endpoint, and ID_ROUTER_2_FORWARDING_ESTABLISHED is sent to the sender.

8. When the endpoint gets ID_ROUTER_2_REROUTE, the system address is updated for the guid of the sender using RakPeer::ChangeSystemAddress(). This happens in OnReroute().

9. When the sender gets ID_ROUTER_2_FORWARDING_ESTABLISHED, then in OnForwardingSuccess() the endpoint is removed from the connectionRequest list and moved to the forwardedConnectionList.

10. In OnClosedConnection(), for the sender, if the closed connection is the endpoint, then the endpoint is removed from the forwardedConnectionList (this is a graceful disconnect). If the connection was instead lost to the router, then ConnectInternal() gets called, which goes back to step 1. If instead this was a connection requset in progress, then UpdateForwarding() gets called, which goes back to step 3.

11. When the user connects the endpoint and sender, then the sender will get ID_CONNECTION_REQUEST_ACCEPTED. The sender will look up the endpoint in the forwardedConnectionList, and send ID_ROUTER_2_INCREASE_TIMEOUT to the endpoint. This message will call SetTimeoutTime() on the endpoint, so that if the router disconnects, enough time is available for the reroute to complete.
*/

void Router2DebugInterface::ShowFailure(const char *message)
{
	printf(message);
}
void Router2DebugInterface::ShowDiagnostic(const char *message)
{
	printf(message);
}

enum Router2MessageIdentifiers
{
	ID_ROUTER_2_QUERY_FORWARDING,
	ID_ROUTER_2_REPLY_FORWARDING,
	ID_ROUTER_2_REQUEST_FORWARDING,
	ID_ROUTER_2_INCREASE_TIMEOUT,
};
Router2::ConnnectRequest::ConnnectRequest()
{

}
Router2::ConnnectRequest::~ConnnectRequest()
{

}

STATIC_FACTORY_DEFINITIONS(Router2,Router2);

Router2::Router2()
{
	udpForwarder=0;
	maximumForwardingRequests=0;
	debugInterface=0;
}
Router2::~Router2()
{
	ClearAll();

	if (udpForwarder)
	{
		udpForwarder->Shutdown();
		RakNet::OP_DELETE(udpForwarder,_FILE_AND_LINE_);
	}
}
void Router2::ClearMinipunches(void)
{
	miniPunchesInProgress.Clear(false,_FILE_AND_LINE_);
}
void Router2::ClearConnectionRequests(void)
{
	for (unsigned int i=0; i < connectionRequests.Size(); i++)
	{
		RakNet::OP_DELETE(connectionRequests[i],_FILE_AND_LINE_);
	}
	connectionRequests.Clear(false,_FILE_AND_LINE_);
}
bool Router2::ConnectInternal(RakNetGUID endpointGuid, bool returnConnectionLostOnFailure)
{
	int largestPing = GetLargestPingAmongConnectedSystems();
	if (largestPing<0)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));

		// Not connected to anyone
		return false;
	}

	// ALready in progress?
	if (GetConnectionRequestIndex(endpointGuid)!=(unsigned int)-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));

		return false;
	}

	// StoreRequest(endpointGuid, Largest(ping*2), systemsSentTo). Set state REQUEST_STATE_QUERY_FORWARDING
	Router2::ConnnectRequest *cr = RakNet::OP_NEW<Router2::ConnnectRequest>(_FILE_AND_LINE_);
	DataStructures::List<SystemAddress> addresses;
	DataStructures::List<RakNetGUID> guids;
	rakPeerInterface->GetSystemList(addresses, guids);
	if (guids.Size()==0)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));

		return false;
	}
	cr->requestState=R2RS_REQUEST_STATE_QUERY_FORWARDING;
	cr->pingTimeout=RakNet::GetTimeMS()+largestPing*2+1000;
	cr->endpointGuid=endpointGuid;
	cr->returnConnectionLostOnFailure=returnConnectionLostOnFailure;
	for (unsigned int i=0; i < guids.Size(); i++)
	{
		ConnectionRequestSystem crs;
		if (guids[i]!=endpointGuid)
		{
			crs.guid=guids[i];
			crs.pingToEndpoint=-1;
			cr->connectionRequestSystems.Push(crs,_FILE_AND_LINE_);

			// Broadcast(ID_ROUTER_2_QUERY_FORWARDING, endpointGuid);
			RakNet::BitStream bsOut;
			bsOut.Write((MessageID)ID_ROUTER_2_INTERNAL);
			bsOut.Write((unsigned char) ID_ROUTER_2_QUERY_FORWARDING);
			bsOut.Write(endpointGuid);
			rakPeerInterface->Send(&bsOut,MEDIUM_PRIORITY,RELIABLE_ORDERED,0,crs.guid,false);
		}
	}
	connectionRequests.Push(cr,_FILE_AND_LINE_);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Broadcasting ID_ROUTER_2_QUERY_FORWARDING at %s:%i\n", _FILE_AND_LINE_));
	}

	return true;
}
void Router2::EstablishRouting(RakNetGUID endpointGuid)
{
	ConnectionState cs = rakPeerInterface->GetConnectionState(endpointGuid);
	if (cs!=IS_DISCONNECTED && cs!=IS_NOT_CONNECTED)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i (already connected)\n", _FILE_AND_LINE_));
		return;
	}
	if (cs==IS_LOOPBACK)
	{
		printf("Router2 failed at %s:%i (loopback)\n", _FILE_AND_LINE_);
		return;
	}

	ConnectInternal(endpointGuid,false);
}
void Router2::SetMaximumForwardingRequests(int max)
{
	if (max>0 && maximumForwardingRequests<=0)
	{
		udpForwarder = RakNet::OP_NEW<UDPForwarder>(_FILE_AND_LINE_);
		udpForwarder->Startup();
	}
	else if (max<=0 && maximumForwardingRequests>0)
	{
		udpForwarder->Shutdown();
		RakNet::OP_DELETE(udpForwarder,_FILE_AND_LINE_);
		udpForwarder=0;
	}

	maximumForwardingRequests=max;
}
PluginReceiveResult Router2::OnReceive(Packet *packet)
{
	SystemAddress sa;
	RakNet::BitStream bs(packet->data,packet->length,false);
	if (packet->data[0]==ID_ROUTER_2_INTERNAL)
	{
		switch (packet->data[1])
		{
		case ID_ROUTER_2_QUERY_FORWARDING:
			{
				OnQueryForwarding(packet);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			}
		case ID_ROUTER_2_REPLY_FORWARDING:
			{
				OnQueryForwardingReply(packet);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			}
		case ID_ROUTER_2_REQUEST_FORWARDING:
			{
				OnRequestForwarding(packet);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			}
		case ID_ROUTER_2_INCREASE_TIMEOUT:
			{
				/// The routed system wants more time to stay alive on no communication, in case the router drops or crashes
				rakPeerInterface->SetTimeoutTime(rakPeerInterface->GetTimeoutTime(packet->systemAddress)+10000, packet->systemAddress);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			}
		}
	}
	else if (packet->data[0]==ID_OUT_OF_BAND_INTERNAL && packet->length>=2)
	{
		switch (packet->data[1])
		{
			case ID_ROUTER_2_REPLY_TO_SENDER_PORT:
				{
					RakNet::BitStream bsOut;
					bsOut.Write(packet->guid);
					SendOOBFromRakNetPort(ID_ROUTER_2_MINI_PUNCH_REPLY, &bsOut, packet->systemAddress);

					if (debugInterface)
					{
						char buff[512];
						debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_REPLY_TO_SENDER_PORT, replying with ID_ROUTER_2_MINI_PUNCH_REPLY at %s:%i\n", _FILE_AND_LINE_));
					}

					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}
			case ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT:
				{
					RakNet::BitStream bsOut;
					bsOut.Write(packet->guid);
					bs.IgnoreBytes(2);
					sa.binaryAddress=packet->systemAddress.binaryAddress;
					bs.Read(sa.port);
					RakAssert(sa.port!=0);
					SendOOBFromRakNetPort(ID_ROUTER_2_MINI_PUNCH_REPLY, &bsOut, sa);

					if (debugInterface)
					{
						char buff[512];
						debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_REPLY_TO_SENDER_PORT, replying with ID_ROUTER_2_MINI_PUNCH_REPLY at %s:%i\n", _FILE_AND_LINE_));
					}

					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}
			case ID_ROUTER_2_MINI_PUNCH_REPLY:
				OnMiniPunchReply(packet);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			case ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE:
				OnMiniPunchReplyBounce(packet);
				return RR_STOP_PROCESSING_AND_DEALLOCATE;
			case ID_ROUTER_2_REROUTE:
				{
					OnReroute(packet);
					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}
		}
	}
	else if (packet->data[0]==ID_ROUTER_2_FORWARDING_ESTABLISHED)
	{
//		printf("Got ID_ROUTER_2_FORWARDING_ESTABLISHED\n");
		if (OnForwardingSuccess(packet)==false)
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}
	else if (packet->data[0]==ID_CONNECTION_REQUEST_ACCEPTED)
	{
		unsigned int forwardingIndex;
		for (forwardingIndex=0; forwardingIndex < forwardedConnectionList.Size(); forwardingIndex++)
		{
			if (forwardedConnectionList[forwardingIndex].endpointGuid==packet->guid)
				break;
		}

		if (forwardingIndex<forwardedConnectionList.Size())
		{
			// We connected to this system through a forwarding system
			// Have the endpoint take longer to drop us, in case the intermediary system drops
			RakNet::BitStream bsOut;
			bsOut.Write((MessageID)ID_ROUTER_2_INTERNAL);
			bsOut.Write((unsigned char) ID_ROUTER_2_INCREASE_TIMEOUT);
			rakPeerInterface->Send(&bsOut,HIGH_PRIORITY,RELIABLE,0,packet->guid,false);

			if (debugInterface)
			{
				char buff[512];
				debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_CONNECTION_REQUEST_ACCEPTED, sending ID_ROUTER_2_INCREASE_TIMEOUT at %s:%i\n", _FILE_AND_LINE_));
			}

			// Also take longer ourselves
			rakPeerInterface->SetTimeoutTime(rakPeerInterface->GetTimeoutTime(packet->systemAddress)+10000, packet->systemAddress);
		}
	}

	return RR_CONTINUE_PROCESSING;
}
void Router2::Update(void)
{
	RakNet::TimeMS curTime = RakNet::GetTimeMS();
	unsigned int connectionRequestIndex=0;
	while (connectionRequestIndex < connectionRequests.Size())
	{
		ConnnectRequest* connectionRequest = connectionRequests[connectionRequestIndex];
		// pingTimeout is only used with R2RS_REQUEST_STATE_QUERY_FORWARDING
		if (connectionRequest->requestState==R2RS_REQUEST_STATE_QUERY_FORWARDING &&
			connectionRequest->pingTimeout < curTime)
		{
			bool anyRemoved=false;
			unsigned int connectionRequestGuidIndex=0;
			while (connectionRequestGuidIndex < connectionRequest->connectionRequestSystems.Size())
			{
				if (connectionRequest->connectionRequestSystems[connectionRequestGuidIndex].pingToEndpoint<0)
				{
					anyRemoved=true;
					connectionRequest->connectionRequestSystems.RemoveAtIndexFast(connectionRequestGuidIndex);
				}
				else
				{
					connectionRequestGuidIndex++;
				}
			}

			if (anyRemoved)
			{
				if (UpdateForwarding(connectionRequestIndex)==false)
				{
					RemoveConnectionRequest(connectionRequestIndex);
				}
				else
				{
					connectionRequestIndex++;
				}
			}
			else
			{
				connectionRequestIndex++;
			}
		}
		else
		{
			connectionRequestIndex++;
		}
	}

	unsigned int i=0;
	while (i < miniPunchesInProgress.Size())
	{
		if (miniPunchesInProgress[i].timeout<curTime)
		{
			SendFailureOnCannotForward(miniPunchesInProgress[i].sourceGuid, miniPunchesInProgress[i].endpointGuid);
			miniPunchesInProgress.RemoveAtIndexFast(i);
		}
		else if (curTime>miniPunchesInProgress[i].nextAction)
		{
			miniPunchesInProgress[i].nextAction=curTime+100;
			SendOOBMessages(&miniPunchesInProgress[i]);
		}
		else
			i++;
	}

}
void Router2::OnClosedConnection(SystemAddress systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	(void) lostConnectionReason;
	(void) systemAddress;


	unsigned int forwardedConnectionIndex=0;
	while (forwardedConnectionIndex<forwardedConnectionList.Size())
	{
		if (forwardedConnectionList[forwardedConnectionIndex].endpointGuid==rakNetGUID)
		{
			if (debugInterface)
			{
				char buff[512];
				debugInterface->ShowDiagnostic(FormatStringTS(buff,"Closed connection, removing forwarding from list at %s:%i\n", _FILE_AND_LINE_));
			}

			// No longer need forwarding
			forwardedConnectionList.RemoveAtIndexFast(forwardedConnectionIndex);
		}
		else if (forwardedConnectionList[forwardedConnectionIndex].intermediaryGuid==rakNetGUID)
		{
			// Lost connection to intermediary. Restart process to connect to endpoint. If failed, push ID_CONNECTION_LOST
			ConnectInternal(forwardedConnectionList[forwardedConnectionIndex].endpointGuid, true);

			forwardedConnectionIndex++;

			if (debugInterface)
			{
				char buff[512];
				debugInterface->ShowDiagnostic(FormatStringTS(buff,"Closed connection, restarting forwarding at %s:%i\n", _FILE_AND_LINE_));
			}

			// This should not be removed - the connection is still forwarded, but perhaps through another system
//			forwardedConnectionList.RemoveAtIndexFast(forwardedConnectionIndex);
		}
		else
			forwardedConnectionIndex++;
	}

	unsigned int connectionRequestIndex=0;
	while (connectionRequestIndex < connectionRequests.Size())
	{
		unsigned int connectionRequestGuidIndex = connectionRequests[connectionRequestIndex]->GetGuidIndex(rakNetGUID);
		if (connectionRequestGuidIndex!=(unsigned int)-1)
		{
			connectionRequests[connectionRequestIndex]->connectionRequestSystems.RemoveAtIndexFast(connectionRequestGuidIndex);
			if (UpdateForwarding(connectionRequestIndex)==false)
			{
				if (debugInterface)
				{
					char buff[512];
					debugInterface->ShowDiagnostic(FormatStringTS(buff,"Aborted connection, aborted forwarding at %s:%i\n", _FILE_AND_LINE_));
				}

				RemoveConnectionRequest(connectionRequestIndex);
			}
			else
			{
				if (debugInterface)
				{
					char buff[512];
					debugInterface->ShowDiagnostic(FormatStringTS(buff,"Aborted connection, restarting forwarding at %s:%i\n", _FILE_AND_LINE_));
				}

				connectionRequestIndex++;
			}
		}
		else
		{
			connectionRequestIndex++;
		}
	}


	unsigned int i=0;
	while (i < miniPunchesInProgress.Size())
	{
		if (miniPunchesInProgress[i].sourceGuid==rakNetGUID || miniPunchesInProgress[i].endpointGuid==rakNetGUID)
		{
			if (miniPunchesInProgress[i].sourceGuid!=rakNetGUID)
			{
				SendFailureOnCannotForward(miniPunchesInProgress[i].sourceGuid, miniPunchesInProgress[i].endpointGuid);
			}
			miniPunchesInProgress.RemoveAtIndexFast(i);
		}
		else
			i++;
	}
}
void Router2::OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason)
{
	(void) failedConnectionAttemptReason;
	(void) packet;

	unsigned int forwardedConnectionIndex=0;
	while (forwardedConnectionIndex<forwardedConnectionList.Size())
	{
		if (forwardedConnectionList[forwardedConnectionIndex].intermediaryAddress==packet->systemAddress)
		{
			if (debugInterface)
			{
				char buff[512];
				debugInterface->ShowDiagnostic(FormatStringTS(buff,"Failed connection attempt to forwarded system at %s:%i\n", _FILE_AND_LINE_));
			}

			packet->guid=forwardedConnectionList[forwardedConnectionIndex].endpointGuid;
			forwardedConnectionList.RemoveAtIndexFast(forwardedConnectionIndex);
		}
		else
			forwardedConnectionIndex++;
	}
}
void Router2::OnRakPeerShutdown(void)
{
	ClearAll();
}
bool Router2::UpdateForwarding(unsigned int connectionRequestIndex)
{
	ConnnectRequest* connectionRequest = connectionRequests[connectionRequestIndex];
	if (connectionRequest->connectionRequestSystems.Size()==0)
	{
	//	printf("Router2 failed at %s:%i\n", _FILE_AND_LINE_);
		if (connectionRequest->returnConnectionLostOnFailure)
			ReturnToUser(ID_CONNECTION_LOST, connectionRequest->endpointGuid, UNASSIGNED_SYSTEM_ADDRESS);
		else
			ReturnToUser(ID_ROUTER_2_FORWARDING_NO_PATH, connectionRequest->endpointGuid, UNASSIGNED_SYSTEM_ADDRESS);

		if (debugInterface)
		{
			char buff[512];
			debugInterface->ShowDiagnostic(FormatStringTS(buff,"Forwarding failed, no remaining systems at %s:%i\n", _FILE_AND_LINE_));
		}

		for (unsigned int forwardedConnectionIndex=0; forwardedConnectionIndex < forwardedConnectionList.Size(); forwardedConnectionIndex++)
		{
			if (forwardedConnectionList[forwardedConnectionIndex].endpointGuid==connectionRequest->endpointGuid)
			{
				forwardedConnectionList.RemoveAtIndexFast(forwardedConnectionIndex);
				break;
			}
		}

		return false;
	}

	if (connectionRequest->requestState==R2RS_REQUEST_STATE_QUERY_FORWARDING)
	{
		for (unsigned int i=0; i < connectionRequest->connectionRequestSystems.Size(); i++)
		{
			if (connectionRequest->connectionRequestSystems[i].pingToEndpoint<0)
				return true;
		}

		RequestForwarding(connectionRequestIndex);
	}
// 	else if (connectionRequest->requestState==REQUEST_STATE_REQUEST_FORWARDING)
// 	{
// 		RequestForwarding(connectionRequestIndex);
// 	}

	return true;
}
void Router2::RemoveConnectionRequest(unsigned int connectionRequestIndex)
{
	RakNet::OP_DELETE(connectionRequests[connectionRequestIndex],_FILE_AND_LINE_);
	connectionRequests.RemoveAtIndexFast(connectionRequestIndex);
}
int ConnectionRequestSystemComp( const Router2::ConnectionRequestSystem & key, const Router2::ConnectionRequestSystem &data )
{
	if (key.pingToEndpoint * (key.usedForwardingEntries+1) < data.pingToEndpoint * (data.usedForwardingEntries+1))
		return -1;
	if (key.pingToEndpoint * (key.usedForwardingEntries+1) == data.pingToEndpoint * (data.usedForwardingEntries+1))
		return 1;
	if (key.guid < data.guid)
		return -1;
	if (key.guid > data.guid)
		return -1;
	return 0;
}
void Router2::RequestForwarding(unsigned int connectionRequestIndex)
{
	ConnnectRequest* connectionRequest = connectionRequests[connectionRequestIndex];
	RakAssert(connectionRequest->requestState==R2RS_REQUEST_STATE_QUERY_FORWARDING);
	connectionRequest->requestState=REQUEST_STATE_REQUEST_FORWARDING;

	if (connectionRequest->GetGuidIndex(connectionRequest->lastRequestedForwardingSystem)!=(unsigned int)-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		return;
	}

	// Prioritize systems to request forwarding
	DataStructures::OrderedList<ConnectionRequestSystem, ConnectionRequestSystem, ConnectionRequestSystemComp> commandList;
	unsigned int connectionRequestGuidIndex;
	for (connectionRequestGuidIndex=0; connectionRequestGuidIndex < connectionRequest->connectionRequestSystems.Size(); connectionRequestGuidIndex++)
	{
		RakAssert(connectionRequest->connectionRequestSystems[connectionRequestGuidIndex].pingToEndpoint>=0);
		commandList.Insert(connectionRequest->connectionRequestSystems[connectionRequestGuidIndex],
			connectionRequest->connectionRequestSystems[connectionRequestGuidIndex],
			true,
			_FILE_AND_LINE_);
	}

	connectionRequest->lastRequestedForwardingSystem=commandList[0].guid;

	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_ROUTER_2_INTERNAL);
	bsOut.Write((unsigned char) ID_ROUTER_2_REQUEST_FORWARDING);
	bsOut.Write(connectionRequest->endpointGuid);
	rakPeerInterface->Send(&bsOut,MEDIUM_PRIORITY,RELIABLE_ORDERED,0,connectionRequest->lastRequestedForwardingSystem,false);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Sending ID_ROUTER_2_REQUEST_FORWARDING at %s:%i\n", _FILE_AND_LINE_));
	}
}
void Router2::SendFailureOnCannotForward(RakNetGUID sourceGuid, RakNetGUID endpointGuid)
{
	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_ROUTER_2_INTERNAL);
	bsOut.Write((unsigned char) ID_ROUTER_2_REPLY_FORWARDING);
	bsOut.Write(endpointGuid);
	bsOut.Write(false);
	rakPeerInterface->Send(&bsOut,MEDIUM_PRIORITY,RELIABLE_ORDERED,0,sourceGuid,false);
}
int Router2::ReturnFailureOnCannotForward(RakNetGUID sourceGuid, RakNetGUID endpointGuid)
{
	// If the number of systems we are currently forwarding>=maxForwarding, return ID_ROUTER_2_REPLY_FORWARDING,endpointGuid,false
	if (udpForwarder==0 || udpForwarder->GetUsedForwardEntries()/2>maximumForwardingRequests)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		SendFailureOnCannotForward(sourceGuid,endpointGuid);
		return -1;
	}

	int pingToEndpoint;
	pingToEndpoint = rakPeerInterface->GetAveragePing(endpointGuid);
	if (pingToEndpoint==-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		SendFailureOnCannotForward(sourceGuid,endpointGuid);
		return -1;
	}
	return pingToEndpoint;
}
void Router2::OnQueryForwarding(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID) + sizeof(unsigned char));
	RakNetGUID endpointGuid;
	// Read endpointGuid
	bs.Read(endpointGuid);

	int pingToEndpoint = ReturnFailureOnCannotForward(packet->guid, endpointGuid);
	if (pingToEndpoint==-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		return;
	}

	// If we are connected to endpointGuid, reply ID_ROUTER_2_REPLY_FORWARDING,endpointGuid,true,ping,numCurrentlyForwarding
	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_ROUTER_2_INTERNAL);
	bsOut.Write((unsigned char) ID_ROUTER_2_REPLY_FORWARDING);
	bsOut.Write(endpointGuid);
	bsOut.Write(true);
	bsOut.Write((unsigned short) pingToEndpoint);
	bsOut.Write((unsigned short) udpForwarder->GetUsedForwardEntries()/2);
	rakPeerInterface->Send(&bsOut,MEDIUM_PRIORITY,RELIABLE_ORDERED,0,packet->guid,false);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Sending ID_ROUTER_2_REPLY_FORWARDING at %s:%i\n", _FILE_AND_LINE_));
	}
}
void Router2::OnQueryForwardingReply(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID) + sizeof(unsigned char));
	RakNetGUID endpointGuid;
	bs.Read(endpointGuid);
	// Find endpointGuid among stored requests

	unsigned int connectionRequestIndex = GetConnectionRequestIndex(endpointGuid);
	if (connectionRequestIndex==(unsigned int)-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		return;
	}
	unsigned int connectionRequestGuidIndex = connectionRequests[connectionRequestIndex]->GetGuidIndex(packet->guid);
	if (connectionRequestGuidIndex==(unsigned int)-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		return;
	}

	bool canForward;
	bs.Read(canForward);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_REPLY_FORWARDING, canForward=%i at %s:%i\n", canForward, _FILE_AND_LINE_));
	}

	if (canForward)
	{
		unsigned short pingToEndpoint;
		unsigned short usedEntries;
		bs.Read(pingToEndpoint);
		bs.Read(usedEntries);
		connectionRequests[connectionRequestIndex]->connectionRequestSystems[connectionRequestGuidIndex].usedForwardingEntries=usedEntries;
		connectionRequests[connectionRequestIndex]->connectionRequestSystems[connectionRequestGuidIndex].pingToEndpoint=rakPeerInterface->GetAveragePing(packet->guid)+pingToEndpoint;
	}
	else
	{
		connectionRequests[connectionRequestIndex]->connectionRequestSystems.RemoveAtIndex(connectionRequestGuidIndex);
	}

	if (UpdateForwarding(connectionRequestIndex)==false)
	{
		RemoveConnectionRequest(connectionRequestIndex);
	}
}
void Router2::SendForwardingSuccess(RakNetGUID sourceGuid, RakNetGUID endpointGuid, unsigned short sourceToDstPort)
{
	RakNet::BitStream bsOut;
	bsOut.Write((MessageID) ID_ROUTER_2_FORWARDING_ESTABLISHED);
	bsOut.Write(endpointGuid);
	bsOut.Write(sourceToDstPort);
	rakPeerInterface->Send(&bsOut,MEDIUM_PRIORITY,RELIABLE_ORDERED,0,sourceGuid,false);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Sending ID_ROUTER_2_FORWARDING_ESTABLISHED at %s:%i\n", _FILE_AND_LINE_));
	}
}
void Router2::SendOOBFromRakNetPort(OutOfBandIdentifiers oob, BitStream *extraData, SystemAddress sa)
{
	RakNet::BitStream oobBs;
	oobBs.Write((unsigned char)oob);
	if (extraData)
	{
		extraData->ResetReadPointer();
		oobBs.Write(*extraData);
	}
	char ipAddressString[32];
	sa.ToString(false, ipAddressString);
	rakPeerInterface->SendOutOfBand((const char*) ipAddressString,sa.port,(const char*) oobBs.GetData(),oobBs.GetNumberOfBytesUsed());
}
void Router2::SendOOBFromSpecifiedSocket(OutOfBandIdentifiers oob, SystemAddress sa, SOCKET socket)
{
	RakNet::BitStream bs;
	rakPeerInterface->WriteOutOfBandHeader(&bs);
	bs.Write((unsigned char) oob);
	SocketLayer::Instance()->SendTo_PC( socket, (const char*) bs.GetData(), bs.GetNumberOfBytesUsed(), sa.binaryAddress, sa.port );
}
void Router2::SendOOBMessages(Router2::MiniPunchRequest *mpr)
{
	// Mini NAT punch
	// Send from srcToDestPort to packet->systemAddress (source). If the message arrives, the remote system should reply.
	SendOOBFromSpecifiedSocket(ID_ROUTER_2_REPLY_TO_SENDER_PORT, mpr->sourceAddress, mpr->destToSourceSocket);

	// Send from destToSourcePort to endpointSystemAddress (destination). If the message arrives, the remote system should reply.
	SendOOBFromSpecifiedSocket(ID_ROUTER_2_REPLY_TO_SENDER_PORT, mpr->endpointAddress, mpr->srcToDestSocket);

	// Tell source to send to srcToDestPort
	RakNet::BitStream extraData;
	extraData.Write(mpr->srcToDestPort);
	RakAssert(mpr->srcToDestPort!=0);
	SendOOBFromRakNetPort(ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT, &extraData, mpr->sourceAddress);

	// Tell destination to send to destToSourcePort
	extraData.Reset();
	extraData.Write(mpr->destToSourcePort);
	RakAssert(mpr->destToSourcePort);
	SendOOBFromRakNetPort(ID_ROUTER_2_REPLY_TO_SPECIFIED_PORT, &extraData, mpr->endpointAddress);
}
void Router2::OnReroute(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID) + sizeof(unsigned char));
	RakNetGUID sourceGuid;
	bs.Read(sourceGuid);

//	char address[64];
	char ip[64];
//	sourceGuid.ToString(address);
	packet->systemAddress.ToString(true,ip);
//	printf("Rerouting source guid %s to address %s\n", address, ip);

	rakPeerInterface->ChangeSystemAddress(sourceGuid,packet->systemAddress);

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Calling RakPeer::ChangeSystemAddress at %s:%i\n", _FILE_AND_LINE_));
	}
}
void Router2::OnRequestForwarding(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID) + sizeof(unsigned char));
	RakNetGUID endpointGuid;
	bs.Read(endpointGuid);

	int pingToEndpoint = ReturnFailureOnCannotForward(packet->guid, endpointGuid);
	if (pingToEndpoint==-1)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		return;
	}

	unsigned short srcToDestPort;
	unsigned short destToSourcePort;
	SOCKET srcToDestSocket;
	SOCKET destToSourceSocket;
	SystemAddress endpointSystemAddress = rakPeerInterface->GetSystemAddressFromGuid(endpointGuid);
	UDPForwarderResult result = udpForwarder->StartForwarding(
		packet->systemAddress, endpointSystemAddress, 10000, 0,
		&srcToDestPort, &destToSourcePort, &srcToDestSocket, &destToSourceSocket);

	if (result==UDPFORWARDER_FORWARDING_ALREADY_EXISTS)
	{
		if (debugInterface)
		{
			char buff[512];
			debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_REQUEST_FORWARDING, result=UDPFORWARDER_FORWARDING_ALREADY_EXISTS at %s:%i\n", _FILE_AND_LINE_));
		}

		SendForwardingSuccess(packet->guid, endpointGuid, srcToDestPort);
	}
	else if (result==UDPFORWARDER_NO_SOCKETS || result==UDPFORWARDER_INVALID_PARAMETERS)
	{
		char buff[512];
		if (debugInterface)	debugInterface->ShowFailure(FormatStringTS(buff,"Router2 failed at %s:%i\n", _FILE_AND_LINE_));
		RakAssert(result!=UDPFORWARDER_INVALID_PARAMETERS);
		SendFailureOnCannotForward(packet->guid, endpointGuid);
	}
	else
	{
		if (debugInterface)
		{
			char buff[512];
			debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_REQUEST_FORWARDING, calling SendOOBMessages at %s:%i\n", _FILE_AND_LINE_));
		}

		// Store the punch request
		MiniPunchRequest miniPunchRequest;
		miniPunchRequest.endpointAddress=endpointSystemAddress;
		miniPunchRequest.endpointGuid=endpointGuid;
		miniPunchRequest.gotReplyFromEndpoint=false;
		miniPunchRequest.gotReplyFromSource=false;
		miniPunchRequest.sourceGuid=packet->guid;
		miniPunchRequest.sourceAddress=packet->systemAddress;
		miniPunchRequest.srcToDestPort=srcToDestPort;
		miniPunchRequest.destToSourcePort=destToSourcePort;
		miniPunchRequest.srcToDestSocket=srcToDestSocket;
		miniPunchRequest.destToSourceSocket=destToSourceSocket;
		int ping1 = rakPeerInterface->GetAveragePing(packet->guid);
		int ping2 = rakPeerInterface->GetAveragePing(endpointGuid);
		if (ping1>ping2)
			miniPunchRequest.timeout=RakNet::GetTimeMS()+ping1*8+300;
		else
			miniPunchRequest.timeout=RakNet::GetTimeMS()+ping2*8+300;
		miniPunchRequest.nextAction=RakNet::GetTimeMS()+100;
		SendOOBMessages(&miniPunchRequest);
		miniPunchesInProgress.Push(miniPunchRequest,_FILE_AND_LINE_);
	}
}
void Router2::OnMiniPunchReplyBounce(Packet *packet)
{
	// Find stored punch request
	unsigned int i=0;
	while (i < miniPunchesInProgress.Size())
	{
		if (miniPunchesInProgress[i].sourceGuid==packet->guid || miniPunchesInProgress[i].endpointGuid==packet->guid)
		{
			if (miniPunchesInProgress[i].sourceGuid==packet->guid)
				miniPunchesInProgress[i].gotReplyFromSource=true;
			if (miniPunchesInProgress[i].endpointGuid==packet->guid)
				miniPunchesInProgress[i].gotReplyFromEndpoint=true;

			if (debugInterface)
			{
				char buff[512];
				debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE, gotReplyFromSource=%i gotReplyFromEndpoint=%i at %s:%i\n", miniPunchesInProgress[i].gotReplyFromSource, miniPunchesInProgress[i].gotReplyFromEndpoint, _FILE_AND_LINE_));
			}

			if (miniPunchesInProgress[i].gotReplyFromEndpoint==true &&
				miniPunchesInProgress[i].gotReplyFromSource==true)
			{
				RakNet::BitStream bs;
				rakPeerInterface->WriteOutOfBandHeader(&bs);
				bs.Write((unsigned char) ID_ROUTER_2_REROUTE);
				bs.Write(miniPunchesInProgress[i].sourceGuid);
				SocketLayer::Instance()->SendTo_PC( miniPunchesInProgress[i].srcToDestSocket, (const char*) bs.GetData(), bs.GetNumberOfBytesUsed(), miniPunchesInProgress[i].endpointAddress.binaryAddress, miniPunchesInProgress[i].endpointAddress.port );

				SendForwardingSuccess(miniPunchesInProgress[i].sourceGuid, miniPunchesInProgress[i].endpointGuid, miniPunchesInProgress[i].srcToDestPort);
				miniPunchesInProgress.RemoveAtIndexFast(i);
			}
			else
			{
				i++;
			}
		}
		else
			i++;
	}
}
void Router2::OnMiniPunchReply(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID) + sizeof(unsigned char));
	RakNetGUID routerGuid;
	bs.Read(routerGuid);
	SendOOBFromRakNetPort(ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE, 0, rakPeerInterface->GetSystemAddressFromGuid(routerGuid));

	if (debugInterface)
	{
		char buff[512];
		debugInterface->ShowDiagnostic(FormatStringTS(buff,"Sending ID_ROUTER_2_MINI_PUNCH_REPLY_BOUNCE at %s:%i\n", _FILE_AND_LINE_));
	}
}
bool Router2::OnForwardingSuccess(Packet *packet)
{
	RakNet::BitStream bs(packet->data, packet->length, false);
	bs.IgnoreBytes(sizeof(MessageID));
	RakNetGUID endpointGuid;
	bs.Read(endpointGuid);
	unsigned short sourceToDestPort;
	bs.Read(sourceToDestPort);

	unsigned int forwardingIndex;
	for (forwardingIndex=0; forwardingIndex < forwardedConnectionList.Size(); forwardingIndex++)
	{
		if (forwardedConnectionList[forwardingIndex].endpointGuid==endpointGuid)
			break;
	}

	if (forwardingIndex<forwardedConnectionList.Size())
	{
		// Return rerouted notice
		SystemAddress intermediaryAddress=packet->systemAddress;
		intermediaryAddress.port=sourceToDestPort;
		rakPeerInterface->ChangeSystemAddress(endpointGuid, intermediaryAddress);

		if (debugInterface)
		{
			char buff[512];
			debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got ID_ROUTER_2_FORWARDING_ESTABLISHED, returning ID_ROUTER_2_REROUTED, Calling RakPeer::ChangeSystemAddress at %s:%i\n", _FILE_AND_LINE_));
		}

		packet->data[0]=ID_ROUTER_2_REROUTED;

		return true; // Return packet to user
	}
	else
	{
		// removeFrom connectionRequests;
		unsigned int connectionRequestIndex = GetConnectionRequestIndex(endpointGuid);

		ForwardedConnection fc;
		fc.endpointGuid=endpointGuid;
		fc.intermediaryAddress=packet->systemAddress;
		fc.intermediaryAddress.port=sourceToDestPort;
		fc.intermediaryGuid=packet->guid;
		fc.returnConnectionLostOnFailure=connectionRequests[connectionRequestIndex]->returnConnectionLostOnFailure;
		// add to forwarding list
		forwardedConnectionList.Push (fc,_FILE_AND_LINE_);

		if (debugInterface)
		{
			char buff[512];
			debugInterface->ShowDiagnostic(FormatStringTS(buff,"Got and returning to user ID_ROUTER_2_FORWARDING_ESTABLISHED at %s:%i\n", _FILE_AND_LINE_));
		}

		connectionRequests.RemoveAtIndexFast(connectionRequestIndex);
	}
	return true; // Return packet to user
}
int Router2::GetLargestPingAmongConnectedSystems(void) const
{
	int avePing;
	int largestPing=-1;
	unsigned short maxPeers = rakPeerInterface->GetMaximumNumberOfPeers();
	if (maxPeers==0)
		return 9999;
	unsigned short index;
	for (index=0; index < rakPeerInterface->GetMaximumNumberOfPeers(); index++)
	{
		RakNetGUID g = rakPeerInterface->GetGUIDFromIndex(index);
		if (g!=UNASSIGNED_RAKNET_GUID)
		{
			avePing=rakPeerInterface->GetAveragePing(rakPeerInterface->GetGUIDFromIndex(index));
			if (avePing>largestPing)
				largestPing=avePing;
		}
	}
	return largestPing;
}

unsigned int Router2::GetConnectionRequestIndex(RakNetGUID endpointGuid)
{
	unsigned int i;
	for (i=0; i < connectionRequests.Size(); i++)
	{
		if (connectionRequests[i]->endpointGuid==endpointGuid)
			return i;
	}
	return (unsigned int) -1;
}
unsigned int Router2::ConnnectRequest::GetGuidIndex(RakNetGUID guid)
{
	unsigned int i;
	for (i=0; i < connectionRequestSystems.Size(); i++)
	{
		if (connectionRequestSystems[i].guid==guid)
			return i;
	}
	return (unsigned int) -1;
}
void Router2::ReturnToUser(MessageID messageId, RakNetGUID endpointGuid, SystemAddress systemAddress)
{
	Packet *p = rakPeerInterface->AllocatePacket(sizeof(MessageID)+sizeof(unsigned char));
	p->data[0]=messageId;
	p->systemAddress=systemAddress;
	p->systemAddress.systemIndex=(SystemIndex)-1;
	p->guid=endpointGuid;
	rakPeerInterface->PushBackPacket(p, true);
}
void Router2::ClearForwardedConnections(void)
{
	forwardedConnectionList.Clear(false,_FILE_AND_LINE_);
}
void Router2::ClearAll(void)
{
	ClearConnectionRequests();
	ClearMinipunches();
	ClearForwardedConnections();
}
void Router2::SetDebugInterface(Router2DebugInterface *_debugInterface)
{
	debugInterface=_debugInterface;
}
Router2DebugInterface *Router2::GetDebugInterface(void) const
{
	return debugInterface;
}

#endif // _RAKNET_SUPPORT_*
