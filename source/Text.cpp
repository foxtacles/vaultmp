#include "Text.hpp"

using namespace std;
using namespace RakNet;

Text::Text() : Window()
{
	initialize();
}

Text::Text(const pPacket& packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_TEXT_NEW>(packet);
}

Text::~Text() noexcept {}

void Text::initialize()
{

}

pPacket Text::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_TEXT_NEW>(pWindowNew);

	return packet;
}
