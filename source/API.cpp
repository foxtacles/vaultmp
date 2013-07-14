#include "API.h"

using namespace std;
using namespace Values;

API::CommandQueue API::queue;

API::ValueMap API::axis = {
		{"X", Axis_X},
		{"Y", Axis_Y},
		{"Z", Axis_Z},
	};

API::ValueMap API::anims = {
		{"Idle", AnimGroup_Idle},
		{"DynamicIdle", AnimGroup_DynamicIdle},
		{"SpecialIdle", AnimGroup_SpecialIdle},
		{"Forward", AnimGroup_Forward},
		{"Backward", AnimGroup_Backward},
		{"Left", AnimGroup_Left},
		{"Right", AnimGroup_Right},
		{"FastForward", AnimGroup_FastForward},
		{"FastBackward", AnimGroup_FastBackward},
		{"FastLeft", AnimGroup_FastLeft},
		{"FastRight", AnimGroup_FastRight},
		{"DodgeForward", AnimGroup_DodgeForward},
		{"DodgeBack", AnimGroup_DodgeBack},
		{"DodgeLeft", AnimGroup_DodgeLeft},
		{"DodgeRight", AnimGroup_DodgeRight},
		{"TurnLeft", AnimGroup_TurnLeft},
		{"TurnRight", AnimGroup_TurnRight},
		{"Aim", AnimGroup_Aim},
		{"AimUp", AnimGroup_AimUp},
		{"AimDown", AnimGroup_AimDown},
		{"AimIS", AnimGroup_AimIS},
		{"AimISUp", AnimGroup_AimISUp},
		{"AimISDown", AnimGroup_AimISDown},
		{"AttackLeft", AnimGroup_AttackLeft},
		{"AttackLeftUp", AnimGroup_AttackLeftUp},
		{"AttackLeftDown", AnimGroup_AttackLeftDown},
		{"AttackLeftIS", AnimGroup_AttackLeftIS},
		{"AttackLeftISUp", AnimGroup_AttackLeftISUp},
		{"AttackLeftISDown", AnimGroup_AttackLeftISDown},
		{"AttackRight", AnimGroup_AttackRight},
		{"AttackRightUp", AnimGroup_AttackRightUp},
		{"AttackRightDown", AnimGroup_AttackRightDown},
		{"AttackRightIS", AnimGroup_AttackRightIS},
		{"AttackRightISUp", AnimGroup_AttackRightISUp},
		{"AttackRightISDown", AnimGroup_AttackRightISDown},
		{"Attack3", AnimGroup_Attack3},
		{"Attack3Up", AnimGroup_Attack3Up},
		{"Attack3Down", AnimGroup_Attack3Down},
		{"Attack3IS", AnimGroup_Attack3IS},
		{"Attack3ISUp", AnimGroup_Attack3ISUp},
		{"Attack3ISDown", AnimGroup_Attack3ISDown},
		{"Attack4", AnimGroup_Attack4},
		{"Attack4Up", AnimGroup_Attack4Up},
		{"Attack4Down", AnimGroup_Attack4Down},
		{"Attack4IS", AnimGroup_Attack4IS},
		{"Attack4ISUp", AnimGroup_Attack4ISUp},
		{"Attack4ISDown", AnimGroup_Attack4ISDown},
		{"Attack5", AnimGroup_Attack5},
		{"Attack5Up", AnimGroup_Attack5Up},
		{"Attack5Down", AnimGroup_Attack5Down},
		{"Attack5IS", AnimGroup_Attack5IS},
		{"Attack5ISUp", AnimGroup_Attack5ISUp},
		{"Attack5ISDown", AnimGroup_Attack5ISDown},
		{"Attack6", AnimGroup_Attack6},
		{"Attack6Up", AnimGroup_Attack6Up},
		{"Attack6Down", AnimGroup_Attack6Down},
		{"Attack6IS", AnimGroup_Attack6IS},
		{"Attack6ISUp", AnimGroup_Attack6ISUp},
		{"Attack6ISDown", AnimGroup_Attack6ISDown},
		{"Attack7", AnimGroup_Attack7},
		{"Attack7Up", AnimGroup_Attack7Up},
		{"Attack7Down", AnimGroup_Attack7Down},
		{"Attack7IS", AnimGroup_Attack7IS},
		{"Attack7ISUp", AnimGroup_Attack7ISUp},
		{"Attack7ISDown", AnimGroup_Attack7ISDown},
		{"Attack8", AnimGroup_Attack8},
		{"Attack8Up", AnimGroup_Attack8Up},
		{"Attack8Down", AnimGroup_Attack8Down},
		{"Attack8IS", AnimGroup_Attack8IS},
		{"Attack8ISUp", AnimGroup_Attack8ISUp},
		{"Attack8ISDown", AnimGroup_Attack8ISDown},
		{"AttackLoop", AnimGroup_AttackLoop},
		{"AttackLoopUp", AnimGroup_AttackLoopUp},
		{"AttackLoopDown", AnimGroup_AttackLoopDown},
		{"AttackLoopIS", AnimGroup_AttackLoopIS},
		{"AttackLoopISUp", AnimGroup_AttackLoopISUp},
		{"AttackLoopISDown", AnimGroup_AttackLoopISDown},
		{"AttackSpin", AnimGroup_AttackSpin},
		{"AttackSpinUp", AnimGroup_AttackSpinUp},
		{"AttackSpinDown", AnimGroup_AttackSpinDown},
		{"AttackSpinIS", AnimGroup_AttackSpinIS},
		{"AttackSpinISUp", AnimGroup_AttackSpinISUp},
		{"AttackSpinISDown", AnimGroup_AttackSpinISDown},
		{"AttackSpin2", AnimGroup_AttackSpin2},
		{"AttackSpin2Up", AnimGroup_AttackSpin2Up},
		{"AttackSpin2Down", AnimGroup_AttackSpin2Down},
		{"AttackSpin2IS", AnimGroup_AttackSpin2IS},
		{"AttackSpin2ISUp", AnimGroup_AttackSpin2ISUp},
		{"AttackSpin2ISDown", AnimGroup_AttackSpin2ISDown},
		{"AttackPower", AnimGroup_AttackPower},
		{"AttackForwardPower", AnimGroup_AttackForwardPower},
		{"AttackBackPower", AnimGroup_AttackBackPower},
		{"AttackLeftPower", AnimGroup_AttackLeftPower},
		{"AttackRightPower", AnimGroup_AttackRightPower},
		{"Holster", AnimGroup_Holster},
		{"Equip", AnimGroup_Equip},
		{"Unequip", AnimGroup_Unequip},

		{"JumpStart", AnimGroup_JumpStart},
		{"JumpLoop", AnimGroup_JumpLoop},
		{"JumpLand", AnimGroup_JumpLand},
		{"JumpLoopForward", AnimGroup_JumpLoopForward},
		{"JumpLoopBackward", AnimGroup_JumpLoopBackward},
		{"JumpLoopLeft", AnimGroup_JumpLoopLeft},
		{"JumpLoopRight", AnimGroup_JumpLoopRight},
		{"JumpLandForward", AnimGroup_JumpLandForward},
		{"JumpLandBackward", AnimGroup_JumpLandBackward},
		{"JumpLandLeft", AnimGroup_JumpLandLeft},
		{"JumpLandRight", AnimGroup_JumpLandRight},
		{"BlockIdle", AnimGroup_BlockIdle},
		{"BlockHit", AnimGroup_BlockHit},
		{"ReloadA", AnimGroup_ReloadA},
		{"ReloadB", AnimGroup_ReloadB},
		{"ReloadC", AnimGroup_ReloadC},
		{"ReloadD", AnimGroup_ReloadD},
		{"ReloadE", AnimGroup_ReloadE},
		{"ReloadF", AnimGroup_ReloadF},
		{"ReloadG", AnimGroup_ReloadG},
		{"ReloadH", AnimGroup_ReloadH},
		{"ReloadI", AnimGroup_ReloadI},
		{"ReloadJ", AnimGroup_ReloadJ},
		{"ReloadK", AnimGroup_ReloadK},
	};

