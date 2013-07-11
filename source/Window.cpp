#include "Window.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

Window::Window() : Reference(0x00000000, 0x00000000)
{
	initialize();
}

Window::Window(const pDefault* packet) : Reference(0x00000000, 0x00000000)
{
	initialize();

	NetworkID id;

	PacketFactory::Access<pTypes::ID_WINDOW_NEW>(packet, id, parent, label, pos, size, locked);

	this->SetNetworkID(id);
}

Window::Window(pPacket&& packet) : Window(packet.get())
{

}

Window::~Window()
{

}

void Window::initialize()
{
	parent = 0;
	locked = false;
}

pPacket Window::toPacket() const
{
	pPacket packet = PacketFactory::Create<pTypes::ID_WINDOW_NEW>(const_cast<Window*>(this)->GetNetworkID(), parent, label, pos, size, locked);

	return packet;
}
