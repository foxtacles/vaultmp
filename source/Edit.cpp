#include "Edit.hpp"

using namespace std;
using namespace RakNet;

Edit::Edit() : Window(), length(DEFAULT_LENGTH), validation(DEFAULT_VALIDATION)
{
	initialize();
}

Edit::Edit(const pPacket& packet) : Window(PacketFactory::Pop<pPacket>(packet))
{
	initialize();

	PacketFactory::Access<pTypes::ID_EDIT_NEW>(packet, length, validation);
}

Edit::~Edit() noexcept {}

void Edit::initialize()
{

}

pPacket Edit::toPacket() const
{
	pPacket pWindowNew = Window::toPacket();
	pPacket packet = PacketFactory::Create<pTypes::ID_EDIT_NEW>(pWindowNew, length, validation);

	return packet;
}
