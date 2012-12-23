#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RelayPlugin==1

#include "RelayPlugin.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "BitStream.h"

using namespace RakNet;

STATIC_FACTORY_DEFINITIONS(RelayPlugin,RelayPlugin);

RelayPlugin::RelayPlugin()
{

}

RelayPlugin::~RelayPlugin()
{
	DataStructures::List<StrAndGuid*> itemList;
	DataStructures::List<RakString> keyList;
	strToGuidHash.GetAsList(itemList, keyList, _FILE_AND_LINE_);
	guidToStrHash.Clear(_FILE_AND_LINE_);
	for (unsigned int i=0; i < itemList.Size(); i++)
		RakNet::OP_DELETE(itemList[i], _FILE_AND_LINE_);
}

bool RelayPlugin::AddParticipant(const RakString &key, const RakNetGUID &guid)
{
	ConnectionState cs = rakPeerInterface->GetConnectionState(guid);
	if (cs!=IS_CONNECTED)
		return false;

	StrAndGuid *strAndGuid = RakNet::OP_NEW<StrAndGuid>(_FILE_AND_LINE_);
	strAndGuid->guid=guid;
	strAndGuid->str=key;

	strToGuidHash.Push(key, strAndGuid, _FILE_AND_LINE_);
	guidToStrHash.Push(guid, strAndGuid, _FILE_AND_LINE_);

	return true;
}

// Send a message to a server running RelayPlugin, to forward a message to the system identified by \a key
void RelayPlugin::SendToParticipant(const RakNetGUID &relayPluginServerGuid, const RakString &key, BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel)
{
	BitStream bsOut;
	bsOut.WriteCasted<MessageID>(ID_RELAY_PLUGIN_TO_RELAY);
	bsOut.WriteCasted<unsigned char>(priority);
	bsOut.WriteCasted<unsigned char>(reliability);
	bsOut.Write(orderingChannel);
	bsOut.WriteCompressed(key);
	bsOut.Write(bitStream);
	SendUnified(&bsOut, priority, reliability, orderingChannel, relayPluginServerGuid, false);
}

PluginReceiveResult RelayPlugin::OnReceive(Packet *packet)
{
	switch (packet->data[0])
	{
	case ID_RELAY_PLUGIN_TO_RELAY:
		{
			BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(sizeof(MessageID));
			PacketPriority priority;
			PacketReliability reliability;
			char orderingChannel;
			unsigned char cIn;
			bsIn.Read(cIn);
			priority = (PacketPriority) cIn;
			bsIn.Read(cIn);
			reliability = (PacketReliability) cIn;
			bsIn.Read(orderingChannel);
			RakString key;
			bsIn.ReadCompressed(key);
			BitStream bsData;
			bsIn.Read(&bsData);
			StrAndGuid **strAndGuid = strToGuidHash.Peek(key);
			if (strAndGuid)
			{
				BitStream bsOut;
				bsOut.WriteCasted<MessageID>(ID_RELAY_PLUGIN_FROM_RELAY);
				bsOut.Write(bsData);
				SendUnified(&bsOut, priority, reliability, orderingChannel, (*strAndGuid)->guid, false);
			}

			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}
	}
	return RR_CONTINUE_PROCESSING;
}

void RelayPlugin::OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	(void) lostConnectionReason;
	(void) systemAddress;

	StrAndGuid *strAndGuid;
	if (guidToStrHash.Pop(strAndGuid, rakNetGUID, _FILE_AND_LINE_))
	{
		strToGuidHash.Remove(strAndGuid->str, _FILE_AND_LINE_);
		RakNet::OP_DELETE(strAndGuid, _FILE_AND_LINE_);
	}
}

#endif // _RAKNET_SUPPORT_*
