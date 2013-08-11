#include <vaultmp>

stock Pickup_Register(const func{}, const name{}) {
	MakePublic(func, name, "ll");
        CallPublic("Pickup::Register", name);
}
