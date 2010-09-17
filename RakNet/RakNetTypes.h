/// \file
/// \brief Types used by RakNet, most of which involve user code.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#ifndef __NETWORK_TYPES_H
#define __NETWORK_TYPES_H

#include "RakNetDefines.h"
#include "NativeTypes.h"
#include "RakNetTime.h"
#include "Export.h"

namespace RakNet {
/// Forward declarations
class RakPeerInterface;
class BitStream;

enum StartupResult
{
	RAKNET_STARTED,
	RAKNET_ALREADY_STARTED,
	INVALID_SOCKET_DESCRIPTORS,
	INVALID_MAX_CONNECTIONS,
	SOCKET_PORT_ALREADY_IN_USE,
	SOCKET_FAILED_TO_BIND,
	SOCKET_FAILED_TEST_SEND,
	FAILED_TO_CREATE_NETWORK_THREAD,
};


enum ConnectionAttemptResult
{
	CONNECTION_ATTEMPT_STARTED,
	INVALID_PARAMETER,
	CANNOT_RESOLVE_DOMAIN_NAME,
	ALREADY_CONNECTED_TO_ENDPOINT,
	CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS,
	SECURITY_INITIALIZATION_FAILED
};

/// Returned from RakPeerInterface::GetConnectionState()
enum ConnectionState
{
	/// Not applicable, because the passed address is the loopback address
	IS_LOOPBACK,
	/// Connect() was called, but the process hasn't started yet
	IS_PENDING,
	/// Processing the connection attempt
	IS_CONNECTING,
	/// Is connected and able to communicate
	IS_CONNECTED,
	/// Was connected, but will disconnect as soon as the remaining messages are delivered
	IS_DISCONNECTING,
	/// A connection attempt failed and will be aborted
	IS_SILENTLY_DISCONNECTING,
	/// No longer connected
	IS_DISCONNECTED,
	/// Was never connected, or else was disconnected long enough ago that the entry has been discarded
	IS_NOT_CONNECTED,
};

/// Given a number of bits, return how many bytes are needed to represent that.
#define BITS_TO_BYTES(x) (((x)+7)>>3)
#define BYTES_TO_BITS(x) ((x)<<3)

/// \sa NetworkIDObject.h
typedef unsigned char UniqueIDType;
typedef unsigned short SystemIndex;
typedef unsigned char RPCIndex;
const int MAX_RPC_MAP_SIZE=((RPCIndex)-1)-1;
const int UNDEFINED_RPC_INDEX=((RPCIndex)-1);

/// First byte of a network message
typedef unsigned char MessageID;

typedef uint32_t BitSize_t;

#if defined(_MSC_VER) && _MSC_VER > 0
#define PRINTF_64_BIT_MODIFIER "I64"
#else
#define PRINTF_64_BIT_MODIFIER "ll"
#endif

/// Used with the PublicKey structure
enum PublicKeyMode
{
	/// The connection is insecure. You can also just pass 0 for the pointer to PublicKey in RakPeerInterface::Connect()
	PKM_INSECURE_CONNECTION,
	/// Use a known public key. PublicKey::publicKey must be non-zero
	PKM_USE_KNOWN_PUBLIC_KEY,
	/// Accept whatever public key the server gives us. This is vulnerable to man in the middle, but does not require
	/// Distribution of the public key in advance of connecting.
	PKM_ACCEPT_ANY_PUBLIC_KEY
};

/// Passed to RakPeerInterface::Connect()
struct RAK_DLL_EXPORT PublicKey
{
	/// Pointer to a public key of length cat::EasyHandshake::PUBLIC_KEY_BYTES. See the Encryption sample.
	char *publicKey;
	/// How to interpret the public key 
	PublicKeyMode publicKeyMode;
};

/// Describes the local socket to use for RakPeer::Startup
struct RAK_DLL_EXPORT SocketDescriptor
{
	SocketDescriptor();
	SocketDescriptor(unsigned short _port, const char *_hostAddress);

	/// The local port to bind to.  Pass 0 to have the OS autoassign a port.
	unsigned short port;

	/// The local network card address to bind to, such as "127.0.0.1".  Pass an empty string to use INADDR_ANY.
	char hostAddress[32];

	// Only need to set for the PS3, when using signaling.
	// Connect with the port returned by signaling. Set this to whatever port RakNet was actually started on
	unsigned short remotePortRakNetWasStartedOn_PS3;
};

extern bool NonNumericHostString( const char *host );

/// \brief Network address for a system
/// \details Corresponds to a network address<BR>
/// This is not necessarily a unique identifier. For example, if a system has both LAN and internet connections, the system may be identified by either one, depending on who is communicating<BR>
/// Use RakNetGUID for a unique per-instance of RakPeer to identify systems
struct RAK_DLL_EXPORT SystemAddress
{
	SystemAddress();
	explicit SystemAddress(const char *a, unsigned short b);
	explicit SystemAddress(unsigned int a, unsigned short b);

