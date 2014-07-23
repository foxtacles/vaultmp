/// \file ConnectionGraph2.h
/// \brief Connection graph plugin, version 2. Tells new systems about existing and new connections
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_ConnectionGraph2==1

#ifndef __CONNECTION_GRAPH_2_H
#define __CONNECTION_GRAPH_2_H

#include "RakMemoryOverride.h"
#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "DS_List.h"
#include "DS_WeightedGraph.h"
#include "GetTime.h"
#include "Export.h"

namespace RakNet
{
/// Forward declarations
class RakPeerInterface;

/// \brief A one hop connection graph.
/// \details Sends ID_REMOTE_CONNECTION_LOST, ID_REMOTE_DISCONNECTION_NOTIFICATION, ID_REMOTE_NEW_INCOMING_CONNECTION<BR>
/// All identifiers are followed by SystemAddress, then RakNetGUID
/// Also stores the list for you, which you can access with GetConnectionListForRemoteSystem 
/// \ingroup CONNECTION_GRAPH_GROUP
class RAK_DLL_EXPORT ConnectionGraph2 : public PluginInterface2
{
public:

	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(ConnectionGraph2)

	ConnectionGraph2();
	~ConnectionGraph2();

	/// \brief Given a remote system identified by RakNetGUID, return the list of SystemAddresses and RakNetGUIDs they are connected to 
	/// \param[in] remoteSystemGuid Which system we are referring to. This only works for remote systems, not ourselves.
	/// \param[out] saOut A preallocated array to hold the output list of SystemAddress. Can be 0 if you don't care.
	/// \param[out] guidOut A preallocated array to hold the output list of RakNetGUID. Can be 0 if you don't care.
	/// \param[in,out] outLength On input, the size of \a saOut and \a guidOut. On output, modified to reflect the number of elements actually written
	/// \return True if \a remoteSystemGuid was found. Otherwise false, and \a saOut, \a guidOut remain unchanged. \a outLength will be set to 0.
	bool GetConnectionListForRemoteSystem(RakNetGUID remoteSystemGuid, SystemAddress *saOut, RakNetGUID *guidOut, unsigned int *outLength);

	/// Returns if g1 is connected to g2
	bool ConnectionExists(RakNetGUID g1, RakNetGUID g2);

	/// Returns the average ping between two systems in the connection graph. Returns -1 if no connection exists between those systems
	uint16_t GetPingBetweenSystems(RakNetGUID g1, RakNetGUID g2) const;

	/// Returns the system with the lowest average ping among all its connections.
	/// If you need one system in the peer to peer group to relay data, have the FullyConnectedMesh2 host call this function after host migration, and use that system
	RakNetGUID GetLowestAveragePingSystem(void) const;

	/// \brief If called with false, then new connections are only added to the connection graph when you call ProcessNewConnection();
	/// \details This is useful if you want to perform validation before connecting a system to a mesh, or if you want a submesh (for example a server cloud)
	/// \param[in] b True to automatically call ProcessNewConnection() on any new connection, false to not do so. Defaults to true.
	void SetAutoProcessNewConnections(bool b);

	/// \brief Returns value passed to SetAutoProcessNewConnections()
	/// \return Value passed to SetAutoProcessNewConnections(), or the default of true if it was never called
	bool GetAutoProcessNewConnections(void) const;

	/// \brief If you call SetAutoProcessNewConnections(false);, then you will need to manually call ProcessNewConnection() on new connections
	/// \details On ID_NEW_INCOMING_CONNECTION or ID_CONNECTION_REQUEST_ACCEPTED, adds that system to the graph
	/// Do not call ProcessNewConnection() manually otherwise
	/// \param[in] The packet->SystemAddress member
	/// \param[in] The packet->guid member
	void AddParticipant(const SystemAddress &systemAddress, RakNetGUID rakNetGUID);

	/// Get the participants added with AddParticipant()
	/// \param[out] participantList Participants added with AddParticipant();
	void GetParticipantList(DataStructures::OrderedList<RakNetGUID, RakNetGUID> &participantList);

	/// \internal
	struct SystemAddressAndGuid
	{
		SystemAddress systemAddress;
		RakNetGUID guid;
		uint16_t sendersPingToThatSystem;
	};
	/// \internal
	static int SystemAddressAndGuidComp( const SystemAddressAndGuid &key, const SystemAddressAndGuid &data );

	/// \internal
	struct RemoteSystem
	{
		DataStructures::OrderedList<SystemAddressAndGuid,SystemAddressAndGuid,ConnectionGraph2::SystemAddressAndGuidComp> remoteConnections;
		RakNetGUID guid;
	};
	/// \internal
	static int RemoteSystemComp( const RakNetGUID &key, RemoteSystem * const &data );
	
protected:
	/// \internal
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason );
	/// \internal
	virtual void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming);
	/// \internal
	virtual PluginReceiveResult OnReceive(Packet *packet);

	// List of systems I am connected to, which in turn stores which systems they are connected to
	DataStructures::OrderedList<RakNetGUID, RemoteSystem*, ConnectionGraph2::RemoteSystemComp> remoteSystems;

	bool autoProcessNewConnections;

};

} // namespace RakNet

#endif // #ifndef __CONNECTION_GRAPH_2_H

#endif // _RAKNET_SUPPORT_*