API::ValueMap API::values = {
		{"Aggression", ActorVal_Aggression},
		{"Confidence", ActorVal_Confidence},
		{"Energy", ActorVal_Energy},
		{"Responsibility", ActorVal_Responsibility},
		{"Mood", ActorVal_Mood},
		{"Strength", ActorVal_Strength},
		{"Perception", ActorVal_Perception},
		{"Endurance", ActorVal_Endurance},
		{"Charisma", ActorVal_Charisma},
		{"Intelligence", ActorVal_Intelligence},
		{"Agility", ActorVal_Agility},
		{"Luck", ActorVal_Luck},
		{"ActionPoints", ActorVal_ActionPoints},
		{"CarryWeight", ActorVal_CarryWeight},
		{"CritChance", ActorVal_CritChance},
		{"HealRate", ActorVal_HealRate},
		{"Health", ActorVal_Health},
		{"MeleeDamage", ActorVal_MeleeDamage},
		{"DamageResist", ActorVal_DamageResistance},
		{"PoisonResist", ActorVal_PoisonResistance},
		{"RadResist", ActorVal_RadResistance},
		{"SpeedMult", ActorVal_SpeedMultiplier},
		{"Fatigue", ActorVal_Fatigue},
		{"Karma", ActorVal_Karma},
		{"XP", ActorVal_XP},
		{"PerceptionCondition", ActorVal_Head},
		{"EnduranceCondition", ActorVal_Torso},
		{"LeftAttackCondition", ActorVal_LeftArm},
		{"RightAttackCondition", ActorVal_RightArm},
		{"LeftMobilityCondition", ActorVal_LeftLeg},
		{"RightMobilityCondition", ActorVal_RightLeg},
		{"BrainCondition", ActorVal_Brain},
		{"Barter", ActorVal_Barter},
		{"BigGuns", ActorVal_BigGuns},
		{"EnergyWeapons", ActorVal_EnergyWeapons},
		{"Explosives", ActorVal_Explosives},
		{"Lockpick", ActorVal_Lockpick},
		{"Medicine", ActorVal_Medicine},
		{"MeleeWeapons", ActorVal_MeleeWeapons},
		{"Repair", ActorVal_Repair},
		{"Science", ActorVal_Science},
		{"SmallGuns", ActorVal_SmallGuns},
		{"Sneak", ActorVal_Sneak},
		{"Speech", ActorVal_Speech},
		//{"Throwing", ActorVal_Throwing},
		{"Unarmed", ActorVal_Unarmed},
		{"InventoryWeight", ActorVal_InventoryWeight},
		{"Paralysis", ActorVal_Paralysis},
		{"Invisibility", ActorVal_Invisibility},
		{"Chameleon", ActorVal_Chameleon},
		{"NightEye", ActorVal_NightEye},
		{"DetectLifeRange", ActorVal_DetectLifeRange},
		{"FireResist", ActorVal_FireResistance},
		{"WaterBreathing", ActorVal_WaterBreathing},
		{"RadiationRads", ActorVal_RadLevel},
		{"BloodyMess", ActorVal_BloodyMess},
		{"UnarmedDamage", ActorVal_UnarmedDamage},
		{"Assistance", ActorVal_Assistance},
		{"EnergyResist", ActorVal_EnergyResistance},
		{"EMPResist", ActorVal_EMPResistance},
		{"Variable01", ActorVal_Var1Medical},
		{"Variable02", ActorVal_Variable02},
		{"Variable03", ActorVal_Variable03},
		{"Variable04", ActorVal_Variable04},
		{"Variable05", ActorVal_Variable05},
		{"Variable06", ActorVal_Variable06},
		{"Variable07", ActorVal_Variable07},
		{"Variable08", ActorVal_Variable08},
		{"Variable09", ActorVal_Variable09},
		{"Variable10", ActorVal_Variable10},
		{"IgnoreCrippledLimbs", ActorVal_IgnoreCrippledLimbs},
	};

