#include "Text.h"
#include "PacketFactory.h"
#include "GameFactory.h"

using namespace std;
using namespace RakNet;

Text::Text() : Window()
{
	initialize();
}

Text::Text(const pDefault* packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_TEXT_NEW>(packet, text);
}

Text::Text(pPacket&& packet) : Text(packet.get())
{

}

Text::~Text()
{

}

void Text::initialize()
{

}

pPacket Text::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_TEXT_NEW>(pWindowNew, text);

	return packet;
}
