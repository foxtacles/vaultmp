/// \file
/// \brief Contains the NAT-punchthrough plugin for the server.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_NatPunchthroughServer==1

#ifndef __NAT_PUNCHTHROUGH_SERVER_H
#define __NAT_PUNCHTHROUGH_SERVER_H

#include "RakNetTypes.h"
#include "Export.h"
#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "SocketIncludes.h"
#include "DS_OrderedList.h"
#include "RakString.h"

namespace RakNet
{
/// Forward declarations
class RakPeerInterface;
struct Packet;
#if _RAKNET_SUPPORT_PacketLogger==1
class PacketLogger;
#endif

/// \defgroup NAT_PUNCHTHROUGH_GROUP NatPunchthrough
/// \brief Connect systems despite both systems being behind a router
/// \details
/// \ingroup PLUGINS_GROUP

/// \ingroup NAT_PUNCHTHROUGH_GROUP
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface
{
	NatPunchthroughServerDebugInterface() {}
	virtual ~NatPunchthroughServerDebugInterface() {}
	virtual void OnServerMessage(const char *msg)=0;
};

/// \ingroup NAT_PUNCHTHROUGH_GROUP
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface_Printf : public NatPunchthroughServerDebugInterface
{
	virtual void OnServerMessage(const char *msg);
};

#if _RAKNET_SUPPORT_PacketLogger==1
/// \ingroup NAT_PUNCHTHROUGH_GROUP
struct RAK_DLL_EXPORT NatPunchthroughServerDebugInterface_PacketLogger : public NatPunchthroughServerDebugInterface
{
	// Set to non-zero to write to the packetlogger!
	PacketLogger *pl;

	NatPunchthroughServerDebugInterface_PacketLogger() {pl=0;}
	~NatPunchthroughServerDebugInterface_PacketLogger() {}
	virtual void OnServerMessage(const char *msg);
};
#endif

/// \brief Server code for NATPunchthrough
/// \details Maintain connection to NatPunchthroughServer to process incoming connection attempts through NatPunchthroughClient<BR>
/// Server maintains two sockets clients can connect to so as to estimate the next port choice<BR>
/// Server tells other client about port estimate, current public port to the server, and a time to start connection attempts
/// \sa NatTypeDetectionClient
/// See also http://www.jenkinssoftware.com/raknet/manual/natpunchthrough.html
/// \ingroup NAT_PUNCHTHROUGH_GROUP
class RAK_DLL_EXPORT NatPunchthroughServer : public PluginInterface2
{
public:

	STATIC_FACTORY_DECLARATIONS(NatPunchthroughServer)

	// Constructor
	NatPunchthroughServer();

	// Destructor
	virtual ~NatPunchthroughServer();

	/// Sets a callback to be called with debug messages
	/// \param[in] i Pointer to an interface. The pointer is stored, so don't delete it while in progress. Pass 0 to clear.
	void SetDebugInterface(NatPunchthroughServerDebugInterface *i);

	/// \internal For plugin handling
	virtual void Update(void);

	/// \internal For plugin handling
	virtual PluginReceiveResult OnReceive(Packet *packet);

	/// \internal For plugin handling
	virtual void OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason );
	virtual void OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming);

	// Each connected user has a ready state. Ready means ready for nat punchthrough.
	struct User;
	struct ConnectionAttempt
	{
		ConnectionAttempt() {sender=0; recipient=0; startTime=0; attemptPhase=NAT_ATTEMPT_PHASE_NOT_STARTED;}
		User *sender, *recipient;
		uint16_t sessionId;
		RakNet::Time startTime;
		enum
		{
			NAT_ATTEMPT_PHASE_NOT_STARTED,
			NAT_ATTEMPT_PHASE_GETTING_RECENT_PORTS,
		} attemptPhase;
	};
	struct User
	{
		RakNetGUID guid;
		SystemAddress systemAddress;
		unsigned short mostRecentPort;
		bool isReady;
		DataStructures::OrderedList<RakNetGUID,RakNetGUID> groupPunchthroughRequests;

		DataStructures::List<ConnectionAttempt *> connectionAttempts;
		bool HasConnectionAttemptToUser(User *user);
		void DerefConnectionAttempt(ConnectionAttempt *ca);
		void DeleteConnectionAttempt(ConnectionAttempt *ca);
		void LogConnectionAttempts(RakNet::RakString &rs);
	};
	RakNet::Time lastUpdate;
	static int NatPunchthroughUserComp( const RakNetGUID &key, User * const &data );
protected:
	void OnNATPunchthroughRequest(Packet *packet);
	DataStructures::OrderedList<RakNetGUID, User*, NatPunchthroughServer::NatPunchthroughUserComp> users;

	void OnGetMostRecentPort(Packet *packet);
	void OnClientReady(Packet *packet);

	void SendTimestamps(void);
	void StartPendingPunchthrough(void);
	void StartPunchthroughForUser(User*user);
	uint16_t sessionId;
	NatPunchthroughServerDebugInterface *natPunchthroughServerDebugInterface;

	SystemAddress boundAddresses[MAXIMUM_NUMBER_OF_INTERNAL_IDS];
	unsigned char boundAddressCount;

};

} // namespace RakNet

#endif

#endif // _RAKNET_SUPPORT_*