API::ValueList API::controls = {
		ControlCode_Forward,
		ControlCode_Backward,
		ControlCode_Left,
		ControlCode_Right,
		ControlCode_Attack,
		ControlCode_Activate,
		ControlCode_Block,
		ControlCode_ReadyItem,
		ControlCode_Crouch,
		ControlCode_Run,
		ControlCode_AlwaysRun,
		ControlCode_AutoMove,
		ControlCode_Jump,
		ControlCode_TogglePOV,
		ControlCode_MenuMode,
		ControlCode_Rest,
		ControlCode_VATS,
		ControlCode_Hotkey1,
		ControlCode_Hotkey2,
		ControlCode_Hotkey3,
		ControlCode_Hotkey4,
		ControlCode_Hotkey5,
		ControlCode_Hotkey6,
		ControlCode_Hotkey7,
		ControlCode_Hotkey8,
		ControlCode_Quicksave,
		ControlCode_Quickload,
		ControlCode_Grab,
	};

API::FunctionMap API::functions = {
		{"GetPos", {"ra", Func_GetPos}},
		{"SetPos", {"rad", Func_SetPos}},
		{"GetAngle", {"ra", Func_GetAngle}},
		{"SetAngle", {"rad", Func_SetAngle}},
		{"GetBaseActorValue", {"rv", Func_GetBaseActorValue}},
		{"SetActorValue", {"rvi", Func_SetActorValue}},
		{"GetActorValue", {"rv", Func_GetActorValue}},
		{"ForceActorValue", {"rvi", Func_ForceActorValue}},
		{"GetDead", {"r", Func_GetDead}},
		{"MoveTo", {"roDDD", Func_MoveTo}},
		{"PlaceAtMe", {"rbIII", Func_PlaceAtMe}},
		{"PlaceAtMeHealthPercent", {"rbdIII", Func_PlaceAtMeHealthPercent}},
		{"SetRestrained", {"ri", Func_SetRestrained}},
		{"PlayGroup", {"rgi", Func_PlayGroup}},
		{"SetAlert", {"ri", Func_SetAlert}},
		{"RemoveAllItems", {"rCI", Func_RemoveAllItems}},
		{"GetCombatTarget", {"r", Func_GetCombatTarget}},
		{"SetForceSneak", {"ri", Func_SetForceSneak}},
		{"GetActorState", {"rI", Func_GetActorState}},
		{"GUIChat", {"s", Func_GUIChat}},
		{"GUIMode", {"i", Func_GUIMode}},
		{"GUICreateWindow", {"s", Func_GUICreateWindow}},
		{"GUICreateButton", {"ss", Func_GUICreateButton}},
		{"GUICreateText", {"ss", Func_GUICreateText}},
		{"GUICreateEdit", {"ss", Func_GUICreateEdit}},
		{"GUIRemoveWindow", {"s", Func_GUIRemoveWindow}},
		{"GUIPos", {"sdddd", Func_GUIPos}},
		{"GUISize", {"sdddd", Func_GUISize}},
		{"GUIVisible", {"si", Func_GUIVisible}},
		{"GUILocked", {"si", Func_GUILocked}},
		{"GUIText", {"ss", Func_GUIText}},
		{"GUIClick", {"s", Func_GUIClick}},
		{"Enable", {"rI", Func_Enable}},
		{"Disable", {"rI", Func_Disable}},
		{"EquipItem", {"rjII", Func_EquipItem}},
		{"UnequipItem", {"rjII", Func_UnequipItem}},
		{"AddItem", {"rkiI", Func_AddItem}},
		{"AddItemHealthPercent", {"rjidI", Func_AddItemHealthPercent}},
		{"RemoveItem", {"rkiI", Func_RemoveItem}},
		{"Kill", {"rQII", Func_Kill}},
		{"IsMoving", {"r", Func_IsMoving}},
		{"MarkForDelete", {"r", Func_MarkForDelete}},
		{"IsAnimPlaying", {"rG", Func_IsAnimPlaying}},
		{"FireWeapon", {"r$b", Func_FireWeapon}},
		{"GetCauseofDeath", {"r", Func_GetCauseofDeath}},
		{"IsLimbGone", {"ri", Func_IsLimbGone}},
		{"EnablePlayerControls", {"IIIIIII", Func_EnablePlayerControls}},
		{"DisablePlayerControls", {"$IIIIIII", Func_DisablePlayerControls}}, // $ required, else access violation...
		{"DamageActorValue", {"rvd", Func_DamageActorValue}},
		{"RestoreActorValue", {"rvd", Func_RestoreActorValue}},
		{"PlayIdle", {"rs", Func_PlayIdle}},
		{"AgeRace", {"r$i", Func_AgeRace}},
		{"MatchRace", {"r$y", Func_MatchRace}}, // has been patched to take Race
		{"SexChange", {"r$I", Func_SexChange}},
		{"ForceWeather", {"nI", Func_ForceWeather}},
		{"ScanContainer", {"r", Func_ScanContainer}},
		{"RemoveAllItemsEx", {"r", Func_RemoveAllItemsEx}},
		{"ForceRespawn", {"", Func_ForceRespawn}},
		{"SetGlobalValue", {"ri", Func_SetGlobalValue}},
		{"UIMessage", {"si", Func_UIMessage}},
		{"Lock", {"rII", Func_Lock}},
		{"Unlock", {"r", Func_Unlock}},
		{"SetOwnership", {"rF", Func_SetOwnership}},
		{"GetLocked", {"r", Func_GetLocked}},
		{"CenterOnCell", {"$s", Func_CenterOnCell}},
		{"CenterOnExterior", {"$ii", Func_CenterOnExterior}},
		{"SetINISetting", {"$ss", Func_SetINISetting}},

		{"Load", {"$s", Func_Load}},
		{"CenterOnWorld", {"$wii", Func_CenterOnWorld}},
		{"SetName", {"rsB", Func_SetName}},
		{"GetParentCell", {"r", Func_GetParentCell}},
		{"GetFirstRef", {"III", Func_GetFirstRef}},
		{"GetNextRef", {"", Func_GetNextRef}},
		{"GetControl", {"x", Func_GetControl}},
		{"DisableControl", {"x", Func_DisableControl}},
		{"EnableControl", {"x", Func_EnableControl}},
		{"DisableKey", {"i", Func_DisableKey}},
		{"EnableKey", {"i", Func_EnableKey}},
		{"GetRefCount", {"r", Func_GetRefCount}},
		{"SetRefCount", {"ri", Func_SetRefCount}},
		{"GetBaseObject", {"r", Func_GetBaseObject}},
		{"SetCurrentHealth", {"rd", Func_SetCurrentHealth}},
	};

