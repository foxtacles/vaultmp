#include <vaultmp>

stock Pickup_Register(const func{}, const name{}) {
	MakePublic(func, name, "ll");
	return CallPublic("Pickup::Register", name);
}
