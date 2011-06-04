/// \file
/// \brief Remote procedure call, supporting C functions only. No external dependencies required.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RPC4Plugin==1

#ifndef __RPC_4_PLUGIN_H
#define __RPC_4_PLUGIN_H

#include "PluginInterface2.h"
#include "PacketPriority.h"
#include "RakNetTypes.h"
#include "BitStream.h"
#include "RakString.h"
#include "NetworkIDObject.h"
#include "DS_Hash.h"
#include "DS_OrderedList.h"

#ifdef _MSC_VER
#pragma warning( push )
#endif

/// \defgroup RPC_PLUGIN_GROUP RPC
/// \brief Remote procedure calls, without external dependencies.
/// \details This should not be used at the same time as RPC3. This is a less functional version of RPC3, and is here for users that do not want the Boost dependency of RPC3.
/// \ingroup PLUGINS_GROUP

namespace RakNet
{
/// Forward declarations
class RakPeerInterface;
class NetworkIDManager;

	/// \brief Error codes returned by a remote system as to why an RPC function call cannot execute
	/// \details Error code follows packet ID ID_RPC_REMOTE_ERROR, that is packet->data[1]<BR>
	/// Name of the function will be appended starting at packet->data[2]
	/// \ingroup RPC_PLUGIN_GROUP
	enum RPCErrorCodes
	{
		/// Named function was not registered with RegisterFunction(). Check your spelling.
		RPC_ERROR_FUNCTION_NOT_REGISTERED,
	};