#ifdef VAULTMP_DEBUG
DebugInput<API> API::debug;
#endif

#pragma pack(push, 1)

struct API::op_Arg1
{
	char* type1;
	unsigned int unk1; // seem to identify the argument types, unk1 is read
	unsigned int unk2; // optional flag
	char* type2;
	unsigned int unk3;
	unsigned int unk4;
	char* type3;
	unsigned int unk5;
	unsigned int unk6;
	char* type4;
	unsigned int unk7;
	unsigned int unk8;
	double more[6];

	op_Arg1()
	{
		type1 = 0x00000000; // ASCII, not read
		type2 = 0x00000000;
		type3 = 0x00000000;
		type4 = 0x00000000;
		ZeroMemory(more, sizeof(more));
	}
};

struct API::op_Arg2
{
	unsigned int unk1; // when we do not operate on a reference, this is missing
	unsigned short opcode; // important
	unsigned short unk2; // varies but not read
	unsigned short numargs; // number of arguments passed
	double param1;
	double more[16];

	op_Arg2()
	{
		unk1 = 0x0001001C; // this is when we operate on a reference (via console)
		param1 = 0x0000000000000000;
		ZeroMemory(more, sizeof(more));
	}
};

struct API::op_Arg3
{
	unsigned int reference; // the reference the command operates on, can be 0x00

	op_Arg3()
	{
		reference = 0x00000000;
	}
};

struct API::op_Arg4
{
	unsigned int unk1; // always 0x00?

	op_Arg4()
	{
		unk1 = 0x00000000;
	}
};

