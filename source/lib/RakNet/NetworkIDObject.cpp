/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.


#include "NetworkIDObject.h"
#include "NetworkIDManager.h"
#include "RakAssert.h"
#include "RakAlloca.h"

using namespace RakNet;

NetworkIDObject::NetworkIDObject()
{
	networkID=UNASSIGNED_NETWORK_ID;
	parent=0;
	networkIDManager=0;
	nextInstanceForNetworkIDManager=0;
}
NetworkIDObject::~NetworkIDObject()
{
	if (networkIDManager)
		networkIDManager->StopTrackingNetworkIDObject(this);
}
void NetworkIDObject::SetNetworkIDManager( NetworkIDManager *manager)
{
	if (manager==networkIDManager)
		return;

	if (networkIDManager)
		networkIDManager->StopTrackingNetworkIDObject(this);

	networkIDManager=manager;
	if (networkIDManager==0)
	{
		networkID = UNASSIGNED_NETWORK_ID;
		return;
	}
	
	if (networkID == UNASSIGNED_NETWORK_ID)
	{
		// Prior ID not set
		networkID = networkIDManager->GetNewNetworkID();
	}

	networkIDManager->TrackNetworkIDObject(this);
}
NetworkIDManager * NetworkIDObject::GetNetworkIDManager( void ) const
{
	return networkIDManager;
}
NetworkID NetworkIDObject::GetNetworkID( void )
{
	return networkID;
}
void NetworkIDObject::SetNetworkID( NetworkID id )
{
	if (networkID==id)
		return;

	if ( id == UNASSIGNED_NETWORK_ID )
	{
		SetNetworkIDManager(0);
		return;
	}

	if ( networkIDManager )
		networkIDManager->StopTrackingNetworkIDObject(this);

	networkID = id;

	if (networkIDManager)
		networkIDManager->TrackNetworkIDObject(this);
}
void NetworkIDObject::SetParent( void *_parent )
{
	parent=_parent;
}
void* NetworkIDObject::GetParent( void ) const
{
	return parent;
}
