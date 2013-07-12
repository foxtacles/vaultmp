#include "Edit.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

Edit::Edit() : Window()
{
	initialize();
}

Edit::Edit(const pDefault* packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_EDIT_NEW>(packet, text);
}

Edit::Edit(pPacket&& packet) : Edit(packet.get())
{

}

Edit::~Edit()
{

}

void Edit::initialize()
{

}

pPacket Edit::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_EDIT_NEW>(pWindowNew, text);

	return packet;
}
