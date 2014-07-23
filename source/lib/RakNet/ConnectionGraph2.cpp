#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ConnectionGraph2==1

#include "ConnectionGraph2.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"

using namespace RakNet;

STATIC_FACTORY_DEFINITIONS(ConnectionGraph2,ConnectionGraph2)

int RakNet::ConnectionGraph2::RemoteSystemComp( const RakNetGUID &key, RemoteSystem * const &data )
{
	if (key < data->guid)
		return -1;
	if (key > data->guid)
		return 1;
	return 0;
}

int RakNet::ConnectionGraph2::SystemAddressAndGuidComp( const SystemAddressAndGuid &key, const SystemAddressAndGuid &data )
{
	if (key.guid<data.guid)
		return -1;
	if (key.guid>data.guid)
		return 1;
	return 0;
}
ConnectionGraph2::ConnectionGraph2()
{
	autoProcessNewConnections=true;
}
ConnectionGraph2::~ConnectionGraph2()
{

}
bool ConnectionGraph2::GetConnectionListForRemoteSystem(RakNetGUID remoteSystemGuid, SystemAddress *saOut, RakNetGUID *guidOut, unsigned int *outLength)
{
	if ((saOut==0 && guidOut==0) || outLength==0 || *outLength==0 || remoteSystemGuid==UNASSIGNED_RAKNET_GUID)
	{
		*outLength=0;
		return false;
	}

	bool objectExists;
	unsigned int idx = remoteSystems.GetIndexFromKey(remoteSystemGuid, &objectExists);
	if (objectExists==false)
	{
		*outLength=0;
		return false;
	}

	unsigned int idx2;
	if (remoteSystems[idx]->remoteConnections.Size() < *outLength)
		*outLength=remoteSystems[idx]->remoteConnections.Size();
	for (idx2=0; idx2 < *outLength; idx2++)
	{
		if (guidOut)
			guidOut[idx2]=remoteSystems[idx]->remoteConnections[idx2].guid;
		if (saOut)
			saOut[idx2]=remoteSystems[idx]->remoteConnections[idx2].systemAddress;
	}
	return true;
}
bool ConnectionGraph2::ConnectionExists(RakNetGUID g1, RakNetGUID g2)
{
	if (g1==g2)
		return false;

	bool objectExists;
	unsigned int idx = remoteSystems.GetIndexFromKey(g1, &objectExists);
	if (objectExists==false)
	{
		return false;
	}
	SystemAddressAndGuid sag;
	sag.guid=g2;
	return remoteSystems[idx]->remoteConnections.HasData(sag);
}
uint16_t ConnectionGraph2::GetPingBetweenSystems(RakNetGUID g1, RakNetGUID g2) const
{
	if (g1==g2)
		return 0;

	if (g1==rakPeerInterface->GetMyGUID())
		return (uint16_t) rakPeerInterface->GetAveragePing(g2);
	if (g2==rakPeerInterface->GetMyGUID())
		return (uint16_t) rakPeerInterface->GetAveragePing(g1);

	bool objectExists;
	unsigned int idx = remoteSystems.GetIndexFromKey(g1, &objectExists);
	if (objectExists==false)
	{
		return (uint16_t) -1;
	}

	SystemAddressAndGuid sag;
	sag.guid=g2;
	unsigned int idx2 = remoteSystems[idx]->remoteConnections.GetIndexFromKey(sag, &objectExists);
	if (objectExists==false)
	{
		return (uint16_t) -1;
	}
	return remoteSystems[idx]->remoteConnections[idx2].sendersPingToThatSystem;
}

/// Returns the system with the lowest total ping among all its connections. This can be used as the 'best host' for a peer to peer session
RakNetGUID ConnectionGraph2::GetLowestAveragePingSystem(void) const
{
	float lowestPing=-1.0;
	unsigned int lowestPingIdx=(unsigned int) -1;
	float thisAvePing=0.0f;
	unsigned int idx, idx2;
	int ap, count=0;

	for (idx=0; idx<remoteSystems.Size(); idx++)
	{
		thisAvePing=0.0f;

		ap = rakPeerInterface->GetAveragePing(remoteSystems[idx]->guid);
		if (ap!=-1)
		{
			thisAvePing+=(float) ap;
			count++;
		}
	}

	if (count>0)
	{
		lowestPing=thisAvePing/count;
	}

	for (idx=0; idx<remoteSystems.Size(); idx++)
	{
		thisAvePing=0.0f;
		count=0;

		RemoteSystem *remoteSystem = remoteSystems[idx];
		for (idx2=0; idx2 < remoteSystem->remoteConnections.Size(); idx2++)
		{
			ap=remoteSystem->remoteConnections[idx2].sendersPingToThatSystem;
			if (ap!=-1)
			{
				thisAvePing+=(float) ap;
				count++;
			}
		}

		if (count>0 && (lowestPing==-1.0f || thisAvePing/count < lowestPing))
		{
			lowestPing=thisAvePing/count;
			lowestPingIdx=idx;
		}
	}

	if (lowestPingIdx==(unsigned int) -1)
		return rakPeerInterface->GetMyGUID();
	return remoteSystems[lowestPingIdx]->guid;
}