	///The peer address from inet_addr.
	uint32_t binaryAddress;
	///The port number
	unsigned short port;
	// Used internally for fast lookup. Optional (use -1 to do regular lookup). Don't transmit this.
	SystemIndex systemIndex;
	static int size() {return (int) sizeof(uint32_t)+sizeof(unsigned short);}

	// Return the systemAddress as a string in the format <IP>:<Port>
	// Returns a static string
	// NOT THREADSAFE
	const char *ToString(bool writePort=true) const;

	// Return the systemAddress as a string in the format <IP>:<Port>
	// dest must be large enough to hold the output
	// THREADSAFE
	void ToString(bool writePort, char *dest) const;

	// Sets the binary address part from a string.  Doesn't set the port
	void SetBinaryAddress(const char *str);

	SystemAddress& operator = ( const SystemAddress& input )
	{
		binaryAddress = input.binaryAddress;
		port = input.port;
		systemIndex = input.systemIndex;
		return *this;
	}

	bool operator==( const SystemAddress& right ) const;
	bool operator!=( const SystemAddress& right ) const;
	bool operator > ( const SystemAddress& right ) const;
	bool operator < ( const SystemAddress& right ) const;
};

/// Uniquely identifies an instance of RakPeer. Use RakPeer::GetGuidFromSystemAddress() and RakPeer::GetSystemAddressFromGuid() to go between SystemAddress and RakNetGUID
/// Use RakPeer::GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS) to get your own GUID
struct RAK_DLL_EXPORT RakNetGUID
{
	RakNetGUID() {systemIndex=(SystemIndex)-1;}
	explicit RakNetGUID(uint64_t _g) {g=_g; systemIndex=(SystemIndex)-1;}
//	uint32_t g[6];
	uint64_t g;

	// Return the GUID as a string
	// Returns a static string
	// NOT THREADSAFE
	const char *ToString(void) const;

	// Return the GUID as a string
	// dest must be large enough to hold the output
	// THREADSAFE
	void ToString(char *dest) const;

	bool FromString(const char *source);

	RakNetGUID& operator = ( const RakNetGUID& input )
	{
		g=input.g;
		systemIndex=input.systemIndex;
		return *this;
	}

	// Used internally for fast lookup. Optional (use -1 to do regular lookup). Don't transmit this.
	SystemIndex systemIndex;
	static const int size() {return (int) sizeof(uint64_t);}

	bool operator==( const RakNetGUID& right ) const;
	bool operator!=( const RakNetGUID& right ) const;
	bool operator > ( const RakNetGUID& right ) const;
	bool operator < ( const RakNetGUID& right ) const;
};

/// Index of an invalid SystemAddress
//const SystemAddress UNASSIGNED_SYSTEM_ADDRESS =
//{
//	0xFFFFFFFF, 0xFFFF
//};
#ifndef SWIG
const SystemAddress UNASSIGNED_SYSTEM_ADDRESS(0xFFFFFFFF, 0xFFFF);
const RakNetGUID UNASSIGNED_RAKNET_GUID((uint64_t)-1);
#endif
//{
//	{0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF,0xFFFFFFFF}
//	0xFFFFFFFFFFFFFFFF
//};


struct RAK_DLL_EXPORT AddressOrGUID
{
	RakNetGUID rakNetGuid;
	SystemAddress systemAddress;

	SystemIndex GetSystemIndex(void) const {if (rakNetGuid!=UNASSIGNED_RAKNET_GUID) return rakNetGuid.systemIndex; else return systemAddress.systemIndex;}
	bool IsUndefined(void) const {return rakNetGuid==UNASSIGNED_RAKNET_GUID && systemAddress==UNASSIGNED_SYSTEM_ADDRESS;}
	void SetUndefined(void) {rakNetGuid=UNASSIGNED_RAKNET_GUID; systemAddress=UNASSIGNED_SYSTEM_ADDRESS;}

	AddressOrGUID() {}
	AddressOrGUID( const AddressOrGUID& input )
	{
		rakNetGuid=input.rakNetGuid;
		systemAddress=input.systemAddress;
	}
	AddressOrGUID( const SystemAddress& input )
	{
		rakNetGuid=UNASSIGNED_RAKNET_GUID;
		systemAddress=input;
	}
	AddressOrGUID( const RakNetGUID& input )
	{
		rakNetGuid=input;
		systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
	}
	AddressOrGUID& operator = ( const AddressOrGUID& input )
	{
		rakNetGuid=input.rakNetGuid;
		systemAddress=input.systemAddress;
		return *this;
	}

