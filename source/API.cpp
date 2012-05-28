#include "API.h"

using namespace std;
using namespace Values;

FunctionMap API::functions;
ValueMap API::values;
ValueMap API::axis;
ValueMap API::anims;
ValueList API::controls;
CommandQueue API::queue;
unsigned char API::game = 0x00;

#ifdef VAULTMP_DEBUG
Debug* API::debug = NULL;
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

	op_Arg1()
	{
		type1 = 0x00000000; // ASCII, not read
		type2 = 0x00000000;
		type3 = 0x00000000;
		type4 = 0x00000000;
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
	unsigned int unk1; // always 0x00

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
		unk1 = (game & FALLOUT3) ? 0x00DD3D0C : 0x01037094;
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

void API::Initialize(unsigned char game)
{
	API::game = game;
	srand(time(NULL));

	DefineAxisString("X", Axis_X, ALL_GAMES);
	DefineAxisString("Y", Axis_Y, ALL_GAMES);
	DefineAxisString("Z", Axis_Z, ALL_GAMES);

	DefineAnimString("Idle", AnimGroup_Idle, ALL_GAMES);
	DefineAnimString("DynamicIdle", AnimGroup_DynamicIdle, ALL_GAMES);
	DefineAnimString("SpecialIdle", AnimGroup_SpecialIdle, ALL_GAMES);
	DefineAnimString("Forward", AnimGroup_Forward, ALL_GAMES);
	DefineAnimString("Backward", AnimGroup_Backward, ALL_GAMES);
	DefineAnimString("Left", AnimGroup_Left, ALL_GAMES);
	DefineAnimString("Right", AnimGroup_Right, ALL_GAMES);
	DefineAnimString("FastForward", AnimGroup_FastForward, ALL_GAMES);
	DefineAnimString("FastBackward", AnimGroup_FastBackward, ALL_GAMES);
	DefineAnimString("FastLeft", AnimGroup_FastLeft, ALL_GAMES);
	DefineAnimString("FastRight", AnimGroup_FastRight, ALL_GAMES);
	DefineAnimString("DodgeForward", AnimGroup_DodgeForward, ALL_GAMES);
	DefineAnimString("DodgeBack", AnimGroup_DodgeBack, ALL_GAMES);
	DefineAnimString("DodgeLeft", AnimGroup_DodgeLeft, ALL_GAMES);
	DefineAnimString("DodgeRight", AnimGroup_DodgeRight, ALL_GAMES);
	DefineAnimString("TurnLeft", AnimGroup_TurnLeft, ALL_GAMES);
	DefineAnimString("TurnRight", AnimGroup_TurnRight, ALL_GAMES);

	DefineAnimString("JumpStart", Fallout3::AnimGroup_JumpStart, FALLOUT3);
	DefineAnimString("JumpLoop", Fallout3::AnimGroup_JumpLoop, FALLOUT3);
	DefineAnimString("JumpLand", Fallout3::AnimGroup_JumpLand, FALLOUT3);
	DefineAnimString("JumpLoopForward", Fallout3::AnimGroup_JumpLoopForward, FALLOUT3);
	DefineAnimString("JumpLoopBackward", Fallout3::AnimGroup_JumpLoopBackward, FALLOUT3);
	DefineAnimString("JumpLoopLeft", Fallout3::AnimGroup_JumpLoopLeft, FALLOUT3);
	DefineAnimString("JumpLoopRight", Fallout3::AnimGroup_JumpLoopRight, FALLOUT3);
	DefineAnimString("JumpLandForward", Fallout3::AnimGroup_JumpLandForward, FALLOUT3);
	DefineAnimString("JumpLandBackward", Fallout3::AnimGroup_JumpLandBackward, FALLOUT3);
	DefineAnimString("JumpLandLeft", Fallout3::AnimGroup_JumpLandLeft, FALLOUT3);
	DefineAnimString("JumpLandRight", Fallout3::AnimGroup_JumpLandRight, FALLOUT3);

	DefineAnimString("JumpStart", FalloutNV::AnimGroup_JumpStart, NEWVEGAS);
	DefineAnimString("JumpLoop", FalloutNV::AnimGroup_JumpLoop, NEWVEGAS);
	DefineAnimString("JumpLand", FalloutNV::AnimGroup_JumpLand, NEWVEGAS);
	DefineAnimString("JumpLoopForward", FalloutNV::AnimGroup_JumpLoopForward, NEWVEGAS);
	DefineAnimString("JumpLoopBackward", FalloutNV::AnimGroup_JumpLoopBackward, NEWVEGAS);
	DefineAnimString("JumpLoopLeft", FalloutNV::AnimGroup_JumpLoopLeft, NEWVEGAS);
	DefineAnimString("JumpLoopRight", FalloutNV::AnimGroup_JumpLoopRight, NEWVEGAS);
	DefineAnimString("JumpLandForward", FalloutNV::AnimGroup_JumpLandForward, NEWVEGAS);
	DefineAnimString("JumpLandBackward", FalloutNV::AnimGroup_JumpLandBackward, NEWVEGAS);
	DefineAnimString("JumpLandLeft", FalloutNV::AnimGroup_JumpLandLeft, NEWVEGAS);
	DefineAnimString("JumpLandRight", FalloutNV::AnimGroup_JumpLandRight, NEWVEGAS);

	DefineValueString("Aggression", Fallout::ActorVal_Aggression, FALLOUT_GAMES);
	DefineValueString("Confidence", Fallout::ActorVal_Confidence, FALLOUT_GAMES);
	DefineValueString("Energy", Fallout::ActorVal_Energy, FALLOUT_GAMES);
	DefineValueString("Responsibility", Fallout::ActorVal_Responsibility, FALLOUT_GAMES);
	DefineValueString("Mood", Fallout::ActorVal_Mood, FALLOUT_GAMES);
	DefineValueString("Strength", Fallout::ActorVal_Strength, FALLOUT_GAMES);
	DefineValueString("Perception", Fallout::ActorVal_Perception, FALLOUT_GAMES);
	DefineValueString("Endurance", Fallout::ActorVal_Endurance, FALLOUT_GAMES);
	DefineValueString("Charisma", Fallout::ActorVal_Charisma, FALLOUT_GAMES);
	DefineValueString("Intelligence", Fallout::ActorVal_Intelligence, FALLOUT_GAMES);
	DefineValueString("Agility", Fallout::ActorVal_Agility, FALLOUT_GAMES);
	DefineValueString("Luck", Fallout::ActorVal_Luck, FALLOUT_GAMES);
	DefineValueString("ActionPoints", Fallout::ActorVal_ActionPoints, FALLOUT_GAMES);
	DefineValueString("CarryWeight", Fallout::ActorVal_CarryWeight, FALLOUT_GAMES);
	DefineValueString("CritChance", Fallout::ActorVal_CritChance, FALLOUT_GAMES);
	DefineValueString("HealRate", Fallout::ActorVal_HealRate, FALLOUT_GAMES);
	DefineValueString("Health", Fallout::ActorVal_Health, FALLOUT_GAMES);
	DefineValueString("MeleeDamage", Fallout::ActorVal_MeleeDamage, FALLOUT_GAMES);
	DefineValueString("DamageResist", Fallout::ActorVal_DamageResistance, FALLOUT_GAMES);
	DefineValueString("PoisonResist", Fallout::ActorVal_PoisonResistance, FALLOUT_GAMES);
	DefineValueString("RadResist", Fallout::ActorVal_RadResistance, FALLOUT_GAMES);
	DefineValueString("SpeedMult", Fallout::ActorVal_SpeedMultiplier, FALLOUT_GAMES);
	DefineValueString("Fatigue", Fallout::ActorVal_Fatigue, FALLOUT_GAMES);
	DefineValueString("Karma", Fallout::ActorVal_Karma, FALLOUT_GAMES);
	DefineValueString("XP", Fallout::ActorVal_XP, FALLOUT_GAMES);
	DefineValueString("PerceptionCondition", Fallout::ActorVal_Head, FALLOUT_GAMES);
	DefineValueString("EnduranceCondition", Fallout::ActorVal_Torso, FALLOUT_GAMES);
	DefineValueString("LeftAttackCondition", Fallout::ActorVal_LeftArm, FALLOUT_GAMES);
	DefineValueString("RightAttackCondition", Fallout::ActorVal_RightArm, FALLOUT_GAMES);
	DefineValueString("LeftMobilityCondition", Fallout::ActorVal_LeftLeg, FALLOUT_GAMES);
	DefineValueString("RightMobilityCondition", Fallout::ActorVal_RightLeg, FALLOUT_GAMES);
	DefineValueString("BrainCondition", Fallout::ActorVal_Brain, FALLOUT_GAMES);
	DefineValueString("Barter", Fallout::ActorVal_Barter, FALLOUT_GAMES);
	DefineValueString("BigGuns", Fallout::ActorVal_BigGuns, FALLOUT_GAMES);
	DefineValueString("EnergyWeapons", Fallout::ActorVal_EnergyWeapons, FALLOUT_GAMES);
	DefineValueString("Explosives", Fallout::ActorVal_Explosives, FALLOUT_GAMES);
	DefineValueString("Lockpick", Fallout::ActorVal_Lockpick, FALLOUT_GAMES);
	DefineValueString("Medicine", Fallout::ActorVal_Medicine, FALLOUT_GAMES);
	DefineValueString("MeleeWeapons", Fallout::ActorVal_MeleeWeapons, FALLOUT_GAMES);
	DefineValueString("Repair", Fallout::ActorVal_Repair, FALLOUT_GAMES);
	DefineValueString("Science", Fallout::ActorVal_Science, FALLOUT_GAMES);
	DefineValueString("SmallGuns", Fallout::ActorVal_SmallGuns, FALLOUT_GAMES);
	DefineValueString("Sneak", Fallout::ActorVal_Sneak, FALLOUT_GAMES);
	DefineValueString("Speech", Fallout::ActorVal_Speech, FALLOUT_GAMES);
	//DefineValueString("Throwing", Fallout::ActorVal_Throwing, FALLOUT_GAMES);
	DefineValueString("Unarmed", Fallout::ActorVal_Unarmed, FALLOUT_GAMES);
	DefineValueString("InventoryWeight", Fallout::ActorVal_InventoryWeight, FALLOUT_GAMES);
	DefineValueString("Paralysis", Fallout::ActorVal_Paralysis, FALLOUT_GAMES);
	DefineValueString("Invisibility", Fallout::ActorVal_Invisibility, FALLOUT_GAMES);
	DefineValueString("Chameleon", Fallout::ActorVal_Chameleon, FALLOUT_GAMES);
	DefineValueString("NightEye", Fallout::ActorVal_NightEye, FALLOUT_GAMES);
	DefineValueString("DetectLifeRange", Fallout::ActorVal_DetectLifeRange, FALLOUT_GAMES);
	DefineValueString("FireResist", Fallout::ActorVal_FireResistance, FALLOUT_GAMES);
	DefineValueString("WaterBreathing", Fallout::ActorVal_WaterBreathing, FALLOUT_GAMES);
	DefineValueString("RadiationRads", Fallout::ActorVal_RadLevel, FALLOUT_GAMES);
	DefineValueString("BloodyMess", Fallout::ActorVal_BloodyMess, FALLOUT_GAMES);
	DefineValueString("UnarmedDamage", Fallout::ActorVal_UnarmedDamage, FALLOUT_GAMES);
	DefineValueString("Assistance", Fallout::ActorVal_Assistance, FALLOUT_GAMES);
	DefineValueString("EnergyResist", Fallout::ActorVal_EnergyResistance, FALLOUT_GAMES);
	DefineValueString("EMPResist", Fallout::ActorVal_EMPResistance, FALLOUT_GAMES);
	DefineValueString("Variable01", Fallout::ActorVal_Var1Medical, FALLOUT_GAMES);
	DefineValueString("Variable02", Fallout::ActorVal_Variable02, FALLOUT_GAMES);
	DefineValueString("Variable03", Fallout::ActorVal_Variable03, FALLOUT_GAMES);
	DefineValueString("Variable04", Fallout::ActorVal_Variable04, FALLOUT_GAMES);
	DefineValueString("Variable05", Fallout::ActorVal_Variable05, FALLOUT_GAMES);
	DefineValueString("Variable06", Fallout::ActorVal_Variable06, FALLOUT_GAMES);
	DefineValueString("Variable07", Fallout::ActorVal_Variable07, FALLOUT_GAMES);
	DefineValueString("Variable08", Fallout::ActorVal_Variable08, FALLOUT_GAMES);
	DefineValueString("Variable09", Fallout::ActorVal_Variable09, FALLOUT_GAMES);
	DefineValueString("Variable10", Fallout::ActorVal_Variable10, FALLOUT_GAMES);
	DefineValueString("IgnoreCrippledLimbs", Fallout::ActorVal_IgnoreCrippledLimbs, FALLOUT_GAMES);

	DefineControl(Fallout::ControlCode_Forward, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Backward, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Left, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Right, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Attack, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Activate, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Block, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_ReadyItem, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Crouch, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Run, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_AlwaysRun, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_AutoMove, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Jump, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_TogglePOV, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_MenuMode, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Rest, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_VATS, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey1, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey2, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey3, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey4, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey5, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey6, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey7, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Hotkey8, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Quicksave, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Quickload, FALLOUT_GAMES);
	DefineControl(Fallout::ControlCode_Grab, FALLOUT_GAMES);

	DefineFunction("GetPos", "ra", Func_GetPos, ALL_GAMES);
	DefineFunction("SetPos", "rad", Func_SetPos, ALL_GAMES);
	DefineFunction("GetAngle", "ra", Func_GetAngle, ALL_GAMES);
	DefineFunction("SetAngle", "rad", Func_SetAngle, ALL_GAMES);
	DefineFunction("GetBaseActorValue", "rv", Func_GetBaseActorValue, ALL_GAMES);
	DefineFunction("SetActorValue", "rvi", Func_SetActorValue, ALL_GAMES);
	DefineFunction("GetActorValue", "rv", Func_GetActorValue, ALL_GAMES);
	DefineFunction("ForceActorValue", "rvi", Func_ForceActorValue, ALL_GAMES);
	DefineFunction("GetDead", "r", Func_GetDead, ALL_GAMES);
	DefineFunction("MoveTo", "roDDD", Func_MoveTo, ALL_GAMES);
	DefineFunction("PlaceAtMe", "rbiII", Func_PlaceAtMe, ALL_GAMES);
	DefineFunction("SetRestrained", "ri", Func_SetRestrained, ALL_GAMES);
	DefineFunction("PlayGroup", "rgi", Func_PlayGroup, ALL_GAMES);
	DefineFunction("SetAlert", "ri", Func_SetAlert, ALL_GAMES);
	DefineFunction("RemoveAllItems", "rCI", Func_RemoveAllItems, ALL_GAMES);
	DefineFunction("GetCombatTarget", "r", Func_GetCombatTarget, ALL_GAMES);
	DefineFunction("SetForceSneak", "ri", Func_SetForceSneak, ALL_GAMES);
	DefineFunction("GetActorState", "rI", Func_GetActorState, ALL_GAMES);   // vaultfunction

	DefineFunction("Enable", "rI", Func_Enable, FALLOUT_GAMES);
	DefineFunction("Disable", "rI", Func_Disable, FALLOUT_GAMES);
	DefineFunction("EquipItem", "rjII", Func_EquipItem, FALLOUT_GAMES);
	DefineFunction("UnequipItem", "rjII", Func_UnequipItem, FALLOUT_GAMES);
	DefineFunction("AddItem", "rkiI", Func_AddItem, FALLOUT_GAMES);
	DefineFunction("AddItemHealthPercent", "rjidI", Func_AddItemHealthPercent, FALLOUT_GAMES);
	DefineFunction("RemoveItem", "rkiI", Func_RemoveItem, FALLOUT_GAMES);
	DefineFunction("Kill", "rQII", Func_Kill, FALLOUT_GAMES);
	DefineFunction("IsMoving", "r", Fallout::Func_IsMoving, FALLOUT_GAMES);
	DefineFunction("MarkForDelete", "r", Fallout::Func_MarkForDelete, FALLOUT_GAMES);
	DefineFunction("IsAnimPlaying", "rG", Fallout::Func_IsAnimPlaying, FALLOUT_GAMES);
	DefineFunction("ScanContainer", "r", Fallout::Func_ScanContainer, FALLOUT_GAMES);
	DefineFunction("UIMessage", "s", Fallout::Func_UIMessage, FALLOUT_GAMES);

	DefineFunction("Load", "$s", Fallout3::Func_Load, FALLOUT3);
	DefineFunction("SetName", "rsB", Fallout3::Func_SetName, FALLOUT3);
	DefineFunction("GetParentCell", "r", Fallout3::Func_GetParentCell, FALLOUT3);
	DefineFunction("GetFirstRef", "III", Fallout3::Func_GetFirstRef, FALLOUT3);
	DefineFunction("GetNextRef", "", Fallout3::Func_GetNextRef, FALLOUT3);
	DefineFunction("GetControl", "x", Fallout3::Func_GetControl, FALLOUT3);

	DefineFunction("Load", "$s", FalloutNV::Func_Load, NEWVEGAS);
	DefineFunction("SetName", "rsB", FalloutNV::Func_SetName, NEWVEGAS);
	DefineFunction("GetParentCell", "r", FalloutNV::Func_GetParentCell, NEWVEGAS);
	DefineFunction("GetFirstRef", "III", FalloutNV::Func_GetFirstRef, NEWVEGAS);
	DefineFunction("GetNextRef", "", FalloutNV::Func_GetNextRef, NEWVEGAS);
	DefineFunction("GetControl", "x", FalloutNV::Func_GetControl, NEWVEGAS);
}

void API::Terminate()
{
	values.clear();
	axis.clear();
	anims.clear();
	controls.clear();
	functions.clear();
	queue.clear();
}

#ifdef VAULTMP_DEBUG
void API::SetDebugHandler(Debug* debug)
{
	API::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to API class", true);
}
#endif

vector<double> API::ParseCommand(char* cmd, const char* def, op_default* result, unsigned short opcode)
{
	if (*cmd == 0x00 || *def == 0x00 || opcode == 0x00)
		throw VaultException("Invalid call to API::ParseCommand, one or more arguments are NULL (%s, %s, %04X)", cmd, def, opcode);

	vector<double> result_data;
	string _cmd(cmd);

	char* arg1_pos = reinterpret_cast<char*>(&result->arg1.unk1);
	char* arg2_pos = reinterpret_cast<char*>(&result->arg2.param1);
	unsigned short* _opcode = &result->arg2.opcode;
	unsigned short* _numargs = &result->arg2.numargs;

	char* tokenizer = NULL;
	unsigned int reference = 0x00;
	result_data.push_back(storeIn<double, unsigned short>(opcode));

	// Skip the function name
	tokenizer = strtok(cmd, " ");

	if (*def == 'r')
	{
		tokenizer = strtok(NULL, " ");

		if (tokenizer == NULL)
			throw VaultException("API::ParseCommand expected a reference base operand, which could not be found");

		reference = strtoul(tokenizer, NULL, 0);

		if (reference == 0x00)
			throw VaultException("API::ParseCommand reference base operand is NULL (%s, %s, %04X)", _cmd.c_str(), def, opcode);

		result->arg3.reference = reference;
		result_data.push_back(storeIn<double, unsigned int>(reference));
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

	while (*def != '\0' && numargs < 4)   // We don't support more than 4 args yet
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

		if (isupper(type))
			*reinterpret_cast<unsigned int*>(arg1_pos + 4) = 0x00000001;

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

			case 'b': // Object ID
				typecode = 0x00000015;
				break;

			case 'c': // Container
				typecode = 0x0000001A;
				break;

			case 'k': // Object ID base item
				typecode = 0x00000032;
				break;

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
		}

		*reinterpret_cast<unsigned int*>(arg1_pos) = typecode;

		arg1_pos += 0x0C;

		if (tokenizer != NULL)
			tokenizer = strtok(NULL, " ");
		else
			continue;

		if (tokenizer == NULL)
		{
			if (isupper(type))
				continue;
			else
				throw VaultException("API::ParseCommand failed parsing command %s (end of input reached, not all required arguments could be found)", _cmd.c_str());
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
			q (Actor, 2 byte, 0x72, stream) - 0x00000006
			s (String, 2 byte, length, followed by chars) - 0x00000000
			x (Control code, 4 byte, 0x6E) - 0x00000001
			r (Reference)

			upper case means optional
		*/

		switch (tolower(type))
		{
			case 'x': // Control code
			case 'i': // Integer
			{
				unsigned int integer = strtoul(tokenizer, NULL, 0);

				if (tolower(type) == 'x' && !IsControl((unsigned char) integer))
					throw VaultException("API::ParseCommand could not find a control code for input %s", tokenizer);

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x6E;
				*reinterpret_cast<unsigned int*>(arg2_pos + sizeof(unsigned char)) = integer;
				result_data.push_back(storeIn<double, unsigned int>(integer));
				arg2_pos += sizeof(unsigned char) + sizeof(unsigned int);
				break;
			}

			case 'd': // Double
			{
				double floating = atof(tokenizer);
				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x7A;
				*reinterpret_cast<double*>(arg2_pos + sizeof(unsigned char)) = floating;
				result_data.push_back(floating);
				arg2_pos += sizeof(unsigned char) + sizeof(double);
				break;
			}

			case 'b': // Object ID
			case 'j': // Object ID item
			case 'k': // Object ID base item
			case 'o': // Object Reference ID
			case 'q': // Actor
			case 'c': // Container
			{
				if (refparam != 0x00)   // We don't support more than one refparam yet
					throw VaultException("API::ParseCommand does only support one reference argument up until now");

				refparam = strtoul(tokenizer, NULL, 0);

				if (!refparam)
					throw VaultException("API::ParseCommand reference argument is NULL");

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x72;
				*reinterpret_cast<unsigned short*>(arg2_pos + sizeof(unsigned char)) = (refparam == reference) ? 0x0001 : 0x0002;
				result_data.push_back(storeIn<double, unsigned int>(refparam));
				arg2_pos += sizeof(unsigned char) + sizeof(unsigned short);
				break;
			}

			case 'v': // Actor Value
			{
				unsigned char value = RetrieveValue(tokenizer);

				if (value == 0xFF)
					throw VaultException("API::ParseCommand could not find an Actor Value identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) value;
				result_data.push_back(storeIn<double, unsigned short>(value));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 'a': // Axis
			{
				unsigned char axis = RetrieveAxis(tokenizer);

				if (axis == 0xFF)
					throw VaultException("API::ParseCommand could not find an Axis identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned char*>(arg2_pos) = axis;
				result_data.push_back(storeIn<double, unsigned char>(axis));
				arg2_pos += sizeof(unsigned char);
				break;
			}

			case 'g': // Animation Group
			{
				unsigned char anim = RetrieveAnim(tokenizer);

				if (anim == 0xFF)
					throw VaultException("API::ParseCommand could not find an Animation identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) anim;
				result_data.push_back(storeIn<double, unsigned short>(anim));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 's': // String
			{
				string str = Utils::str_replace(tokenizer, "|", " ");

				unsigned short length = (unsigned short) str.length();

				if (length > 63)
					throw VaultException("API::ParseCommand string argument exceeds the limit of 64 characters");

				*reinterpret_cast<unsigned short*>(arg2_pos) = length;
				memcpy(arg2_pos + sizeof(unsigned short), str.c_str(), length +  sizeof(unsigned char));
				result_data.push_back(0); // Don't pass on string for now
				arg2_pos += sizeof(unsigned short);
				arg2_pos += length;
				break;
			}

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
		}

		++numargs;
	}

	*_numargs = numargs;

	if (reference != 0x00)
	{
		result->arg5.numargs++;
		result->arg5.param1_reference = reference;

		if (reference == PLAYER_REFERENCE)
			result->arg5.param1_unk2 = 0x00060006;
		else
			result->arg5.param1_unk2 = (reference & 0xFF000000) == 0xFF000000 ? 0x00060006 : 0x00050005;

		if (reference != refparam && refparam != 0x00)
		{
			result->arg5.numargs++;
			result->arg5.param2_reference = refparam;

			if (refparam == PLAYER_REFERENCE)
				result->arg5.param2_unk2 = 0x00060006;
			else
				result->arg5.param2_unk2 = (refparam & 0xFF000000) == 0xFF000000 ? 0x00060006 : 0x00050005;
		}
	}

	return result_data;
}

void API::DefineFunction(string name, string def, unsigned short opcode, unsigned char games)
{
	if (games & game)
		functions.insert(pair<string, pair<string, unsigned short> >(name, pair<string, unsigned short>(def, opcode)));
}

void API::DefineValueString(string name, unsigned char value, unsigned char games)
{
	if (games & game)
		values.insert(pair<string, unsigned char>(name, value));
}

void API::DefineAxisString(string name, unsigned char axis, unsigned char games)
{
	if (games & game)
		API::axis.insert(pair<string, unsigned char>(name, axis));
}

void API::DefineAnimString(string name, unsigned char anim, unsigned char games)
{
	if (games & game)
		anims.insert(pair<string, unsigned char>(name, anim));
}

void API::DefineControl(unsigned char control, unsigned char games)
{
	if (games & game)
		controls.insert(control);
}

unsigned char API::RetrieveValue(char* value)
{
	ValueMap::iterator it;
	it = values.find(string(value));

	if (it != values.end())
		return it->second;

	return 0xFF;
}

unsigned char API::RetrieveAxis(char* axis)
{
	ValueMap::iterator it;
	it = API::axis.find(string(axis));

	if (it != API::axis.end())
		return it->second;

	return 0xFF;
}

unsigned char API::RetrieveAnim(char* anim)
{
	ValueMap::iterator it;
	it = anims.find(string(anim));

	if (it != anims.end())
		return it->second;

	return 0xFF;
}

bool API::IsControl(unsigned char control)
{
	return (controls.find(control) != controls.end() ? true : false);
}

string API::RetrieveValue_Reverse(unsigned char value)
{
	ValueMap::iterator it;

	for (it = values.begin(); it != values.end() && it->second != value; ++it);

	if (it != values.end())
		return it->first;

	return string();
}

string API::RetrieveAxis_Reverse(unsigned char axis)
{
	ValueMap::iterator it;

	for (it = API::axis.begin(); it != API::axis.end() && it->second != axis; ++it);

	if (it != API::axis.end())
		return it->first;

	return string();
}

string API::RetrieveAnim_Reverse(unsigned char anim)
{
	ValueMap::iterator it;

	for (it = anims.begin(); it != anims.end() && it->second != anim; ++it);

	if (it != anims.end())
		return it->first;

	return string();
}

vector<unsigned char> API::RetrieveAllValues()
{
	vector<unsigned char> result;
	ValueMap::iterator it;

	for (it = values.begin(); it != values.end(); ++it)
		result.push_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllAxis()
{
	vector<unsigned char> result;
	ValueMap::iterator it;

	for (it = axis.begin(); it != axis.end(); ++it)
		result.push_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllAnims()
{
	vector<unsigned char> result;
	ValueMap::iterator it;

	for (it = anims.begin(); it != anims.end(); ++it)
		result.push_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllControls()
{
	vector<unsigned char> result;
	ValueList::iterator it;

	for (it = controls.begin(); it != controls.end(); ++it)
		result.push_back(*it);

	return result;
}

vector<string> API::RetrieveAllValues_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = values.begin(); it != values.end(); ++it)
		result.push_back(it->first);

	return result;
}

vector<string> API::RetrieveAllAxis_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = axis.begin(); it != axis.end(); ++it)
		result.push_back(it->first);

	return result;
}

vector<string> API::RetrieveAllAnims_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = anims.begin(); it != anims.end(); ++it)
		result.push_back(it->first);

	return result;
}

pair<string, unsigned short> API::RetrieveFunction(string name)
{
	FunctionMap::iterator it;
	it = functions.find(name);

	if (it != functions.end())
		return it->second;

	pair<string, unsigned short> empty = pair<string, unsigned short>("", 0x00);

	return empty;
}

bool API::AnnounceFunction(string name)
{
	pair<string, unsigned short> func = RetrieveFunction(name);

	if (!func.first.empty())
		return true;

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("API function %s not found or not supported by the game", true, name.c_str());

#endif

	return false;
}

unsigned char* API::BuildCommandStream(vector<double>& info, unsigned int key, unsigned char* command, unsigned int size)
{
	if (size + 5 > PIPE_LENGTH)
		throw VaultException("Error in API class; command size (%d bytes) exceeds the pipe length of %d bytes", size + 5, PIPE_LENGTH);

	unsigned char* data = new unsigned char[PIPE_LENGTH];
	ZeroMemory(data, sizeof(data));
	data[0] = PIPE_OP_COMMAND;

	memcpy(data + 5, command, size);

	unsigned int r = rand();
	*reinterpret_cast<unsigned int*>(data + 1) = r;
	queue.push_front(pair<pair<unsigned int, vector<double>>, unsigned int>(pair<unsigned int, vector<double>>(r, info), key));

	return data;
}

CommandParsed API::Translate(const vector<string>& cmd, unsigned int key)
{
	CommandParsed stream;

	for (const string& command : cmd)
	{
		pair<string, unsigned short> func = RetrieveFunction(command.substr(0, command.find_first_of(' ')));

		if (!func.second)
		{
#ifdef VAULTMP_DEBUG

			if (debug)
				debug->PrintFormat("API was not able to find function for %s", true, command.c_str());

#endif
			continue;
		}

		char content[command.length() + 1];
		ZeroMemory(content, sizeof(content));
		strcpy(content, command.c_str());

		op_default result;

		vector<double> parsed = ParseCommand(content, func.first.c_str(), &result, func.second);
		unsigned char* data = BuildCommandStream(parsed, key, reinterpret_cast<unsigned char*>(&result), sizeof(op_default));
		stream.push_back(unique_ptr<unsigned char[]>(data));
	}

	return stream;
}

vector<CommandResult> API::Translate(unsigned char* stream)
{
	if (stream[0] != PIPE_OP_RETURN && stream[0] != PIPE_OP_RETURN_BIG)
		throw VaultException("API could not recognize stream identifier %02X", stream[0]);

	vector<CommandResult> result;
	unsigned int r = *reinterpret_cast<unsigned int*>(stream + 1);

	while (!queue.empty() && queue.back().first.first != r)
	{
#ifdef VAULTMP_DEBUG

		if (debug)
			debug->PrintFormat("API did not retrieve the result of command with identifier %08X (opcode %04hX)", true, queue.back().first.first, getFrom<double, unsigned short>(queue.back().first.second.at(0)));

#endif

		double zero = 0x0000000000000000;
		result.push_back(CommandResult());
		result.back().first.first.first = queue.back().second;
		result.back().first.first.second = queue.back().first.second;
		result.back().first.second = zero;
		result.back().second = true;

		queue.pop_back();
	}

	if (queue.empty())
	{
#ifdef VAULTMP_DEBUG

		if (debug)
			debug->PrintFormat("API could not find a stored command with identifier %08X (queue is empty)", true, r);

#endif
		return result;
	}

	result.push_back(CommandResult());
	result.back().first.first.first = queue.back().second;
	result.back().first.first.second = queue.back().first.second;

	if (stream[0] == PIPE_OP_RETURN_BIG)
	{
		unsigned int length = *reinterpret_cast<unsigned int*>(stream + 5);
		unsigned char* data = reinterpret_cast<unsigned char*>(stream + 9);
		vector<unsigned char>* big = new vector<unsigned char>(data, data + length);
		result.back().first.second = storeIn<double, vector<unsigned char>*>(big);
	}
	else
		result.back().first.second = *reinterpret_cast<double*>(stream + 5);

	result.back().second = false;

	queue.pop_back();

	return result;
}

unsigned char API::GetGameCode()
{
	return game;
}
