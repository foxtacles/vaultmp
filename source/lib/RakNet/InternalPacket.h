/// \file
/// \brief \b [Internal] A class which stores a user message, and all information associated with sending and receiving that message.
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.jenkinssoftware.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#ifndef __INTERNAL_PACKET_H
#define __INTERNAL_PACKET_H

#include "PacketPriority.h"
#include "RakNetTypes.h"
#include "RakMemoryOverride.h"
#include "RakNetDefines.h"
#include "NativeTypes.h"
#include "RakNetDefines.h"
#if USE_SLIDING_WINDOW_CONGESTION_CONTROL!=1
#include "CCRakNetUDT.h"
#else
#include "CCRakNetSlidingWindow.h"
#endif

namespace RakNet {

typedef uint16_t SplitPacketIdType;
typedef uint32_t SplitPacketIndexType;

/// This is the counter used for holding packet numbers, so we can detect duplicate packets.  It should be large enough that if the variables
/// Internally assumed to be 4 bytes, but written as 3 bytes in ReliabilityLayer::WriteToBitStreamFromInternalPacket
typedef uint24_t MessageNumberType;

/// This is the counter used for holding ordered packet numbers, so we can detect out-of-order packets.  It should be large enough that if the variables
/// were to wrap, the newly wrapped values would no longer be in use.  Warning: Too large of a value wastes bandwidth!
typedef MessageNumberType OrderingIndexType;

typedef RakNet::TimeUS RemoteSystemTimeType;

struct InternalPacketFixedSizeTransmissionHeader
{
	/// A unique numerical identifier given to this user message. Used to identify reliable messages on the network
	MessageNumberType reliableMessageNumber;
	///The ID used as identification for ordering messages. Also included in sequenced messages
	OrderingIndexType orderingIndex;
	// Used only with sequenced messages
	OrderingIndexType sequencingIndex;
	///What ordering channel this packet is on, if the reliability type uses ordering channels
	unsigned char orderingChannel;
	///The ID of the split packet, if we have split packets.  This is the maximum number of split messages we can send simultaneously per connection.
	SplitPacketIdType splitPacketId;
	///If this is a split packet, the index into the array of subsplit packets
	SplitPacketIndexType splitPacketIndex;
	///The size of the array of subsplit packets
	SplitPacketIndexType splitPacketCount;;
	///How many bits long the data is
	BitSize_t dataBitLength;
	///What type of reliability algorithm to use with this packet
	PacketReliability reliability;
	// Not endian safe
	// unsigned char priority : 3;
	// unsigned char reliability : 5;
};

/// Used in InternalPacket when pointing to sharedDataBlock, rather than allocating itself
struct InternalPacketRefCountedData
{
	unsigned char *sharedDataBlock;
	unsigned int refCount;
};

/// Holds a user message, and related information
/// Don't use a constructor or destructor, due to the memory pool I am using
struct InternalPacket : public InternalPacketFixedSizeTransmissionHeader
{
	/// Identifies the order in which this number was sent. Used locally
	MessageNumberType messageInternalOrder;
	/// Has this message number been assigned yet?  We don't assign until the message is actually sent.
	/// This fixes a bug where pre-determining message numbers and then sending a message on a different channel creates a huge gap.
	/// This causes performance problems and causes those messages to timeout.
	bool messageNumberAssigned;
	/// Was this packet number used this update to track windowing drops or increases?  Each packet number is only used once per update.
//	bool allowWindowUpdate;
	///When this packet was created
	RakNet::TimeUS creationTime;
	///The resendNext time to take action on this packet
	RakNet::TimeUS nextActionTime;
	// For debugging
	RakNet::TimeUS retransmissionTime;
	// Size of the header when encoded into a bitstream
	BitSize_t headerLength;
	/// Buffer is a pointer to the actual data, assuming this packet has data at all
	unsigned char *data;
	/// How to alloc and delete the data member
	enum AllocationScheme
	{
		/// Data is allocated using rakMalloc. Just free it
		NORMAL,

		/// data points to a larger block of data, where the larger block is reference counted. internalPacketRefCountedData is used in this case
		REF_COUNTED,
	
		/// If allocation scheme is STACK, data points to stackData and should not be deallocated
		/// This is only used when sending. Received packets are deallocated in RakPeer
		STACK
	} allocationScheme;
	InternalPacketRefCountedData *refCountedData;
	/// How many attempts we made at sending this message
	unsigned char timesSent;
	/// The priority level of this packet
	PacketPriority priority;
	/// If the reliability type requires a receipt, then return this number with it
	uint32_t sendReceiptSerial;

	// Used for the resend queue
	// Linked list implementation so I can remove from the list via a pointer, without finding it in the list
	InternalPacket *resendPrev, *resendNext,*unreliablePrev,*unreliableNext;

	unsigned char stackData[128];
};

} // namespace RakNet

#endif

