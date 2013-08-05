#include "Edit.h"

using namespace std;
using namespace RakNet;

Edit::Edit() : Window(), length(DEFAULT_LENGTH), validation(DEFAULT_VALIDATION)
{
	initialize();
}

Edit::Edit(const pDefault* packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_EDIT_NEW>(packet, length, validation);
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
	pPacket packet = PacketFactory::Create<pTypes::ID_EDIT_NEW>(pWindowNew, length, validation);

	return packet;
}
