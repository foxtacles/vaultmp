#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_RPC4Plugin==1

#include "RPC4Plugin.h"
#include "MessageIdentifiers.h"
#include "RakPeerInterface.h"
#include "PacketizedTCP.h"
#include "RakSleep.h"
#include "RakNetDefines.h"

using namespace RakNet;

STATIC_FACTORY_DEFINITIONS(RPC4,RPC4);

struct GlobalRegistration
{
	void ( *registerFunctionPointer ) ( RakNet::BitStream *userData, Packet *packet );
	void ( *registerBlockingFunctionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet );
	char functionName[RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH];
	MessageID messageId;
};
static GlobalRegistration globalRegistrationBuffer[RPC4_GLOBAL_REGISTRATION_MAX_FUNCTIONS];
static unsigned int globalRegistrationIndex=0;

RPC4GlobalRegistration::RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ))
{
	RakAssert(globalRegistrationIndex!=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTIONS);
	unsigned int i;
	for (i=0; uniqueID[i]; i++)
	{
		RakAssert(i<=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH-1);
		globalRegistrationBuffer[globalRegistrationIndex].functionName[i]=uniqueID[i];
	}
	globalRegistrationBuffer[globalRegistrationIndex].registerFunctionPointer=functionPointer;
	globalRegistrationBuffer[globalRegistrationIndex].registerBlockingFunctionPointer=0;
	globalRegistrationIndex++;
}
RPC4GlobalRegistration::RPC4GlobalRegistration(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ))
{
	RakAssert(globalRegistrationIndex!=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTIONS);
	unsigned int i;
	for (i=0; uniqueID[i]; i++)
	{
		RakAssert(i<=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH-1);
		globalRegistrationBuffer[globalRegistrationIndex].functionName[i]=uniqueID[i];
	}
	globalRegistrationBuffer[globalRegistrationIndex].registerFunctionPointer=0;
	globalRegistrationBuffer[globalRegistrationIndex].registerBlockingFunctionPointer=functionPointer;
	globalRegistrationIndex++;
}
RPC4GlobalRegistration::RPC4GlobalRegistration(const char* uniqueID, MessageID messageId)
{
	RakAssert(globalRegistrationIndex!=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTIONS);
	unsigned int i;
	for (i=0; uniqueID[i]; i++)
	{
		RakAssert(i<=RPC4_GLOBAL_REGISTRATION_MAX_FUNCTION_NAME_LENGTH-1);
		globalRegistrationBuffer[globalRegistrationIndex].functionName[i]=uniqueID[i];
	}
	globalRegistrationBuffer[globalRegistrationIndex].registerFunctionPointer=0;
	globalRegistrationBuffer[globalRegistrationIndex].registerBlockingFunctionPointer=0;
	globalRegistrationBuffer[globalRegistrationIndex].messageId=messageId;
	globalRegistrationIndex++;
}

enum RPC4Identifiers
{
	ID_RPC4_CALL,
	ID_RPC4_RETURN,
};

int RPC4::LocalCallbackComp(const MessageID &key, RPC4::LocalCallback* const &data )
{
	if (key < data->messageId)
		return -1;
	if (key > data->messageId)
		return 1;
	return 0;
}