	AddressOrGUID& operator = ( const SystemAddress& input )
	{
		rakNetGuid=UNASSIGNED_RAKNET_GUID;
		systemAddress=input;
		return *this;
	}

	AddressOrGUID& operator = ( const RakNetGUID& input )
	{
		rakNetGuid=input;
		systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
		return *this;
	}
};

typedef uint64_t NetworkID;

/// This represents a user message from another system.
struct Packet
{
	// This is now in the systemAddress struct and is used for lookups automatically
	/// Server only - this is the index into the player array that this systemAddress maps to
//	SystemIndex systemIndex;

	/// The system that send this packet.
	SystemAddress systemAddress;

	/// A unique identifier for the system that sent this packet, regardless of IP address (internal / external / remote system)
	/// Only valid once a connection has been established (ID_CONNECTION_REQUEST_ACCEPTED, or ID_NEW_INCOMING_CONNECTION)
	/// Until that time, will be UNASSIGNED_RAKNET_GUID
	RakNetGUID guid;

	/// The length of the data in bytes
	unsigned int length;

	/// The length of the data in bits
	BitSize_t bitSize;

	/// The data from the sender
	unsigned char* data;

	/// @internal
	/// Indicates whether to delete the data, or to simply delete the packet.
	bool deleteData;
};

///  Index of an unassigned player
const SystemIndex UNASSIGNED_PLAYER_INDEX = 65535;

/// Unassigned object ID
const NetworkID UNASSIGNED_NETWORK_ID = (uint64_t) -1;

const int PING_TIMES_ARRAY_SIZE = 5;

struct RAK_DLL_EXPORT uint24_t
{
	uint32_t val;

	uint24_t() {}
	inline operator uint32_t() { return val; }
	inline operator uint32_t() const { return val; }

	inline uint24_t(const uint24_t& a) {val=a.val;}
	inline uint24_t operator++() {++val; val&=0x00FFFFFF; return *this;}
	inline uint24_t operator--() {--val; val&=0x00FFFFFF; return *this;}
	inline uint24_t operator++(int) {uint24_t temp(val); ++val; val&=0x00FFFFFF; return temp;}
	inline uint24_t operator--(int) {uint24_t temp(val); --val; val&=0x00FFFFFF; return temp;}
	inline uint24_t operator&(const uint24_t& a) {return uint24_t(val&a.val);}
	inline uint24_t& operator=(const uint24_t& a) { val=a.val; return *this; }
	inline uint24_t& operator+=(const uint24_t& a) { val+=a.val; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator-=(const uint24_t& a) { val-=a.val; val&=0x00FFFFFF; return *this; }
	inline bool operator==( const uint24_t& right ) const {return val==right.val;}
	inline bool operator!=( const uint24_t& right ) const {return val!=right.val;}
	inline bool operator > ( const uint24_t& right ) const {return val>right.val;}
	inline bool operator < ( const uint24_t& right ) const {return val<right.val;}
	inline const uint24_t operator+( const uint24_t &other ) const { return uint24_t(val+other.val); }
	inline const uint24_t operator-( const uint24_t &other ) const { return uint24_t(val-other.val); }
	inline const uint24_t operator/( const uint24_t &other ) const { return uint24_t(val/other.val); }
	inline const uint24_t operator*( const uint24_t &other ) const { return uint24_t(val*other.val); }

	inline uint24_t(const uint32_t& a) {val=a; val&=0x00FFFFFF;}
	inline uint24_t operator&(const uint32_t& a) {return uint24_t(val&a);}
	inline uint24_t& operator=(const uint32_t& a) { val=a; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator+=(const uint32_t& a) { val+=a; val&=0x00FFFFFF; return *this; }
	inline uint24_t& operator-=(const uint32_t& a) { val-=a; val&=0x00FFFFFF; return *this; }
	inline bool operator==( const uint32_t& right ) const {return val==(right&0x00FFFFFF);}
	inline bool operator!=( const uint32_t& right ) const {return val!=(right&0x00FFFFFF);}
	inline bool operator > ( const uint32_t& right ) const {return val>(right&0x00FFFFFF);}
	inline bool operator < ( const uint32_t& right ) const {return val<(right&0x00FFFFFF);}
	inline const uint24_t operator+( const uint32_t &other ) const { return uint24_t(val+other); }
	inline const uint24_t operator-( const uint32_t &other ) const { return uint24_t(val-other); }
	inline const uint24_t operator/( const uint32_t &other ) const { return uint24_t(val/other); }
	inline const uint24_t operator*( const uint32_t &other ) const { return uint24_t(val*other); }
};

} // namespace RakNet

#endif
