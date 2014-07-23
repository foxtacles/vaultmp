#ifndef __SENDTO_THREAD
#define __SENDTO_THREAD

#include "RakNetDefines.h"

#ifdef USE_THREADED_SEND

#include "InternalPacket.h"
#include "SocketLayer.h"
#include "DS_ThreadsafeAllocatingQueue.h"
#include "ThreadPool.h"

namespace RakNet
{
class SendToThread
{
public:
	SendToThread();
	~SendToThread();

	struct SendToThreadBlock
	{
		SOCKET s;
		SystemAddress systemAddress;
		unsigned short remotePortRakNetWasStartedOn_PS3;
		unsigned int extraSocketOptions;
		char data[MAXIMUM_MTU_SIZE];
		unsigned short dataWriteOffset;
	};

	static SendToThreadBlock* AllocateBlock(void);
	static void ProcessBlock(SendToThreadBlock* threadedSend);

	static void AddRef(void);
	static void Deref(void);
	static DataStructures::ThreadsafeAllocatingQueue<SendToThreadBlock> objectQueue;
protected:
	static int refCount;
	static ThreadPool<SendToThreadBlock*,SendToThreadBlock*> threadPool;

};
}


#endif

#endif