	/// \brief Instantiate this class globally if you want to register a function with RPC4 at the global space
	class RAK_DLL_EXPORT RPC4GlobalRegistration
	{
	public:
		/// \brief Queue a call to RPC4::RegisterFunction() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface.
		RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ));

		/// \brief Queue a call to RPC4::RegisterBlockingFunction() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface.
		RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ));

		/// \brief Queue a call to RPC4::RegisterLocalCallback() globally. Actual call occurs once RPC4 is attached to an instance of RakPeer or TCPInterface.
		RPC4GlobalRegistration(const char* uniqueID, MessageID messageId);
	};

	/// \brief The RPC4 plugin is just an association between a C function pointer and a string.
	/// \details It is for users that want to use RPC, but do not want to use boost.
	/// You do not have the automatic serialization or other features of RPC3, and C++ member calls are not supported.
	/// \note You cannot use RPC4 at the same time as RPC3Plugin
	/// \ingroup RPC_PLUGIN_GROUP
	class RAK_DLL_EXPORT RPC4 : public PluginInterface2
	{
	public:
		// GetInstance() and DestroyInstance(instance*)
		STATIC_FACTORY_DECLARATIONS(RPC4)

		// Constructor
		RPC4();

		// Destructor
		virtual ~RPC4();

		/// \brief Register a function pointer to be callable from a remote system
		/// \details The hash of the function name will be stored as an association with the function pointer
		/// When a call is made to call this function from the \a Call() or CallLoopback() function, the function pointer will be invoked with the passed bitStream to Call() and the actual Packet that RakNet got.
		/// \sa RegisterPacketCallback()
		/// \param[in] uniqueID Identifier to be associated with \a functionPointer. If this identifier is already in use, the call will return false.
		/// \param[in] functionPointer C function pointer to be called
		/// \return True if the hash of uniqueID is not in use, false otherwise.
		bool RegisterFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ));

		/// \brief Same as \a RegisterFunction, but is called with CallBlocking() instead of Call() and returns a value to the caller
		bool RegisterBlockingFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ));

		/// When a RakNet Packet with the specified identifier is returned, execute CallLoopback() on a function previously registered with RegisterFunction()
		/// For example, you could call "OnClosedConnection" whenever you get ID_DISCONNECTION_NOTIFICATION or ID_CONNECTION_LOST
		/// \param[in] uniqueID Identifier passed to RegisterFunction()
		/// \param[in] messageId What RakNet packet ID to call on, for example ID_DISCONNECTION_NOTIFICATION or ID_CONNECTION_LOST
		void RegisterLocalCallback(const char* uniqueID, MessageID messageId);

		/// \brief Unregister a function pointer previously registered with RegisterFunction()
		/// \param[in] Identifier originally passed to RegisterFunction()
		/// \return True if the hash of uniqueID was in use, and hence removed. false otherwise.
		bool UnregisterFunction(const char* uniqueID);

		/// \brief Same as UnregisterFunction, except for a blocking function
		bool UnregisterBlockingFunction(const char* uniqueID);

		/// Remove the association created with RegisterPacketCallback()
		/// \param[in] uniqueID Identifier passed as uniqueID to RegisterLocalCallback()
		/// \param[in] messageId Identifier passed as messageId to RegisterLocalCallback()
		/// \return True if the combination of uniqueID and messageId was in use, and hence removed
		bool UnregisterLocalCallback(const char* uniqueID, MessageID messageId);

		/// Send to the attached instance of RakPeer. See RakPeerInterface::SendLoopback()
		/// \param[in] Identifier originally passed to RegisterFunction() on the local system
		/// \param[in] bitStream bitStream encoded data to send to the function callback
		void CallLoopback( const char* uniqueID, RakNet::BitStream * bitStream );

		/// Send to the specified remote instance of RakPeer.
		/// \param[in] Identifier originally passed to RegisterFunction() on the remote system(s)
		/// \param[in] bitStream bitStream encoded data to send to the function callback
		/// \param[in] priority See RakPeerInterface::Send()
		/// \param[in] reliability See RakPeerInterface::Send()
		/// \param[in] orderingChannel See RakPeerInterface::Send()
		/// \param[in] systemIdentifier See RakPeerInterface::Send()
		/// \param[in] broadcast See RakPeerInterface::Send()
		void Call( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast );

		/// \brief Same as call, but don't return until the remote system 
		/// Broadcasting parameter does not exist, this can only call one remote system
		/// \note This function does not return until the remote system responds, disconnects, or was never connected to begin with
		/// \param[in] Identifier originally passed to RegisterBlockingFunction() on the remote system(s)
		/// \param[in] bitStream bitStream encoded data to send to the function callback
		/// \param[in] priority See RakPeerInterface::Send()
		/// \param[in] reliability See RakPeerInterface::Send()
		/// \param[in] orderingChannel See RakPeerInterface::Send()
		/// \param[in] systemIdentifier See RakPeerInterface::Send()
		/// \param[out] returnData Written to by the function registered with RegisterBlockingFunction.
		/// \return true if successfully called. False on disconnect, function not registered, or not connected to begin with
		bool CallBlocking( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, RakNet::BitStream *returnData );

		/// \internal
		struct LocalCallback
		{
			MessageID messageId;
			DataStructures::OrderedList<RakNet::RakString,RakNet::RakString> functions;
		};
		static int LocalCallbackComp(const MessageID &key, LocalCallback* const &data );
	protected:

		// --------------------------------------------------------------------------------------------
		// Packet handling functions
		// --------------------------------------------------------------------------------------------
		virtual void OnAttach(void);
		virtual PluginReceiveResult OnReceive(Packet *packet);

		DataStructures::Hash<RakNet::RakString, void ( * ) ( RakNet::BitStream *, Packet * ),64, RakNet::RakString::ToInteger> registeredNonblockingFunctions;
		DataStructures::Hash<RakNet::RakString, void ( * ) ( RakNet::BitStream *, RakNet::BitStream *, Packet * ),64, RakNet::RakString::ToInteger> registeredBlockingFunctions;
		DataStructures::OrderedList<MessageID,LocalCallback*,RPC4::LocalCallbackComp> localCallbacks;

		RakNet::BitStream blockingReturnValue;
		bool gotBlockingReturnValue;
	};

} // End namespace

#endif

#ifdef _MSC_VER
#pragma warning( pop )
#endif

#endif // _RAKNET_SUPPORT_*