struct API::op_Arg5
{
	unsigned int unk1; // accessed when a command fails, I found this out while experimenting with PlayGroup
	unsigned int unk2;
	unsigned int unk3; // don't have a clue what this is, but seems to be constant and it's being read
	unsigned int unk4; // next refID?
	double unk5;
	unsigned int unk6;
	unsigned int numargs; // count of reference arguments below
	double unk8;
	double unk9;
	double unk10;
	double unk11;
	unsigned int unk12; // param1 for Oblivion
	unsigned int* param1; // pointer to reference argument, offset 0x44
	unsigned int** param2; // pointer to pointer to reference argument

	// param1
	char* param1_reftext; // ASCII reference
	unsigned int param1_unk2; // 0x00060006 = player, 0x00080008 = temp actor, 0x00050005 = base model, static ref?
	unsigned int param1_reference; // pointer to object
	unsigned int param1_unk4; // 0x00

	// param2
	unsigned int* param2_real;
	unsigned int param2_null; // 0x00!

	char* param2_reftext; // see above
	unsigned int param2_unk2;
	unsigned int param2_reference;
	unsigned int param2_unk4;

	op_Arg5()
	{
		unk1 = 0x00DD3D0C;
		unk2 = 0x00000000;
		unk3 = 0x0000400A;
		unk4 = 0x00000000;
		unk12 = 0x00000000;
		param1 = 0x00000000;
		param2 = 0x00000000;

		numargs = 0x00000000;

		param1_reftext = 0x00000000;
		param1_unk2 = 0x00000000;
		param1_reference = 0x00000000;
		param1_unk4 = 0x00000000;

		param2_reftext = 0x00000000;
		param2_unk2 = 0x00000000;
		param2_reference = 0x00000000;
		param2_unk4 = 0x00000000;

		unsigned int** param1 = &this->param1;
		unsigned int** * param2 = &this->param2;

		*param1 = (unsigned int*)((unsigned) &param1_reftext - (unsigned) &unk1);
		*param2 = (unsigned int**)((unsigned) &param2_real - (unsigned) &unk1);
		param2_real = (unsigned int*)((unsigned) &param2_reftext - (unsigned) &unk1);
		param2_null = 0x00000000;
	}
};

struct API::op_Arg6
{
	void* arg5; // pointer to Arg5

	op_Arg6()
	{
		arg5 = 0x00000000;
	}
};

struct API::op_Arg7
{
	double result; // result storage

	op_Arg7()
	{
		result = 0x0000000000000000;
	}
};

struct API::op_Arg8
{
	unsigned int offset; // offset beginning arg2 to arguments

	op_Arg8()
	{
		offset = 0x00000008;
	}
};

struct API::op_default
{
	bool delegate;
	unsigned char size_arg1;
	op_Arg1 arg1;
	unsigned char size_arg2;
	op_Arg2 arg2;
	unsigned char size_arg3;
	op_Arg3 arg3;
	unsigned char size_arg4;
	op_Arg4 arg4;
	unsigned char size_arg5;
	op_Arg5 arg5;
	unsigned char size_arg6;
	op_Arg6 arg6;
	unsigned char size_arg7;
	op_Arg7 arg7;
	unsigned char size_arg8;
	op_Arg8 arg8;

	op_default()
	{
		delegate = false;
		size_arg1 = sizeof(op_Arg1);
		size_arg2 = sizeof(op_Arg2);
		size_arg3 = sizeof(op_Arg3);
		size_arg4 = sizeof(op_Arg4);
		size_arg5 = sizeof(op_Arg5);
		size_arg6 = sizeof(op_Arg6);
		size_arg7 = sizeof(op_Arg7);
		size_arg8 = sizeof(op_Arg8);
	}
};

#pragma pack(pop)

void API::Initialize()
{
	srand(time(nullptr));
}

void API::Terminate()
{
	queue.clear();
}

