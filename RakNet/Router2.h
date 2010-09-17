/// \file
/// \brief Router2 plugin. Allows you to connect to a system by routing packets through another system that is connected to both you and the destination. Useful for getting around NATs.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_Router2==1

#ifndef __ROUTER_2_PLUGIN_H
#define __ROUTER_2_PLUGIN_H

#include "RakNetTypes.h"
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "Export.h"
#include "UDPForwarder.h"
#include "MessageIdentifiers.h"

namespace RakNet
{
/// Forward declarations
class RakPeerInterface;

struct Router2DebugInterface
{
	Router2DebugInterface() {}
	virtual ~Router2DebugInterface() {}
	virtual void ShowFailure(const char *message);
	virtual void ShowDiagnostic(const char *message);
};

/// \defgroup ROUTER_2_GROUP Router2
/// \brief Part of the NAT punchthrough solution, allowing you to connect to systems by routing through a shared connection.
/// \details
/// \ingroup PLUGINS_GROUP

/// \ingroup ROUTER_2_GROUP
/// \brief Class interface for the Router2 system
/// \details
class RAK_DLL_EXPORT Router2 : public PluginInterface2
{
public:
	// GetInstance() and DestroyInstance(instance*)
	STATIC_FACTORY_DECLARATIONS(Router2)

	Router2();
	virtual ~Router2();

	/// \brief Query all connected systems to connect through them to a third system.
	/// System will return ID_ROUTER_2_FORWARDING_NO_PATH if unable to connect.
	/// Else you will get ID_ROUTER_2_FORWARDING_ESTABLISHED
	///
	/// On ID_ROUTER_2_FORWARDING_ESTABLISHED, EstablishRouting as follows:
	///
	/// RakNet::BitStream bs(packet->data, packet->length, false);
	/// bs.IgnoreBytes(sizeof(MessageID));
	/// RakNetGUID endpointGuid;
	/// bs.Read(endpointGuid);
	/// unsigned short sourceToDestPort;
	/// bs.Read(sourceToDestPort);
	/// char ipAddressString[32];
	/// packet->systemAddress.ToString(false, ipAddressString);
	/// rakPeerInterface->EstablishRouting(ipAddressString, sourceToDestPort, 0,0);
	///
	/// \note The SystemAddress for a connection should not be used - always use RakNetGuid as the address can change at any time.
	/// When the address changes, you will get ID_ROUTER_2_REROUTED
	void EstablishRouting(RakNetGUID endpointGuid);

	/// Set the maximum number of bidirectional connections this system will support
	/// Defaults to 0
	void SetMaximumForwardingRequests(int max);

	/// For testing and debugging
	void SetDebugInterface(Router2DebugInterface *_debugInterface);

	/// Get the pointer passed to SetDebugInterface()
	Router2DebugInterface *GetDebugInterface(void) const;

	// --------------------------------------------------------------------------------------------
	// Packet handling functions
	// --------------------------------------------------------------------------------------------
	virtual PluginReceiveResult OnReceive(Packet *packet);
	virtual void Update(void);
	virtual void OnClosedConnection(SystemAddress systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason );
	virtual void OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason);
	virtual void OnRakPeerShutdown(void);


	enum Router2RequestStates
	{
		R2RS_REQUEST_STATE_QUERY_FORWARDING,
		REQUEST_STATE_REQUEST_FORWARDING,
	};

	struct ConnectionRequestSystem
	{
		RakNetGUID guid;
		int pingToEndpoint;
		unsigned short usedForwardingEntries;
	};

	struct ConnnectRequest
	{
		ConnnectRequest();
		~ConnnectRequest();

		DataStructures::List<ConnectionRequestSystem> connectionRequestSystems;
		Router2RequestStates requestState;
		RakNet::TimeMS pingTimeout;
		RakNetGUID endpointGuid;
		RakNetGUID lastRequestedForwardingSystem;
		bool returnConnectionLostOnFailure;
		unsigned int GetGuidIndex(RakNetGUID guid);
	};

	unsigned int GetConnectionRequestIndex(RakNetGUID endpointGuid);

	struct MiniPunchRequest
	{
		RakNetGUID endpointGuid;
		SystemAddress endpointAddress;
		bool gotReplyFromEndpoint;
		RakNetGUID sourceGuid;
		SystemAddress sourceAddress;
		bool gotReplyFromSource;
		RakNet::TimeMS timeout;
		RakNet::TimeMS nextAction;
		unsigned short srcToDestPort;
		unsigned short destToSourcePort;
		SOCKET srcToDestSocket;
		SOCKET destToSourceSocket;
	};

	struct ForwardedConnection
	{
		RakNetGUID endpointGuid;
		RakNetGUID intermediaryGuid;
		SystemAddress intermediaryAddress;
		bool returnConnectionLostOnFailure;
	};

protected:

	bool UpdateForwarding(unsigned int connectionRequestIndex);
	void RemoveConnectionRequest(unsigned int connectionRequestIndex);
	void RequestForwarding(unsigned int connectionRequestIndex);
	void OnQueryForwarding(Packet *packet);
	void OnQueryForwardingReply(Packet *packet);
	void OnRequestForwarding(Packet *packet);
	void OnReroute(Packet *packet);
	void OnMiniPunchReply(Packet *packet);
	void OnMiniPunchReplyBounce(Packet *packet);
	bool OnForwardingSuccess(Packet *packet);
	int GetLargestPingAmongConnectedSystems(void) const;
	void ReturnToUser(MessageID messageId, RakNetGUID endpointGuid, SystemAddress systemAddress);
	bool ConnectInternal(RakNetGUID endpointGuid, bool returnConnectionLostOnFailure);

	UDPForwarder *udpForwarder;
	int maximumForwardingRequests;
	DataStructures::List<ConnnectRequest*> connectionRequests;
	DataStructures::List<MiniPunchRequest> miniPunchesInProgress;
	DataStructures::List<ForwardedConnection> forwardedConnectionList;

	void ClearConnectionRequests(void);
	void ClearMinipunches(void);
	void ClearForwardedConnections(void);
	void ClearAll(void);
	int ReturnFailureOnCannotForward(RakNetGUID sourceGuid, RakNetGUID endpointGuid);
	void SendFailureOnCannotForward(RakNetGUID sourceGuid, RakNetGUID endpointGuid);
	void SendForwardingSuccess(RakNetGUID sourceGuid, RakNetGUID endpointGuid, unsigned short sourceToDstPort);
	void SendOOBFromRakNetPort(OutOfBandIdentifiers oob, BitStream *extraData, SystemAddress sa);
	void SendOOBFromSpecifiedSocket(OutOfBandIdentifiers oob, SystemAddress sa, SOCKET socket);
	void SendOOBMessages(MiniPunchRequest *mpr);

	Router2DebugInterface *debugInterface;
};

}

#endif

#endif // _RAKNET_SUPPORT_*
