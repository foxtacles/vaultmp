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

const unsigned char API::FalloutSavegame[] =
{0x46, 0x4F, 0x33, 0x53, 0x41, 0x56, 0x45, 0x47, 0x41, 0x4D,
0x45, 0x27, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x7C,
0x01, 0x00, 0x00, 0x00, 0x7C, 0x01, 0x00, 0x00, 0x00, 0x7C,
0x00, 0x00, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00,
0x7C, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x01, 0x00, 0x7C, 0x3F,
0x7C, 0x00, 0x00, 0x7C, 0x00, 0x00, 0x00, 0x15, 0x02, 0x00,
0x00, 0x00, 0x00, 0x7C, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x01, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7C};

#ifdef VAULTMP_DEBUG
Debug* API::debug;
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
	srand(time(nullptr));

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
	DefineAnimString("Aim", AnimGroup_Aim, ALL_GAMES);
	DefineAnimString("AimUp", AnimGroup_AimUp, ALL_GAMES);
	DefineAnimString("AimDown", AnimGroup_AimDown, ALL_GAMES);
	DefineAnimString("AimIS", AnimGroup_AimIS, ALL_GAMES);
	DefineAnimString("AimISUp", AnimGroup_AimISUp, ALL_GAMES);
	DefineAnimString("AimISDown", AnimGroup_AimISDown, ALL_GAMES);
	DefineAnimString("AttackLeft", AnimGroup_AttackLeft, ALL_GAMES);
	DefineAnimString("AttackLeftUp", AnimGroup_AttackLeftUp, ALL_GAMES);
	DefineAnimString("AttackLeftDown", AnimGroup_AttackLeftDown, ALL_GAMES);
	DefineAnimString("AttackLeftIS", AnimGroup_AttackLeftIS, ALL_GAMES);
	DefineAnimString("AttackLeftISUp", AnimGroup_AttackLeftISUp, ALL_GAMES);
	DefineAnimString("AttackLeftISDown", AnimGroup_AttackLeftISDown, ALL_GAMES);
	DefineAnimString("AttackRight", AnimGroup_AttackRight, ALL_GAMES);
	DefineAnimString("AttackRightUp", AnimGroup_AttackRightUp, ALL_GAMES);
	DefineAnimString("AttackRightDown", AnimGroup_AttackRightDown, ALL_GAMES);
	DefineAnimString("AttackRightIS", AnimGroup_AttackRightIS, ALL_GAMES);
	DefineAnimString("AttackRightISUp", AnimGroup_AttackRightISUp, ALL_GAMES);
	DefineAnimString("AttackRightISDown", AnimGroup_AttackRightISDown, ALL_GAMES);
	DefineAnimString("Attack3", AnimGroup_Attack3, ALL_GAMES);
	DefineAnimString("Attack3Up", AnimGroup_Attack3Up, ALL_GAMES);
	DefineAnimString("Attack3Down", AnimGroup_Attack3Down, ALL_GAMES);
	DefineAnimString("Attack3IS", AnimGroup_Attack3IS, ALL_GAMES);
	DefineAnimString("Attack3ISUp", AnimGroup_Attack3ISUp, ALL_GAMES);
	DefineAnimString("Attack3ISDown", AnimGroup_Attack3ISDown, ALL_GAMES);
	DefineAnimString("Attack4", AnimGroup_Attack4, ALL_GAMES);
	DefineAnimString("Attack4Up", AnimGroup_Attack4Up, ALL_GAMES);
	DefineAnimString("Attack4Down", AnimGroup_Attack4Down, ALL_GAMES);
	DefineAnimString("Attack4IS", AnimGroup_Attack4IS, ALL_GAMES);
	DefineAnimString("Attack4ISUp", AnimGroup_Attack4ISUp, ALL_GAMES);
	DefineAnimString("Attack4ISDown", AnimGroup_Attack4ISDown, ALL_GAMES);
	DefineAnimString("Attack5", AnimGroup_Attack5, ALL_GAMES);
	DefineAnimString("Attack5Up", AnimGroup_Attack5Up, ALL_GAMES);
	DefineAnimString("Attack5Down", AnimGroup_Attack5Down, ALL_GAMES);
	DefineAnimString("Attack5IS", AnimGroup_Attack5IS, ALL_GAMES);
	DefineAnimString("Attack5ISUp", AnimGroup_Attack5ISUp, ALL_GAMES);
	DefineAnimString("Attack5ISDown", AnimGroup_Attack5ISDown, ALL_GAMES);
	DefineAnimString("Attack6", AnimGroup_Attack6, ALL_GAMES);
	DefineAnimString("Attack6Up", AnimGroup_Attack6Up, ALL_GAMES);
	DefineAnimString("Attack6Down", AnimGroup_Attack6Down, ALL_GAMES);
	DefineAnimString("Attack6IS", AnimGroup_Attack6IS, ALL_GAMES);
	DefineAnimString("Attack6ISUp", AnimGroup_Attack6ISUp, ALL_GAMES);
	DefineAnimString("Attack6ISDown", AnimGroup_Attack6ISDown, ALL_GAMES);
	DefineAnimString("Attack7", AnimGroup_Attack7, ALL_GAMES);
	DefineAnimString("Attack7Up", AnimGroup_Attack7Up, ALL_GAMES);
	DefineAnimString("Attack7Down", AnimGroup_Attack7Down, ALL_GAMES);
	DefineAnimString("Attack7IS", AnimGroup_Attack7IS, ALL_GAMES);
	DefineAnimString("Attack7ISUp", AnimGroup_Attack7ISUp, ALL_GAMES);
	DefineAnimString("Attack7ISDown", AnimGroup_Attack7ISDown, ALL_GAMES);
	DefineAnimString("Attack8", AnimGroup_Attack8, ALL_GAMES);
	DefineAnimString("Attack8Up", AnimGroup_Attack8Up, ALL_GAMES);
	DefineAnimString("Attack8Down", AnimGroup_Attack8Down, ALL_GAMES);
	DefineAnimString("Attack8IS", AnimGroup_Attack8IS, ALL_GAMES);
	DefineAnimString("Attack8ISUp", AnimGroup_Attack8ISUp, ALL_GAMES);
	DefineAnimString("Attack8ISDown", AnimGroup_Attack8ISDown, ALL_GAMES);
	DefineAnimString("AttackLoop", AnimGroup_AttackLoop, ALL_GAMES);
	DefineAnimString("AttackLoopUp", AnimGroup_AttackLoopUp, ALL_GAMES);
	DefineAnimString("AttackLoopDown", AnimGroup_AttackLoopDown, ALL_GAMES);
	DefineAnimString("AttackLoopIS", AnimGroup_AttackLoopIS, ALL_GAMES);
	DefineAnimString("AttackLoopISUp", AnimGroup_AttackLoopISUp, ALL_GAMES);
	DefineAnimString("AttackLoopISDown", AnimGroup_AttackLoopISDown, ALL_GAMES);
	DefineAnimString("AttackSpin", AnimGroup_AttackSpin, ALL_GAMES);
	DefineAnimString("AttackSpinUp", AnimGroup_AttackSpinUp, ALL_GAMES);
	DefineAnimString("AttackSpinDown", AnimGroup_AttackSpinDown, ALL_GAMES);
	DefineAnimString("AttackSpinIS", AnimGroup_AttackSpinIS, ALL_GAMES);
	DefineAnimString("AttackSpinISUp", AnimGroup_AttackSpinISUp, ALL_GAMES);
	DefineAnimString("AttackSpinISDown", AnimGroup_AttackSpinISDown, ALL_GAMES);
	DefineAnimString("AttackSpin2", AnimGroup_AttackSpin2, ALL_GAMES);
	DefineAnimString("AttackSpin2Up", AnimGroup_AttackSpin2Up, ALL_GAMES);
	DefineAnimString("AttackSpin2Down", AnimGroup_AttackSpin2Down, ALL_GAMES);
	DefineAnimString("AttackSpin2IS", AnimGroup_AttackSpin2IS, ALL_GAMES);
	DefineAnimString("AttackSpin2ISUp", AnimGroup_AttackSpin2ISUp, ALL_GAMES);
	DefineAnimString("AttackSpin2ISDown", AnimGroup_AttackSpin2ISDown, ALL_GAMES);
	DefineAnimString("AttackPower", AnimGroup_AttackPower, ALL_GAMES);
	DefineAnimString("AttackForwardPower", AnimGroup_AttackForwardPower, ALL_GAMES);
	DefineAnimString("AttackBackPower", AnimGroup_AttackBackPower, ALL_GAMES);
	DefineAnimString("AttackLeftPower", AnimGroup_AttackLeftPower, ALL_GAMES);
	DefineAnimString("AttackRightPower", AnimGroup_AttackRightPower, ALL_GAMES);
	DefineAnimString("Holster", AnimGroup_Holster, ALL_GAMES);
	DefineAnimString("Equip", AnimGroup_Equip, ALL_GAMES);
	DefineAnimString("Unequip", AnimGroup_Unequip, ALL_GAMES);

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
	DefineAnimString("BlockIdle", Fallout3::AnimGroup_BlockIdle, FALLOUT3);
	DefineAnimString("ReloadA", Fallout3::AnimGroup_ReloadA, FALLOUT3);
	DefineAnimString("ReloadB", Fallout3::AnimGroup_ReloadB, FALLOUT3);
	DefineAnimString("ReloadC", Fallout3::AnimGroup_ReloadC, FALLOUT3);
	DefineAnimString("ReloadD", Fallout3::AnimGroup_ReloadD, FALLOUT3);
	DefineAnimString("ReloadE", Fallout3::AnimGroup_ReloadE, FALLOUT3);
	DefineAnimString("ReloadF", Fallout3::AnimGroup_ReloadF, FALLOUT3);
	DefineAnimString("ReloadG", Fallout3::AnimGroup_ReloadG, FALLOUT3);
	DefineAnimString("ReloadH", Fallout3::AnimGroup_ReloadH, FALLOUT3);
	DefineAnimString("ReloadI", Fallout3::AnimGroup_ReloadI, FALLOUT3);
	DefineAnimString("ReloadJ", Fallout3::AnimGroup_ReloadJ, FALLOUT3);
	DefineAnimString("ReloadK", Fallout3::AnimGroup_ReloadK, FALLOUT3);

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
	DefineAnimString("BlockIdle", FalloutNV::AnimGroup_BlockIdle, NEWVEGAS);
	DefineAnimString("ReloadWStart", FalloutNV::AnimGroup_ReloadWStart, NEWVEGAS);
	DefineAnimString("ReloadXStart", FalloutNV::AnimGroup_ReloadXStart, NEWVEGAS);
	DefineAnimString("ReloadYStart", FalloutNV::AnimGroup_ReloadYStart, NEWVEGAS);
	DefineAnimString("ReloadZStart", FalloutNV::AnimGroup_ReloadZStart, NEWVEGAS);
	DefineAnimString("ReloadA", FalloutNV::AnimGroup_ReloadA, NEWVEGAS);
	DefineAnimString("ReloadB", FalloutNV::AnimGroup_ReloadB, NEWVEGAS);
	DefineAnimString("ReloadC", FalloutNV::AnimGroup_ReloadC, NEWVEGAS);
	DefineAnimString("ReloadD", FalloutNV::AnimGroup_ReloadD, NEWVEGAS);
	DefineAnimString("ReloadE", FalloutNV::AnimGroup_ReloadE, NEWVEGAS);
	DefineAnimString("ReloadF", FalloutNV::AnimGroup_ReloadF, NEWVEGAS);
	DefineAnimString("ReloadG", FalloutNV::AnimGroup_ReloadG, NEWVEGAS);
	DefineAnimString("ReloadH", FalloutNV::AnimGroup_ReloadH, NEWVEGAS);
	DefineAnimString("ReloadI", FalloutNV::AnimGroup_ReloadI, NEWVEGAS);
	DefineAnimString("ReloadJ", FalloutNV::AnimGroup_ReloadJ, NEWVEGAS);
	DefineAnimString("ReloadK", FalloutNV::AnimGroup_ReloadK, NEWVEGAS);
	DefineAnimString("ReloadL", FalloutNV::AnimGroup_ReloadL, NEWVEGAS);
	DefineAnimString("ReloadM", FalloutNV::AnimGroup_ReloadM, NEWVEGAS);
	DefineAnimString("ReloadN", FalloutNV::AnimGroup_ReloadN, NEWVEGAS);
	DefineAnimString("ReloadO", FalloutNV::AnimGroup_ReloadO, NEWVEGAS);
	DefineAnimString("ReloadP", FalloutNV::AnimGroup_ReloadP, NEWVEGAS);
	DefineAnimString("ReloadQ", FalloutNV::AnimGroup_ReloadQ, NEWVEGAS);
	DefineAnimString("ReloadR", FalloutNV::AnimGroup_ReloadR, NEWVEGAS);
	DefineAnimString("ReloadS", FalloutNV::AnimGroup_ReloadS, NEWVEGAS);
	DefineAnimString("ReloadW", FalloutNV::AnimGroup_ReloadW, NEWVEGAS);
	DefineAnimString("ReloadX", FalloutNV::AnimGroup_ReloadX, NEWVEGAS);
	DefineAnimString("ReloadY", FalloutNV::AnimGroup_ReloadY, NEWVEGAS);
	DefineAnimString("ReloadZ", FalloutNV::AnimGroup_ReloadZ, NEWVEGAS);

	DefineValueString("Aggression", ActorVal_Aggression, FALLOUT_GAMES);
	DefineValueString("Confidence", ActorVal_Confidence, FALLOUT_GAMES);
	DefineValueString("Energy", ActorVal_Energy, FALLOUT_GAMES);
	DefineValueString("Responsibility", ActorVal_Responsibility, FALLOUT_GAMES);
	DefineValueString("Mood", ActorVal_Mood, FALLOUT_GAMES);
	DefineValueString("Strength", ActorVal_Strength, FALLOUT_GAMES);
	DefineValueString("Perception", ActorVal_Perception, FALLOUT_GAMES);
	DefineValueString("Endurance", ActorVal_Endurance, FALLOUT_GAMES);
	DefineValueString("Charisma", ActorVal_Charisma, FALLOUT_GAMES);
	DefineValueString("Intelligence", ActorVal_Intelligence, FALLOUT_GAMES);
	DefineValueString("Agility", ActorVal_Agility, FALLOUT_GAMES);
	DefineValueString("Luck", ActorVal_Luck, FALLOUT_GAMES);
	DefineValueString("ActionPoints", ActorVal_ActionPoints, FALLOUT_GAMES);
	DefineValueString("CarryWeight", ActorVal_CarryWeight, FALLOUT_GAMES);
	DefineValueString("CritChance", ActorVal_CritChance, FALLOUT_GAMES);
	DefineValueString("HealRate", ActorVal_HealRate, FALLOUT_GAMES);
	DefineValueString("Health", ActorVal_Health, FALLOUT_GAMES);
	DefineValueString("MeleeDamage", ActorVal_MeleeDamage, FALLOUT_GAMES);
	DefineValueString("DamageResist", ActorVal_DamageResistance, FALLOUT_GAMES);
	DefineValueString("PoisonResist", ActorVal_PoisonResistance, FALLOUT_GAMES);
	DefineValueString("RadResist", ActorVal_RadResistance, FALLOUT_GAMES);
	DefineValueString("SpeedMult", ActorVal_SpeedMultiplier, FALLOUT_GAMES);
	DefineValueString("Fatigue", ActorVal_Fatigue, FALLOUT_GAMES);
	DefineValueString("Karma", ActorVal_Karma, FALLOUT_GAMES);
	DefineValueString("XP", ActorVal_XP, FALLOUT_GAMES);
	DefineValueString("PerceptionCondition", ActorVal_Head, FALLOUT_GAMES);
	DefineValueString("EnduranceCondition", ActorVal_Torso, FALLOUT_GAMES);
	DefineValueString("LeftAttackCondition", ActorVal_LeftArm, FALLOUT_GAMES);
	DefineValueString("RightAttackCondition", ActorVal_RightArm, FALLOUT_GAMES);
	DefineValueString("LeftMobilityCondition", ActorVal_LeftLeg, FALLOUT_GAMES);
	DefineValueString("RightMobilityCondition", ActorVal_RightLeg, FALLOUT_GAMES);
	DefineValueString("BrainCondition", ActorVal_Brain, FALLOUT_GAMES);
	DefineValueString("Barter", ActorVal_Barter, FALLOUT_GAMES);
	DefineValueString("BigGuns", ActorVal_BigGuns, FALLOUT_GAMES);
	DefineValueString("EnergyWeapons", ActorVal_EnergyWeapons, FALLOUT_GAMES);
	DefineValueString("Explosives", ActorVal_Explosives, FALLOUT_GAMES);
	DefineValueString("Lockpick", ActorVal_Lockpick, FALLOUT_GAMES);
	DefineValueString("Medicine", ActorVal_Medicine, FALLOUT_GAMES);
	DefineValueString("MeleeWeapons", ActorVal_MeleeWeapons, FALLOUT_GAMES);
	DefineValueString("Repair", ActorVal_Repair, FALLOUT_GAMES);
	DefineValueString("Science", ActorVal_Science, FALLOUT_GAMES);
	DefineValueString("SmallGuns", ActorVal_SmallGuns, FALLOUT_GAMES);
	DefineValueString("Sneak", ActorVal_Sneak, FALLOUT_GAMES);
	DefineValueString("Speech", ActorVal_Speech, FALLOUT_GAMES);
	//DefineValueString("Throwing", ActorVal_Throwing, FALLOUT_GAMES);
	DefineValueString("Unarmed", ActorVal_Unarmed, FALLOUT_GAMES);
	DefineValueString("InventoryWeight", ActorVal_InventoryWeight, FALLOUT_GAMES);
	DefineValueString("Paralysis", ActorVal_Paralysis, FALLOUT_GAMES);
	DefineValueString("Invisibility", ActorVal_Invisibility, FALLOUT_GAMES);
	DefineValueString("Chameleon", ActorVal_Chameleon, FALLOUT_GAMES);
	DefineValueString("NightEye", ActorVal_NightEye, FALLOUT_GAMES);
	DefineValueString("DetectLifeRange", ActorVal_DetectLifeRange, FALLOUT_GAMES);
	DefineValueString("FireResist", ActorVal_FireResistance, FALLOUT_GAMES);
	DefineValueString("WaterBreathing", ActorVal_WaterBreathing, FALLOUT_GAMES);
	DefineValueString("RadiationRads", ActorVal_RadLevel, FALLOUT_GAMES);
	DefineValueString("BloodyMess", ActorVal_BloodyMess, FALLOUT_GAMES);
	DefineValueString("UnarmedDamage", ActorVal_UnarmedDamage, FALLOUT_GAMES);
	DefineValueString("Assistance", ActorVal_Assistance, FALLOUT_GAMES);
	DefineValueString("EnergyResist", ActorVal_EnergyResistance, FALLOUT_GAMES);
	DefineValueString("EMPResist", ActorVal_EMPResistance, FALLOUT_GAMES);
	DefineValueString("Variable01", ActorVal_Var1Medical, FALLOUT_GAMES);
	DefineValueString("Variable02", ActorVal_Variable02, FALLOUT_GAMES);
	DefineValueString("Variable03", ActorVal_Variable03, FALLOUT_GAMES);
	DefineValueString("Variable04", ActorVal_Variable04, FALLOUT_GAMES);
	DefineValueString("Variable05", ActorVal_Variable05, FALLOUT_GAMES);
	DefineValueString("Variable06", ActorVal_Variable06, FALLOUT_GAMES);
	DefineValueString("Variable07", ActorVal_Variable07, FALLOUT_GAMES);
	DefineValueString("Variable08", ActorVal_Variable08, FALLOUT_GAMES);
	DefineValueString("Variable09", ActorVal_Variable09, FALLOUT_GAMES);
	DefineValueString("Variable10", ActorVal_Variable10, FALLOUT_GAMES);
	DefineValueString("IgnoreCrippledLimbs", ActorVal_IgnoreCrippledLimbs, FALLOUT_GAMES);

	DefineControl(ControlCode_Forward, FALLOUT_GAMES);
	DefineControl(ControlCode_Backward, FALLOUT_GAMES);
	DefineControl(ControlCode_Left, FALLOUT_GAMES);
	DefineControl(ControlCode_Right, FALLOUT_GAMES);
	DefineControl(ControlCode_Attack, FALLOUT_GAMES);
	DefineControl(ControlCode_Activate, FALLOUT_GAMES);
	DefineControl(ControlCode_Block, FALLOUT_GAMES);
	DefineControl(ControlCode_ReadyItem, FALLOUT_GAMES);
	DefineControl(ControlCode_Crouch, FALLOUT_GAMES);
	DefineControl(ControlCode_Run, FALLOUT_GAMES);
	DefineControl(ControlCode_AlwaysRun, FALLOUT_GAMES);
	DefineControl(ControlCode_AutoMove, FALLOUT_GAMES);
	DefineControl(ControlCode_Jump, FALLOUT_GAMES);
	DefineControl(ControlCode_TogglePOV, FALLOUT_GAMES);
	DefineControl(ControlCode_MenuMode, FALLOUT_GAMES);
	DefineControl(ControlCode_Rest, FALLOUT_GAMES);
	DefineControl(ControlCode_VATS, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey1, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey2, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey3, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey4, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey5, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey6, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey7, FALLOUT_GAMES);
	DefineControl(ControlCode_Hotkey8, FALLOUT_GAMES);
	DefineControl(ControlCode_Quicksave, FALLOUT_GAMES);
	DefineControl(ControlCode_Quickload, FALLOUT_GAMES);
	DefineControl(ControlCode_Grab, FALLOUT_GAMES);

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
	DefineFunction("PlaceAtMe", "rbIII", Func_PlaceAtMe, ALL_GAMES);
	DefineFunction("PlaceAtMeHealthPercent", "rbdIII", Func_PlaceAtMeHealthPercent, ALL_GAMES);
	DefineFunction("SetRestrained", "ri", Func_SetRestrained, ALL_GAMES);
	DefineFunction("PlayGroup", "rgi", Func_PlayGroup, ALL_GAMES);
	DefineFunction("SetAlert", "ri", Func_SetAlert, ALL_GAMES);
	DefineFunction("RemoveAllItems", "rCI", Func_RemoveAllItems, ALL_GAMES);
	DefineFunction("GetCombatTarget", "r", Func_GetCombatTarget, ALL_GAMES);
	DefineFunction("SetForceSneak", "ri", Func_SetForceSneak, ALL_GAMES);
	DefineFunction("GetActorState", "rI", Func_GetActorState, ALL_GAMES);
	DefineFunction("ChatMessage", "s", Func_Chat, ALL_GAMES);
	DefineFunction("Enable", "rI", Func_Enable, FALLOUT_GAMES);
	DefineFunction("Disable", "rI", Func_Disable, FALLOUT_GAMES);
	DefineFunction("EquipItem", "rjII", Func_EquipItem, FALLOUT_GAMES);
	DefineFunction("UnequipItem", "rjII", Func_UnequipItem, FALLOUT_GAMES);
	DefineFunction("AddItem", "rkiI", Func_AddItem, FALLOUT_GAMES);
	DefineFunction("AddItemHealthPercent", "rjidI", Func_AddItemHealthPercent, FALLOUT_GAMES);
	DefineFunction("RemoveItem", "rkiI", Func_RemoveItem, FALLOUT_GAMES);
	DefineFunction("Kill", "rQII", Func_Kill, FALLOUT_GAMES);
	DefineFunction("IsMoving", "r", Func_IsMoving, FALLOUT_GAMES);
	DefineFunction("MarkForDelete", "r", Func_MarkForDelete, FALLOUT_GAMES);
	DefineFunction("IsAnimPlaying", "rG", Func_IsAnimPlaying, FALLOUT_GAMES);
	DefineFunction("FireWeapon", "r$b", Func_FireWeapon, FALLOUT_GAMES);
	DefineFunction("GetCauseofDeath", "r", Func_GetCauseofDeath, FALLOUT_GAMES);
	DefineFunction("IsLimbGone", "ri", Func_IsLimbGone, FALLOUT_GAMES);
	DefineFunction("EnablePlayerControls", "IIIIIII", Func_EnablePlayerControls, FALLOUT_GAMES);
	DefineFunction("DisablePlayerControls", "$IIIIIII", Func_DisablePlayerControls, FALLOUT_GAMES); // $ required, else access violation...
	DefineFunction("DamageActorValue", "rvd", Func_DamageActorValue, FALLOUT_GAMES);
	DefineFunction("RestoreActorValue", "rvd", Func_RestoreActorValue, FALLOUT_GAMES);
	DefineFunction("PlayIdle", "rs", Func_PlayIdle, FALLOUT_GAMES);
	DefineFunction("MatchRace", "r$y", Func_MatchRace, FALLOUT_GAMES); // has been patched to take Race
	DefineFunction("ScanContainer", "r", Func_ScanContainer, FALLOUT_GAMES);
	DefineFunction("RemoveAllItemsEx", "r", Func_RemoveAllItemsEx, FALLOUT_GAMES);
	DefineFunction("ForceRespawn", "", Func_ForceRespawn, FALLOUT_GAMES);
	DefineFunction("UIMessage", "s", Func_UIMessage, FALLOUT_GAMES);
	DefineFunction("CenterOnCell", "$s", Func_CenterOnCell, FALLOUT_GAMES);
	DefineFunction("CenterOnExterior", "$ii", Func_CenterOnExterior, FALLOUT_GAMES);
	DefineFunction("SetINISetting", "$ss", Func_SetINISetting, FALLOUT_GAMES);

	DefineFunction("Load", "$s", Fallout3::Func_Load, FALLOUT3);
	DefineFunction("CenterOnWorld", "$wii", Fallout3::Func_CenterOnWorld, FALLOUT3);
	DefineFunction("SetName", "rsB", Fallout3::Func_SetName, FALLOUT3);
	DefineFunction("GetParentCell", "r", Fallout3::Func_GetParentCell, FALLOUT3);
	DefineFunction("GetFirstRef", "III", Fallout3::Func_GetFirstRef, FALLOUT3);
	DefineFunction("GetNextRef", "", Fallout3::Func_GetNextRef, FALLOUT3);
	DefineFunction("GetControl", "x", Fallout3::Func_GetControl, FALLOUT3);
	DefineFunction("DisableControl", "x", Fallout3::Func_DisableControl, FALLOUT3);
	DefineFunction("EnableControl", "x", Fallout3::Func_EnableControl, FALLOUT3);
	DefineFunction("DisableKey", "i", Fallout3::Func_DisableKey, FALLOUT3);
	DefineFunction("EnableKey", "i", Fallout3::Func_EnableKey, FALLOUT3);
	DefineFunction("GetRefCount", "r", Fallout3::Func_GetRefCount, FALLOUT3);
	DefineFunction("SetRefCount", "ri", Fallout3::Func_SetRefCount, FALLOUT3);
	DefineFunction("GetBaseObject", "r", Fallout3::Func_GetBaseObject, FALLOUT3);

	DefineFunction("Load", "$s", FalloutNV::Func_Load, NEWVEGAS);
	DefineFunction("CenterOnWorld", "$wii", FalloutNV::Func_CenterOnWorld, NEWVEGAS);
	DefineFunction("SetName", "rsB", FalloutNV::Func_SetName, NEWVEGAS);
	DefineFunction("GetParentCell", "r", FalloutNV::Func_GetParentCell, NEWVEGAS);
	DefineFunction("GetFirstRef", "III", FalloutNV::Func_GetFirstRef, NEWVEGAS);
	DefineFunction("GetNextRef", "", FalloutNV::Func_GetNextRef, NEWVEGAS);
	DefineFunction("GetControl", "x", FalloutNV::Func_GetControl, NEWVEGAS);
	DefineFunction("DisableControl", "x", FalloutNV::Func_DisableControl, NEWVEGAS);
	DefineFunction("EnableControl", "x", FalloutNV::Func_EnableControl, NEWVEGAS);
	DefineFunction("DisableKey", "i", FalloutNV::Func_DisableKey, NEWVEGAS);
	DefineFunction("EnableKey", "i", FalloutNV::Func_EnableKey, NEWVEGAS);
	DefineFunction("GetRefCount", "r", FalloutNV::Func_GetRefCount, NEWVEGAS);
	DefineFunction("SetRefCount", "ri", FalloutNV::Func_SetRefCount, NEWVEGAS);
	DefineFunction("GetBaseObject", "r", FalloutNV::Func_GetBaseObject, NEWVEGAS);
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
	if (!cmd || !def || !result || !*cmd || !opcode)
		throw VaultException("Invalid call to API::ParseCommand, one or more arguments are NULL (%s, %s, %04X)", cmd, def, opcode);

	vector<double> result_data;
	string _cmd(cmd);

	char* arg1_pos = reinterpret_cast<char*>(&result->arg1.unk1);
	char* arg1_end = reinterpret_cast<char*>(&result->arg1) + result->size_arg1;
	char* arg2_pos = reinterpret_cast<char*>(&result->arg2.param1);
	char* arg2_end = reinterpret_cast<char*>(&result->arg2) + result->size_arg2;
	unsigned short* _opcode = &result->arg2.opcode;
	unsigned short* _numargs = &result->arg2.numargs;

	char* tokenizer = nullptr;
	unsigned int reference = 0x00;
	result_data.emplace_back(storeIn<double, unsigned short>(opcode));

	// Skip the function name
	tokenizer = strtok(cmd, " ");

	if (*def == 'r')
	{
		tokenizer = strtok(nullptr, " ");

		if (tokenizer == nullptr)
			throw VaultException("API::ParseCommand expected a reference base operand, which could not be found");

		reference = strtoul(tokenizer, nullptr, 0);

		if (reference == 0x00)
			throw VaultException("API::ParseCommand reference base operand is NULL (%s, %s, %04X)", _cmd.c_str(), def, opcode);

		result->arg3.reference = reference;
		result_data.emplace_back(storeIn<double, unsigned int>(reference));
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

			case 'k': // Object ID base item
				typecode = 0x00000032;
				break;

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
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
			w (World space, 2 byte, 0x72, stream) - 0x0000001B
			q (Actor, 2 byte, 0x72, stream) - 0x00000006
			y (Race, 2 byte, 0x72, stream) - 0x0000000F
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
				unsigned int integer = strtoul(tokenizer, nullptr, 0);

				if (tolower(type) == 'x' && !IsControl((unsigned char) integer))
					throw VaultException("API::ParseCommand could not find a control code for input %s", tokenizer);

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x6E;
				*reinterpret_cast<unsigned int*>(arg2_pos + sizeof(unsigned char)) = integer;
				result_data.emplace_back(storeIn<double, unsigned int>(integer));
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
			{
				if (refparam != 0x00)   // We don't support more than one refparam yet
					throw VaultException("API::ParseCommand does only support one reference argument up until now");

				refparam = strtoul(tokenizer, nullptr, 0);

				if (!refparam)
					throw VaultException("API::ParseCommand reference argument is NULL");

				*reinterpret_cast<unsigned char*>(arg2_pos) = 0x72;
				*reinterpret_cast<unsigned short*>(arg2_pos + sizeof(unsigned char)) = (!reference || refparam == reference) ? 0x0001 : 0x0002;
				result_data.emplace_back(storeIn<double, unsigned int>(refparam));
				arg2_pos += sizeof(unsigned char) + sizeof(unsigned short);
				break;
			}

			case 'v': // Actor Value
			{
				unsigned char value = RetrieveValue(tokenizer);

				if (value == 0xFF)
					throw VaultException("API::ParseCommand could not find an Actor Value identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) value;
				result_data.emplace_back(storeIn<double, unsigned short>(value));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 'a': // Axis
			{
				unsigned char axis = RetrieveAxis(tokenizer);

				if (axis == 0xFF)
					throw VaultException("API::ParseCommand could not find an Axis identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned char*>(arg2_pos) = axis;
				result_data.emplace_back(storeIn<double, unsigned char>(axis));
				arg2_pos += sizeof(unsigned char);
				break;
			}

			case 'g': // Animation Group
			{
				unsigned char anim = RetrieveAnim(tokenizer);

				if (anim == 0xFF)
					throw VaultException("API::ParseCommand could not find an Animation identifier for input %s", tokenizer);

				*reinterpret_cast<unsigned short*>(arg2_pos) = (unsigned short) anim;
				result_data.emplace_back(storeIn<double, unsigned short>(anim));
				arg2_pos += sizeof(unsigned short);
				break;
			}

			case 's': // String
			{
				string str = Utils::str_replace(tokenizer, "|", " ");

				unsigned short length = (unsigned short) str.length();

				*reinterpret_cast<unsigned short*>(arg2_pos) = length;
				memcpy(arg2_pos + sizeof(unsigned short), str.c_str(), length +  sizeof(unsigned char));
				result_data.emplace_back(0); // Don't pass on string for now
				arg2_pos += sizeof(unsigned short);
				arg2_pos += length;
				break;
			}

			default:
				throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
		}

		if (reinterpret_cast<unsigned int>(arg1_end) - reinterpret_cast<unsigned int>(arg1_pos) < 0)
			throw VaultException("API::ParseCommand argument #1 size overrun while processing %s", _cmd.c_str());

		if (reinterpret_cast<unsigned int>(arg2_end) - reinterpret_cast<unsigned int>(arg2_pos) < 0)
			throw VaultException("API::ParseCommand argument #2 size overrun while processing %s", _cmd.c_str());

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
		result.emplace_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllAxis()
{
	vector<unsigned char> result;
	ValueMap::iterator it;

	for (it = axis.begin(); it != axis.end(); ++it)
		result.emplace_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllAnims()
{
	vector<unsigned char> result;
	ValueMap::iterator it;

	for (it = anims.begin(); it != anims.end(); ++it)
		result.emplace_back(it->second);

	return result;
}

vector<unsigned char> API::RetrieveAllControls()
{
	vector<unsigned char> result;
	ValueList::iterator it;

	for (it = controls.begin(); it != controls.end(); ++it)
		result.emplace_back(*it);

	return result;
}

vector<string> API::RetrieveAllValues_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = values.begin(); it != values.end(); ++it)
		result.emplace_back(it->first);

	return result;
}

vector<string> API::RetrieveAllAxis_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = axis.begin(); it != axis.end(); ++it)
		result.emplace_back(it->first);

	return result;
}

