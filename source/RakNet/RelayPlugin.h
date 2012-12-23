/// \file
/// \brief Contains the class RelayPlugin
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RelayPlugin==1

#ifndef __RELAY_PLUGIN_H
#define __RELAY_PLUGIN_H

#include "PluginInterface2.h"
#include "RakString.h"
#include "DS_Hash.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

/// \defgroup RELAY_PLUGIN_GROUP RelayPlugin
/// \brief A simple class to relay messages from one system to another through an intermediary
/// \ingroup PLUGINS_GROUP

namespace RakNet
{

/// Forward declarations
class RakPeerInterface;


/// \brief A simple class to relay messages from one system to another through an intermediary
/// \ingroup RELAY_PLUGIN_GROUP
class RAK_DLL_EXPORT RelayPlugin : public PluginInterface2
{
public:
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(RelayPlugin)

	/// Constructor
	RelayPlugin();

	/// Destructor
	virtual ~RelayPlugin();

	/// \brief Forward messages from any system, to the system specified by the combination of key and guid. The sending system only needs to know the key.
	/// \param[in] key A string to identify the target's RakNetGUID. This is so the sending system does not need to know the RakNetGUID of the target system. The key should be unique among all guids added. If the key is not unique, only one system will be sent to (at random).
	/// \param[in] guid The RakNetGuid of the system to send to. If this system disconnects, it is removed from the internal hash 
	/// \return true if the participant was added. False if the target \a guid is not connected
	bool AddParticipant(const RakString &key, const RakNetGUID &guid);

	/// \brief Request that the server relay \a bitStream to the system designated by \a key
	/// \param[in] relayPluginServerGuid the RakNetGUID of the system running RelayPlugin
	/// \param[in] key The key value passed to AddParticipant() earlier on the server. If this was not done, the server will not relay the message (it will be silently discarded).
	/// \param[in] bitStream The data to relay
	/// \param[in] priority See the parameter of the same name in RakPeerInterface::Send()
	/// \param[in] reliability See the parameter of the same name in RakPeerInterface::Send()
	/// \param[in] orderingChannel See the parameter of the same name in RakPeerInterface::Send()
	void SendToParticipant(const RakNetGUID &relayPluginServerGuid, const RakString &key, BitStream *bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel);

	/// \internal
	virtual PluginReceiveResult OnReceive(Packet *packet);
	/// \internal
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason );

	struct StrAndGuid
	{
		RakString str;
		RakNetGUID guid;
	};
	
protected:

	DataStructures::Hash<RakString, StrAndGuid*, 8096, RakNet::RakString::ToInteger> strToGuidHash;
	DataStructures::Hash<RakNetGUID, StrAndGuid*, 8096, RakNet::RakNetGUID::ToUint32> guidToStrHash;

};

} // End namespace

#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // _RAKNET_SUPPORT_*