vector<double> API::ParseCommand(const char* cmd_, const char* def, op_default* result, unsigned short opcode)
{
	if (!cmd_ || !def || !result || !*cmd_ || !opcode)
		throw VaultException("Invalid call to API::ParseCommand, one or more arguments are NULL (%s, %s, %04X)", cmd_, def, opcode).stacktrace();

	string _cmd(cmd_);
	vector<char> cmd_buf(cmd_, cmd_ + _cmd.length() + 1);
	vector<double> result_data;

	char* cmd = &cmd_buf[0];

	char* arg1_pos = reinterpret_cast<char*>(&result->arg1.unk1);
	char* arg2_pos = reinterpret_cast<char*>(&result->arg2.param1);
	unsigned short* _opcode = &result->arg2.opcode;
	unsigned short* _numargs = &result->arg2.numargs;

	char* tokenizer = nullptr;
	unsigned int reference = 0x00;
	result_data.emplace_back(storeIn<double>(opcode));

	// Skip the function name
	tokenizer = strtok(cmd, " ");

	if (*def == 'r')
	{
		tokenizer = strtok(nullptr, " ");

		if (tokenizer == nullptr)
			throw VaultException("API::ParseCommand expected a reference base operand, which could not be found").stacktrace();

		reference = strtoul(tokenizer, nullptr, 0);

		if (reference == 0x00)
			throw VaultException("API::ParseCommand reference base operand is NULL (%s, %s, %04X)", _cmd.c_str(), def, opcode).stacktrace();

		result->arg3.reference = reference;
		result_data.emplace_back(storeIn<double>(reference));
		++def;
	}
	else
	{
		// shift the stream pointers 4 byte
		arg2_pos -= 4;
		_opcode -= 2;
		_numargs -= 2;
		result->arg8.offset = 0x00000004;
	}

	*_opcode = opcode;

	// Skip the function name

	unsigned short numargs = 0x00;
	unsigned int refparam = 0x00;

	while (*def)
	{
		char type = *def++;

		switch (type)
		{
			case '$': // delegate
				result->delegate = true;
				continue;

			default:
				break;
		}

		*reinterpret_cast<unsigned int*>(arg1_pos + 4) = isupper(type) ? 0x00000001 : 0x00000000;

		unsigned int typecode;

		switch (tolower(type))
		{
			case 's': // String
				typecode = 0x00000000;
				break;

			case 'x': // Control code
			case 'i': // Integer
				typecode = 0x00000001;
				break;

			case 'd': // Double
				typecode = 0x00000002;
				break;

			case 'j': // Object ID item
				typecode = 0x00000003;
				break;

			case 'o': // Object Reference ID
				typecode = 0x00000004;
				break;

			case 'v': // Actor Value
				typecode = 0x00000005;
				break;

			case 'q': // Actor
				typecode = 0x00000006;
				break;

			case 'a': // Axis
				typecode = 0x00000008;
				break;

			case 'g': // Animation Group
				typecode = 0x0000000A;
				break;

			case 'y': // Race
				typecode = 0x0000000F;
				break;

			case 'b': // Object ID
				typecode = 0x00000015;
				break;

			case 'c': // Container
				typecode = 0x0000001A;
				break;

			case 'w': // Container
				typecode = 0x0000001B;
				break;

			case 'n': // Weather
				typecode = 0x00000021;
				break;

			case 'f': // Owner
				typecode = 0x00000023;
				break;

			case 'k': // Object ID base item
				typecode = 0x00000032;
				break;

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", static_cast<unsigned int>(tolower(type))).stacktrace();
		}

		*reinterpret_cast<unsigned int*>(arg1_pos) = typecode;

		arg1_pos += 0x0C;

		if (tokenizer != nullptr)
			tokenizer = strtok(nullptr, " ");
		else
			continue;

		if (tokenizer == nullptr)
		{
			if (isupper(type))
				continue;
			else
				throw VaultException("API::ParseCommand failed parsing command %s (end of input reached, not all required arguments could be found)", _cmd.c_str()).stacktrace();
		}

		/* Types:
			a (Axis, 1 byte) - 0x00000008
			d (Double, 8 byte, 0x7A) - 0x00000002
			i (Integer, 4 byte, 0x6E) - 0x00000001
			v (Actor Value, 2 byte) - 0x00000005
			o (Object Reference ID, 2 byte, 0x72, stream) - 0x00000004
			b (Object ID, 2 byte, 0x72, stream) - 0x00000015
			g (Animation Group, 2 byte) - 0x0000000A
			j (Object ID item, 2 byte, 0x72, stream) - 0x00000003
			k (Object ID base item, 2 byte, 0x72, stream) - 0x00000032
			c (Container, 2 byte, 0x72, stream) - 0x0000001A
			w (World space, 2 byte, 0x72, stream) - 0x0000001B
			q (Actor, 2 byte, 0x72, stream) - 0x00000006
			y (Race, 2 byte, 0x72, stream) - 0x0000000F
			n (Weather, 2 byte, 0x72, stream) - 0x00000021
			s (String, 2 byte, length, followed by chars) - 0x00000000
			x (Control code, 4 byte, 0x6E) - 0x00000001
			f (Owner, 2 byte, 0x72, stream) - 0x00000023
			r (Reference)

			upper case means optional
		*/

		switch (tolower(type))
		{
			case 'x': // Control code
			case 'i': // Integer
			{
				unsigned int integer = strtoul(tokenizer, nullptr, 0);

				if (tolower(type) == 'x' && !IsControl((unsigned char) integer))
					throw VaultException("API::ParseCommand could not find a control code for input %s", tokenizer).stacktrace();

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x6E;
				*reinterpret_cast<unsigned int*>(arg2_pos + sizeof(unsigned char)) = integer;
				result_data.emplace_back(storeIn<double>(integer));
				arg2_pos += sizeof(unsigned char) + sizeof(unsigned int);
				break;
			}

			case 'd': // Double
			{
				double floating = atof(tokenizer);
				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x7A;
				*reinterpret_cast<double*>(arg2_pos + sizeof(unsigned char)) = floating;
				result_data.emplace_back(floating);
				arg2_pos += sizeof(unsigned char) + sizeof(double);
				break;
			}

			case 'b': // Object ID
			case 'j': // Object ID item
			case 'k': // Object ID base item
			case 'o': // Object Reference ID
			case 'q': // Actor
			case 'c': // Container
			case 'w': // World space
			case 'y': // Race
			case 'n': // Weather
			case 'f': // Owner
			{
				if (refparam != 0x00)   // We don't support more than one refparam yet
					throw VaultException("API::ParseCommand does only support one reference argument up until now").stacktrace();

				refparam = strtoul(tokenizer, nullptr, 0);

				if (!refparam)
					throw VaultException("API::ParseCommand reference argument is NULL").stacktrace();

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x72;
				*reinterpret_cast<unsigned short*>(arg2_pos + sizeof(unsigned char)) = (!reference || refparam == reference) ? 0x0001 : 0x0002;
				result_data.emplace_back(storeIn<double>(refparam));
				arg2_pos += sizeof(unsigned char) + sizeof(unsigned short);
				break;
			}

			case 'v': // Actor Value
			{
				unsigned char value = RetrieveValue(tokenizer);

				if (value == 0xFF)
					throw VaultException("API::ParseCommand could not find an Actor Value identifier for input %s", tokenizer).stacktrace();

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) value;
				result_data.emplace_back(storeIn<double>(value));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 'a': // Axis
			{
				unsigned char axis = RetrieveAxis(tokenizer);

				if (axis == 0xFF)
					throw VaultException("API::ParseCommand could not find an Axis identifier for input %s", tokenizer).stacktrace();

				*reinterpret_cast<unsigned char*>(arg2_pos) = axis;
				result_data.emplace_back(storeIn<double>(axis));
				arg2_pos += sizeof(unsigned char);
				break;
			}

			case 'g': // Animation Group
			{
				unsigned char anim = RetrieveAnim(tokenizer);

				if (anim == 0xFF)
					throw VaultException("API::ParseCommand could not find an Animation identifier for input %s", tokenizer).stacktrace();

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) anim;
				result_data.emplace_back(storeIn<double>(anim));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 's': // String
			{
				string str = *tokenizer != '^' ? Utils::str_replace(tokenizer, "|", " ") : "";

				unsigned short length = (unsigned short) str.length();

				*reinterpret_cast<unsigned short*>(arg2_pos) = length;
				memcpy(arg2_pos + sizeof(unsigned short), str.c_str(), length + sizeof(unsigned char));
				result_data.emplace_back(0); // Don't pass on string for now
				arg2_pos += sizeof(unsigned short);
				arg2_pos += length + sizeof(unsigned char);
				break;
			}

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", static_cast<unsigned int>(tolower(type))).stacktrace();
		}

		++numargs;
	}

	*_numargs = numargs;

	unsigned int first_ref = reference ? reference : refparam;
	unsigned int second_ref = refparam;

	if (first_ref)
	{
		result->arg5.numargs++;
		result->arg5.param1_reference = first_ref;

		// this is totally incomplete, but I don't think it matters (afaik)

		if (first_ref == PLAYER_REFERENCE)
			result->arg5.param1_unk2 = 0x00060006;
		else
			result->arg5.param1_unk2 = (first_ref & 0xFF000000) == 0xFF000000 ? 0x00060006 : 0x00050005;

		if (second_ref != 0x00 && first_ref != second_ref)
		{
			result->arg5.numargs++;
			result->arg5.param2_reference = second_ref;

			if (second_ref == PLAYER_REFERENCE)
				result->arg5.param2_unk2 = 0x00060006;
			else
				result->arg5.param2_unk2 = (second_ref & 0xFF000000) == 0xFF000000 ? 0x00060006 : 0x00050005;
		}
	}

	return result_data;
}

