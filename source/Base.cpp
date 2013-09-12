#include "Base.hpp"
#include "Network.hpp"

using namespace std;
using namespace RakNet;

Base::Base()
{
	this->SetNetworkIDManager(Network::Manager());
}

Base::Base(const pPacket& packet) : Base()
{
	NetworkID id;

	PacketFactory::Access<pTypes::ID_BASE_NEW>(packet, id);

	this->SetNetworkID(id);
}

Base::~Base() noexcept {}

pPacket Base::toPacket() const
{
	pPacket packet = PacketFactory::Create<pTypes::ID_BASE_NEW>(const_cast<Base*>(this)->GetNetworkID());

	return packet;
}