RPC4::RPC4()
{
	gotBlockingReturnValue=false;
}
RPC4::~RPC4()
{
	unsigned int i;
	for (i=0; i < localCallbacks.Size(); i++)
	{
		RakNet::OP_DELETE(localCallbacks[i],_FILE_AND_LINE_);
	}
}
bool RPC4::RegisterFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, Packet *packet ))
{
	DataStructures::HashIndex skhi = registeredNonblockingFunctions.GetIndexOf(uniqueID);
	if (skhi.IsInvalid()==false)
		return false;

	registeredNonblockingFunctions.Push(uniqueID,functionPointer,_FILE_AND_LINE_);
	return true;
}
bool RPC4::RegisterBlockingFunction(const char* uniqueID, void ( *functionPointer ) ( RakNet::BitStream *userData, RakNet::BitStream *returnData, Packet *packet ))
{
	DataStructures::HashIndex skhi = registeredBlockingFunctions.GetIndexOf(uniqueID);
	if (skhi.IsInvalid()==false)
		return false;

	registeredBlockingFunctions.Push(uniqueID,functionPointer,_FILE_AND_LINE_);
	return true;
}
void RPC4::RegisterLocalCallback(const char* uniqueID, MessageID messageId)
{
	bool objectExists;
	unsigned int index;
	LocalCallback *lc;
	RakNet::RakString str;
	str=uniqueID;
	index = localCallbacks.GetIndexFromKey(messageId,&objectExists);
	if (objectExists)
	{
		lc = localCallbacks[index];
		index = lc->functions.GetIndexFromKey(str,&objectExists);
		if (objectExists==false)
			lc->functions.InsertAtIndex(str,index,_FILE_AND_LINE_);
	}
	else
	{
		lc = RakNet::OP_NEW<LocalCallback>(_FILE_AND_LINE_);
		lc->messageId=messageId;
		lc->functions.Insert(str,str,false,_FILE_AND_LINE_);
		localCallbacks.InsertAtIndex(lc,index,_FILE_AND_LINE_);
	}
}
bool RPC4::UnregisterFunction(const char* uniqueID)
{
	void ( *f ) ( RakNet::BitStream *, Packet * );
	return registeredNonblockingFunctions.Pop(f,uniqueID,_FILE_AND_LINE_);
}
bool RPC4::UnregisterBlockingFunction(const char* uniqueID)
{
	void ( *f ) ( RakNet::BitStream *, RakNet::BitStream *,Packet * );
	return registeredBlockingFunctions.Pop(f,uniqueID,_FILE_AND_LINE_);
}
bool RPC4::UnregisterLocalCallback(const char* uniqueID, MessageID messageId)
{
	bool objectExists;
	unsigned int index, index2;
	LocalCallback *lc;
	RakNet::RakString str;
	str=uniqueID;
	index = localCallbacks.GetIndexFromKey(messageId,&objectExists);
	if (objectExists)
	{
		lc = localCallbacks[index];
		index2 = lc->functions.GetIndexFromKey(str,&objectExists);
		if (objectExists)
		{
			lc->functions.RemoveAtIndex(index2);
			if (lc->functions.Size()==0)
			{
				RakNet::OP_DELETE(lc,_FILE_AND_LINE_);
				localCallbacks.RemoveAtIndex(index);
				return true;
			}
		}
	}
	return false;
}
void RPC4::CallLoopback( const char* uniqueID, RakNet::BitStream * bitStream )
{
	Packet *p=0;

	DataStructures::HashIndex skhi = registeredNonblockingFunctions.GetIndexOf(uniqueID);

	if (skhi.IsInvalid()==true)
	{
		if (rakPeerInterface) 
			p=AllocatePacketUnified(sizeof(MessageID)+sizeof(unsigned char)+(unsigned int) strlen(uniqueID)+1);
#if _RAKNET_SUPPORT_PacketizedTCP==1 && _RAKNET_SUPPORT_TCPInterface==1
		else
			p=packetizedTCP->AllocatePacket(sizeof(MessageID)+sizeof(unsigned char)+(unsigned int) strlen(uniqueID)+1);
#endif

		if (rakPeerInterface)
			p->guid=rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
#if _RAKNET_SUPPORT_PacketizedTCP==1 && _RAKNET_SUPPORT_TCPInterface==1
		else
			p->guid=UNASSIGNED_RAKNET_GUID;
#endif

		p->systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
		p->systemAddress.systemIndex=(SystemIndex)-1;
		p->data[0]=ID_RPC_REMOTE_ERROR;
		p->data[1]=RPC_ERROR_FUNCTION_NOT_REGISTERED;
		strcpy((char*) p->data+2, uniqueID);
		
		PushBackPacketUnified(p,false);

		return;
	}

	RakNet::BitStream out;
	out.Write((MessageID) ID_RPC_PLUGIN);
	out.Write((MessageID) ID_RPC4_CALL);
	out.WriteCompressed(uniqueID);
	out.Write(false); // nonblocking
	if (bitStream)
	{
		bitStream->ResetReadPointer();
        out.AlignWriteToByteBoundary();
		out.Write(bitStream);
	}
	if (rakPeerInterface) 
		p=AllocatePacketUnified(out.GetNumberOfBytesUsed());
#if _RAKNET_SUPPORT_PacketizedTCP==1 && _RAKNET_SUPPORT_TCPInterface==1
	else
		p=packetizedTCP->AllocatePacket(out.GetNumberOfBytesUsed());
#endif

	if (rakPeerInterface)
		p->guid=rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
#if _RAKNET_SUPPORT_PacketizedTCP==1 && _RAKNET_SUPPORT_TCPInterface==1
	else
		p->guid=UNASSIGNED_RAKNET_GUID;
#endif
	p->systemAddress=UNASSIGNED_SYSTEM_ADDRESS;
	p->systemAddress.systemIndex=(SystemIndex)-1;
	memcpy(p->data,out.GetData(),out.GetNumberOfBytesUsed());
	PushBackPacketUnified(p,false);
	return;
}
void RPC4::Call( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, bool broadcast )
{
	RakNet::BitStream out;
	out.Write((MessageID) ID_RPC_PLUGIN);
	out.Write((MessageID) ID_RPC4_CALL);
	out.WriteCompressed(uniqueID);
	out.Write(false); // Nonblocking
	if (bitStream)
	{
		bitStream->ResetReadPointer();
        out.AlignWriteToByteBoundary();
		out.Write(bitStream);
	}
	SendUnified(&out,priority,reliability,orderingChannel,systemIdentifier,broadcast);
}
bool RPC4::CallBlocking( const char* uniqueID, RakNet::BitStream * bitStream, PacketPriority priority, PacketReliability reliability, char orderingChannel, const AddressOrGUID systemIdentifier, RakNet::BitStream *returnData )
{
	RakNet::BitStream out;
	out.Write((MessageID) ID_RPC_PLUGIN);
	out.Write((MessageID) ID_RPC4_CALL);
	out.WriteCompressed(uniqueID);
	out.Write(true); // Blocking
	if (bitStream)
	{
		bitStream->ResetReadPointer();
        out.AlignWriteToByteBoundary();
		out.Write(bitStream);
	}
	RakAssert(returnData);
	RakAssert(rakPeerInterface);
	ConnectionState cs;
	cs = rakPeerInterface->GetConnectionState(systemIdentifier);
	if (cs!=IS_CONNECTED && cs!=IS_LOOPBACK)
		return false;

	SendUnified(&out,priority,reliability,orderingChannel,systemIdentifier,false);

	returnData->Reset();
	blockingReturnValue.Reset();
	gotBlockingReturnValue=false;
	Packet *packet;
	DataStructures::Queue<Packet*> packetQueue;
	while (gotBlockingReturnValue==false)
	{
		// TODO - block, filter until gotBlockingReturnValue==true or ID_CONNECTION_LOST or ID_DISCONNECTION_NOTIFICXATION or ID_RPC_REMOTE_ERROR/RPC_ERROR_FUNCTION_NOT_REGISTERED
		RakSleep(30);

		packet=rakPeerInterface->Receive();

		if (packet)
		{
			if (
				(packet->data[0]==ID_CONNECTION_LOST || packet->data[0]==ID_DISCONNECTION_NOTIFICATION) &&
				((systemIdentifier.rakNetGuid!=UNASSIGNED_RAKNET_GUID && packet->guid==systemIdentifier.rakNetGuid) ||
				(systemIdentifier.systemAddress!=UNASSIGNED_SYSTEM_ADDRESS && packet->systemAddress==systemIdentifier.systemAddress))
				)
			{
				// Push back to head in reverse order
				rakPeerInterface->PushBackPacket(packet,true);
				while (packetQueue.Size())
					rakPeerInterface->PushBackPacket(packetQueue.Pop(),true);
				return false;
			}
			else if (packet->data[0]==ID_RPC_REMOTE_ERROR && packet->data[1]==RPC_ERROR_FUNCTION_NOT_REGISTERED)
			{
				RakNet::RakString functionName;
				RakNet::BitStream bsIn(packet->data,packet->length,false);
				bsIn.IgnoreBytes(2);
				bsIn.Read(functionName);
				if (functionName==uniqueID)
				{
					// Push back to head in reverse order
					rakPeerInterface->PushBackPacket(packet,true);
					while (packetQueue.Size())
						rakPeerInterface->PushBackPacket(packetQueue.Pop(),true);
					return false;
				}
				else
				{
					packetQueue.PushAtHead(packet,0,_FILE_AND_LINE_);
				}
			}
			else
			{
				packetQueue.PushAtHead(packet,0,_FILE_AND_LINE_);
			}
		}
	}

	returnData->Read(blockingReturnValue);
	return true;
}
void RPC4::OnAttach(void)
{
	unsigned int i;
	for (i=0; i < globalRegistrationIndex; i++)
	{
		if (globalRegistrationBuffer[i].registerFunctionPointer)
			RegisterFunction(globalRegistrationBuffer[i].functionName, globalRegistrationBuffer[i].registerFunctionPointer);
		else if (globalRegistrationBuffer[i].registerBlockingFunctionPointer)
			RegisterBlockingFunction(globalRegistrationBuffer[i].functionName, globalRegistrationBuffer[i].registerBlockingFunctionPointer);
		else
			RegisterLocalCallback(globalRegistrationBuffer[i].functionName, globalRegistrationBuffer[i].messageId);
	}
}
PluginReceiveResult RPC4::OnReceive(Packet *packet)
{
	if (packet->data[0]==ID_RPC_PLUGIN)
	{
		RakNet::BitStream bsIn(packet->data,packet->length,false);
		bsIn.IgnoreBytes(2);

		if (packet->data[1]==ID_RPC4_CALL)
		{
			RakNet::RakString functionName;
			bsIn.ReadCompressed(functionName);
			bool isBlocking;
			bsIn.Read(isBlocking);
			if (isBlocking==false)
			{
				DataStructures::HashIndex skhi = registeredNonblockingFunctions.GetIndexOf(functionName.C_String());
				if (skhi.IsInvalid())
				{
					RakNet::BitStream bsOut;
					bsOut.Write((unsigned char) ID_RPC_REMOTE_ERROR);
					bsOut.Write((unsigned char) RPC_ERROR_FUNCTION_NOT_REGISTERED);
					bsOut.Write(functionName.C_String(),(unsigned int) functionName.GetLength()+1);
					SendUnified(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}

				void ( *fp ) ( RakNet::BitStream *, Packet * );
				fp = registeredNonblockingFunctions.ItemAtIndex(skhi);
                bsIn.AlignReadToByteBoundary();
				fp(&bsIn,packet);
			}
			else
			{
				DataStructures::HashIndex skhi = registeredBlockingFunctions.GetIndexOf(functionName.C_String());
				if (skhi.IsInvalid())
				{
					RakNet::BitStream bsOut;
					bsOut.Write((unsigned char) ID_RPC_REMOTE_ERROR);
					bsOut.Write((unsigned char) RPC_ERROR_FUNCTION_NOT_REGISTERED);
					bsOut.Write(functionName.C_String(),(unsigned int) functionName.GetLength()+1);
					SendUnified(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
					return RR_STOP_PROCESSING_AND_DEALLOCATE;
				}

				void ( *fp ) ( RakNet::BitStream *, RakNet::BitStream *, Packet * );
				fp = registeredBlockingFunctions.ItemAtIndex(skhi);
				RakNet::BitStream returnData;
                bsIn.AlignReadToByteBoundary();
				fp(&bsIn, &returnData, packet);

				RakNet::BitStream out;
				out.Write((MessageID) ID_RPC_PLUGIN);
				out.Write((MessageID) ID_RPC4_RETURN);
				returnData.ResetReadPointer();
                out.AlignWriteToByteBoundary();
				out.Write(returnData);
				SendUnified(&out,IMMEDIATE_PRIORITY,RELIABLE_ORDERED,0,packet->systemAddress,false);
			}
		}
		else
		{
			RakAssert(packet->data[1]==ID_RPC4_RETURN);
			blockingReturnValue.Reset();
			blockingReturnValue.Read(bsIn);
			gotBlockingReturnValue=true;
		}
		
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}

	bool objectExists;
	unsigned int index, index2;
	index = localCallbacks.GetIndexFromKey(packet->data[0],&objectExists);
	if (objectExists)
	{
		LocalCallback *lc;
		lc = localCallbacks[index];
		for (index2=0; index2 < lc->functions.Size(); index2++)
		{
			RakNet::BitStream bsIn(packet->data, packet->length, false);

			DataStructures::HashIndex skhi = registeredNonblockingFunctions.GetIndexOf(lc->functions[index2].C_String());
			if (skhi.IsInvalid()==false)
			{
				void ( *fp ) ( RakNet::BitStream *, Packet * );
				fp = registeredNonblockingFunctions.ItemAtIndex(skhi);
                bsIn.AlignReadToByteBoundary();
				fp(&bsIn,packet);
			}		
		}
	}

	return RR_CONTINUE_PROCESSING;
}

#endif // _RAKNET_SUPPORT_*