unsigned char API::RetrieveValue(const char* value)
{
	auto it = values.find(value);
	return it != values.end() ? it->second : 0xFF;
}

unsigned char API::RetrieveAxis(const char* axis)
{
	auto it = API::axis.find(axis);
	return it != API::axis.end() ? it->second : 0xFF;
}

unsigned char API::RetrieveAnim(const char* anim)
{
	auto it = anims.find(anim);
	return it != anims.end() ? it->second : 0xFF;
}

bool API::IsControl(unsigned char control)
{
	return controls.find(control) != controls.end();
}

string API::RetrieveValue_Reverse(unsigned char value)
{
	auto it = find_if(values.begin(), values.end(), [value](const ValueMap::value_type& data) { return data.second == value; });
	return it != values.end() ? it->first : string();
}

string API::RetrieveAxis_Reverse(unsigned char axis)
{
	auto it = find_if(API::axis.begin(), API::axis.end(), [axis](const ValueMap::value_type& data) { return data.second == axis; });
	return it != API::axis.end() ? it->first : string();
}

string API::RetrieveAnim_Reverse(unsigned char anim)
{
	auto it = find_if(anims.begin(), anims.end(), [anim](const ValueMap::value_type& data) { return data.second == anim; });
	return it != anims.end() ? it->first : string();
}

vector<unsigned char> API::RetrieveAllValues()
{
	vector<unsigned char> result;
	result.reserve(values.size());
	transform(values.begin(), values.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.second; });
	return result;
}