void ConnectionGraph2::OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	// Send notice to all existing connections
	RakNet::BitStream bs;
	if (lostConnectionReason==LCR_CONNECTION_LOST)
		bs.Write((MessageID)ID_REMOTE_CONNECTION_LOST);
	else
		bs.Write((MessageID)ID_REMOTE_DISCONNECTION_NOTIFICATION);
	bs.Write(systemAddress);
	bs.Write(rakNetGUID);
	SendUnified(&bs,HIGH_PRIORITY,RELIABLE_ORDERED,0,systemAddress,true);

	bool objectExists;
	unsigned int idx = remoteSystems.GetIndexFromKey(rakNetGUID, &objectExists);
	if (objectExists)
	{
		RakNet::OP_DELETE(remoteSystems[idx],_FILE_AND_LINE_);
		remoteSystems.RemoveAtIndex(idx);
	}
}
void ConnectionGraph2::SetAutoProcessNewConnections(bool b)
{
	autoProcessNewConnections=b;
}
bool ConnectionGraph2::GetAutoProcessNewConnections(void) const
{
	return autoProcessNewConnections;
}
void ConnectionGraph2::AddParticipant(const SystemAddress &systemAddress, RakNetGUID rakNetGUID)
{
	// Relay the new connection to other systems.
	RakNet::BitStream bs;
	bs.Write((MessageID)ID_REMOTE_NEW_INCOMING_CONNECTION);
	bs.Write((uint32_t)1);
	bs.Write(systemAddress);
	bs.Write(rakNetGUID);
	bs.WriteCasted<uint16_t>(rakPeerInterface->GetAveragePing(rakNetGUID));
	SendUnified(&bs,HIGH_PRIORITY,RELIABLE_ORDERED,0,systemAddress,true);

	// Send everyone to the new guy
	DataStructures::List<SystemAddress> addresses;
	DataStructures::List<RakNetGUID> guids;
	rakPeerInterface->GetSystemList(addresses, guids);
	bs.Reset();
	bs.Write((MessageID)ID_REMOTE_NEW_INCOMING_CONNECTION);
	BitSize_t writeOffset = bs.GetWriteOffset();
	bs.Write((uint32_t) addresses.Size());

	unsigned int i;
	uint32_t count=0;
	for (i=0; i < addresses.Size(); i++)
	{
		if (addresses[i]==systemAddress)
			continue;

		bs.Write(addresses[i]);
		bs.Write(guids[i]);
		bs.WriteCasted<uint16_t>(rakPeerInterface->GetAveragePing(guids[i]));
		count++;
	}

	if (count>0)
	{
		BitSize_t writeOffset2 = bs.GetWriteOffset();
		bs.SetWriteOffset(writeOffset);
		bs.Write(count);
		bs.SetWriteOffset(writeOffset2);
		SendUnified(&bs,HIGH_PRIORITY,RELIABLE_ORDERED,0,systemAddress,false);
	}

	bool objectExists;
	unsigned int ii = remoteSystems.GetIndexFromKey(rakNetGUID, &objectExists);
	if (objectExists==false)
	{
		RemoteSystem* remoteSystem = RakNet::OP_NEW<RemoteSystem>(_FILE_AND_LINE_);
		remoteSystem->guid=rakNetGUID;
		remoteSystems.InsertAtIndex(remoteSystem,ii,_FILE_AND_LINE_);
	}
}
void ConnectionGraph2::GetParticipantList(DataStructures::OrderedList<RakNetGUID, RakNetGUID> &participantList)
{
	participantList.Clear(true, _FILE_AND_LINE_);
	unsigned int i;
	for (i=0; i < remoteSystems.Size(); i++)
		participantList.InsertAtEnd(remoteSystems[i]->guid, _FILE_AND_LINE_);
}
void ConnectionGraph2::OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming)
{
	(void) isIncoming;
	if (autoProcessNewConnections)
		AddParticipant(systemAddress, rakNetGUID);
}
PluginReceiveResult ConnectionGraph2::OnReceive(Packet *packet)
{
	if (packet->data[0]==ID_REMOTE_CONNECTION_LOST || packet->data[0]==ID_REMOTE_DISCONNECTION_NOTIFICATION)
	{
		bool objectExists;
		unsigned idx = remoteSystems.GetIndexFromKey(packet->guid, &objectExists);
		if (objectExists)
		{
			RakNet::BitStream bs(packet->data,packet->length,false);
			bs.IgnoreBytes(1);
			SystemAddressAndGuid saag;
			bs.Read(saag.systemAddress);
			bs.Read(saag.guid);
			unsigned long idx2 = remoteSystems[idx]->remoteConnections.GetIndexFromKey(saag, &objectExists);
			if (objectExists)
				remoteSystems[idx]->remoteConnections.RemoveAtIndex(idx2);
		}
	}
	else if (packet->data[0]==ID_REMOTE_NEW_INCOMING_CONNECTION)
	{
		bool objectExists;
		unsigned idx = remoteSystems.GetIndexFromKey(packet->guid, &objectExists);
		if (objectExists)
		{
			uint32_t numAddresses;
			RakNet::BitStream bs(packet->data,packet->length,false);
			bs.IgnoreBytes(1);
			bs.Read(numAddresses);
			for (unsigned int idx2=0; idx2 < numAddresses; idx2++)
			{
				SystemAddressAndGuid saag;
				bs.Read(saag.systemAddress);
				bs.Read(saag.guid);
				bs.Read(saag.sendersPingToThatSystem);
				bool objectExists;
				unsigned int ii = remoteSystems[idx]->remoteConnections.GetIndexFromKey(saag, &objectExists);
				if (objectExists==false)
					remoteSystems[idx]->remoteConnections.InsertAtIndex(saag,ii,_FILE_AND_LINE_);
			}
		}
	}
	
	return RR_CONTINUE_PROCESSING;
}

#endif // _RAKNET_SUPPORT_*
