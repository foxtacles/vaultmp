#include "Button.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

Button::Button() : Window()
{
	initialize();
}

Button::Button(const pDefault* packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_BUTTON_NEW>(packet, text);
}

Button::Button(pPacket&& packet) : Button(packet.get())
{

}

Button::~Button()
{

}

void Button::initialize()
{

}

pPacket Button::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_BUTTON_NEW>(pWindowNew, text);

	return packet;
}