vector<unsigned char> API::RetrieveAllAxis()
{
	vector<unsigned char> result;
	result.reserve(axis.size());
	transform(axis.begin(), axis.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.second; });
	return result;
}

vector<unsigned char> API::RetrieveAllAnims()
{
	vector<unsigned char> result;
	result.reserve(anims.size());
	transform(anims.begin(), anims.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.second; });
	return result;
}

vector<unsigned char> API::RetrieveAllControls()
{
	return {controls.begin(), controls.end()};
}

vector<string> API::RetrieveAllValues_Reverse()
{
	vector<string> result;
	result.reserve(values.size());
	transform(values.begin(), values.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.first; });
	return result;
}

vector<string> API::RetrieveAllAxis_Reverse()
{
	vector<string> result;
	result.reserve(axis.size());
	transform(axis.begin(), axis.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.first; });
	return result;
}

vector<string> API::RetrieveAllAnims_Reverse()
{
	vector<string> result;
	result.reserve(anims.size());
	transform(anims.begin(), anims.end(), back_inserter(result), [](const ValueMap::value_type& data) { return data.first; });
	return result;
}

const pair<string, unsigned short>& API::RetrieveFunction(const string& name)
{
	auto it = functions.find(name);

	if (it != functions.end())
		return it->second;

	static auto empty = pair<string, unsigned short>();

	return empty;
}

unsigned char* API::BuildCommandStream(vector<double>&& info, unsigned int key, unsigned char* command, unsigned int size)
{
	if (size + 5 > PIPE_LENGTH)
		throw VaultException("Error in API class; command size (%d bytes) exceeds the pipe length of %d bytes", size + 5, PIPE_LENGTH).stacktrace();

	unsigned char* data = new unsigned char[PIPE_LENGTH];
	ZeroMemory(data, PIPE_LENGTH);
	data[0] = PIPE_OP_COMMAND;

	memcpy(data + 5, command, size);

	unsigned int r = rand();
	*reinterpret_cast<unsigned int*>(data + 1) = r;
	queue.emplace_front(r, move(info), key);

	return data;
}

API::CommandParsed API::Translate(const vector<string>& cmd, unsigned int key)
{
	CommandParsed stream;

	for (const string& command : cmd)
	{
		const auto& func = RetrieveFunction(command.substr(0, command.find_first_of(' ')));

		if (!func.second)
		{
#ifdef VAULTMP_DEBUG
			debug.print("API was not able to find function for ", command.c_str());
#endif
			continue;
		}

		op_default result;

		vector<double> parsed = ParseCommand(command.c_str(), func.first.c_str(), &result, func.second);
		unsigned char* data = BuildCommandStream(move(parsed), key, reinterpret_cast<unsigned char*>(&result), sizeof(op_default));
		stream.emplace_back(data);
	}

	return stream;
}

vector<API::CommandResult> API::Translate(unsigned char* stream)
{
	if (stream[0] != PIPE_OP_RETURN && stream[0] != PIPE_OP_RETURN_BIG && stream[0] != PIPE_OP_RETURN_RAW)
		throw VaultException("API could not recognize stream identifier %02X", stream[0]).stacktrace();

	vector<CommandResult> result;

	if (stream[0] != PIPE_OP_RETURN_RAW)
	{
		unsigned int r = *reinterpret_cast<unsigned int*>(stream + 1);

		while (!queue.empty() && get<0>(queue.back()) != r)
		{
			auto& element = queue.back();

#ifdef VAULTMP_DEBUG
			debug.print("API did not retrieve the result of command with identifier ", hex, get<0>(element), " (opcode ", getFrom<unsigned short>(get<1>(element).at(0)), ")");
#endif

			result.emplace_back();

			auto& _result = result.back();

			get<0>(_result) = get<2>(element);
			get<1>(_result).swap(get<1>(element));
			get<2>(_result) = 0;
			get<3>(_result) = true;

			queue.pop_back();
		}

		if (queue.empty())
		{
#ifdef VAULTMP_DEBUG
			debug.print("API could not find a stored command with identifier ", hex, r, " (queue is empty)");
#endif
			return result;
		}
	}

	result.emplace_back();

	auto& _result = result.back();

	if (stream[0] != PIPE_OP_RETURN_RAW)
	{
		auto& element = queue.back();
		get<0>(_result) = get<2>(element);
		get<1>(_result).swap(get<1>(element));
	}
	else
	{
		get<0>(_result) = 0x00000000;
		get<1>(_result).emplace_back(storeIn<double, unsigned short>(*reinterpret_cast<unsigned int*>(stream + 1)));
	}

	unsigned char* data = stream + 5;

	if (stream[0] != PIPE_OP_RETURN)
	{
		unsigned int length = *reinterpret_cast<unsigned int*>(data);
		data += sizeof(unsigned int);
		vector<unsigned char>* big = new vector<unsigned char>(data, data + length);
		get<2>(_result) = storeIn<double>(big);
	}
	else
		get<2>(_result) = *reinterpret_cast<double*>(data);

	get<3>(_result) = false;

	if (stream[0] != PIPE_OP_RETURN_RAW)
		queue.pop_back();

	return result;
}