vector<string> API::RetrieveAllAnims_Reverse()
{
	vector<string> result;
	ValueMap::iterator it;

	for (it = anims.begin(); it != anims.end(); ++it)
		result.emplace_back(it->first);

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

unsigned char* API::BuildCommandStream(vector<double>&& info, unsigned int key, unsigned char* command, unsigned int size)
{
	if (size + 5 > PIPE_LENGTH)
		throw VaultException("Error in API class; command size (%d bytes) exceeds the pipe length of %d bytes", size + 5, PIPE_LENGTH);

	unsigned char* data = new unsigned char[PIPE_LENGTH];
	ZeroMemory(data, sizeof(data));
	data[0] = PIPE_OP_COMMAND;

	memcpy(data + 5, command, size);

	unsigned int r = rand();
	*reinterpret_cast<unsigned int*>(data + 1) = r;
	queue.emplace_front(r, move(info), key);

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
		unsigned char* data = BuildCommandStream(move(parsed), key, reinterpret_cast<unsigned char*>(&result), sizeof(op_default));
		stream.emplace_back(data);
	}

	return stream;
}

vector<CommandResult> API::Translate(unsigned char* stream)
{
	if (stream[0] != PIPE_OP_RETURN && stream[0] != PIPE_OP_RETURN_BIG && stream[0] != PIPE_OP_RETURN_RAW)
		throw VaultException("API could not recognize stream identifier %02X", stream[0]);

	vector<CommandResult> result;

	if (stream[0] != PIPE_OP_RETURN_RAW)
	{
		unsigned int r = *reinterpret_cast<unsigned int*>(stream + 1);

		while (!queue.empty() && get<0>(queue.back()) != r)
		{
			auto& element = queue.back();

	#ifdef VAULTMP_DEBUG

			if (debug)
				debug->PrintFormat("API did not retrieve the result of command with identifier %08X (opcode %04hX)", true, get<0>(element), getFrom<double, unsigned short>(get<1>(element).at(0)));

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

			if (debug)
				debug->PrintFormat("API could not find a stored command with identifier %08X (queue is empty)", true, r);

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
		get<2>(_result) = storeIn<double, vector<unsigned char>*>(big);
	}
	else
		get<2>(_result) = *reinterpret_cast<double*>(data);

	get<3>(_result) = false;

	if (stream[0] != PIPE_OP_RETURN_RAW)
		queue.pop_back();

	return result;
}

unsigned char API::GetGameCode()
{
	return game;
}
