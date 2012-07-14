#include "ESMLoader.h"

using namespace ESMLoader;

extern vector<RECORD_NPC> NPCList;
extern vector<RECORD_WEAP> WeaponList;
extern vector<RECORD_AMMO> AmmoList;

int main()
{
	Load("C:\\Program Files (x86)\\Bethesda Softworks\\Fallout New Vegas\\Data\\FalloutNV.esm");

	cin.get();

	return 0;
}