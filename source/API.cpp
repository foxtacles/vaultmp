#include "API.h"

using namespace std;
using namespace Values;

API::FunctionMap API::functions;
API::ValueMap API::values;
API::ValueMap API::axis;
API::ValueMap API::anims;
API::ValueList API::controls;
API::CommandQueue API::queue;

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

	DefineAxisString("X", Axis_X);
	DefineAxisString("Y", Axis_Y);
	DefineAxisString("Z", Axis_Z);

	DefineAnimString("Idle", AnimGroup_Idle);
	DefineAnimString("DynamicIdle", AnimGroup_DynamicIdle);
	DefineAnimString("SpecialIdle", AnimGroup_SpecialIdle);
	DefineAnimString("Forward", AnimGroup_Forward);
	DefineAnimString("Backward", AnimGroup_Backward);
	DefineAnimString("Left", AnimGroup_Left);
	DefineAnimString("Right", AnimGroup_Right);
	DefineAnimString("FastForward", AnimGroup_FastForward);
	DefineAnimString("FastBackward", AnimGroup_FastBackward);
	DefineAnimString("FastLeft", AnimGroup_FastLeft);
	DefineAnimString("FastRight", AnimGroup_FastRight);
	DefineAnimString("DodgeForward", AnimGroup_DodgeForward);
	DefineAnimString("DodgeBack", AnimGroup_DodgeBack);
	DefineAnimString("DodgeLeft", AnimGroup_DodgeLeft);
	DefineAnimString("DodgeRight", AnimGroup_DodgeRight);
	DefineAnimString("TurnLeft", AnimGroup_TurnLeft);
	DefineAnimString("TurnRight", AnimGroup_TurnRight);
	DefineAnimString("Aim", AnimGroup_Aim);
	DefineAnimString("AimUp", AnimGroup_AimUp);
	DefineAnimString("AimDown", AnimGroup_AimDown);
	DefineAnimString("AimIS", AnimGroup_AimIS);
	DefineAnimString("AimISUp", AnimGroup_AimISUp);
	DefineAnimString("AimISDown", AnimGroup_AimISDown);
	DefineAnimString("AttackLeft", AnimGroup_AttackLeft);
	DefineAnimString("AttackLeftUp", AnimGroup_AttackLeftUp);
	DefineAnimString("AttackLeftDown", AnimGroup_AttackLeftDown);
	DefineAnimString("AttackLeftIS", AnimGroup_AttackLeftIS);
	DefineAnimString("AttackLeftISUp", AnimGroup_AttackLeftISUp);
	DefineAnimString("AttackLeftISDown", AnimGroup_AttackLeftISDown);
	DefineAnimString("AttackRight", AnimGroup_AttackRight);
	DefineAnimString("AttackRightUp", AnimGroup_AttackRightUp);
	DefineAnimString("AttackRightDown", AnimGroup_AttackRightDown);
	DefineAnimString("AttackRightIS", AnimGroup_AttackRightIS);
	DefineAnimString("AttackRightISUp", AnimGroup_AttackRightISUp);
	DefineAnimString("AttackRightISDown", AnimGroup_AttackRightISDown);
	DefineAnimString("Attack3", AnimGroup_Attack3);
	DefineAnimString("Attack3Up", AnimGroup_Attack3Up);
	DefineAnimString("Attack3Down", AnimGroup_Attack3Down);
	DefineAnimString("Attack3IS", AnimGroup_Attack3IS);
	DefineAnimString("Attack3ISUp", AnimGroup_Attack3ISUp);
	DefineAnimString("Attack3ISDown", AnimGroup_Attack3ISDown);
	DefineAnimString("Attack4", AnimGroup_Attack4);
	DefineAnimString("Attack4Up", AnimGroup_Attack4Up);
	DefineAnimString("Attack4Down", AnimGroup_Attack4Down);
	DefineAnimString("Attack4IS", AnimGroup_Attack4IS);
	DefineAnimString("Attack4ISUp", AnimGroup_Attack4ISUp);
	DefineAnimString("Attack4ISDown", AnimGroup_Attack4ISDown);
	DefineAnimString("Attack5", AnimGroup_Attack5);
	DefineAnimString("Attack5Up", AnimGroup_Attack5Up);
	DefineAnimString("Attack5Down", AnimGroup_Attack5Down);
	DefineAnimString("Attack5IS", AnimGroup_Attack5IS);
	DefineAnimString("Attack5ISUp", AnimGroup_Attack5ISUp);
	DefineAnimString("Attack5ISDown", AnimGroup_Attack5ISDown);
	DefineAnimString("Attack6", AnimGroup_Attack6);
	DefineAnimString("Attack6Up", AnimGroup_Attack6Up);
	DefineAnimString("Attack6Down", AnimGroup_Attack6Down);
	DefineAnimString("Attack6IS", AnimGroup_Attack6IS);
	DefineAnimString("Attack6ISUp", AnimGroup_Attack6ISUp);
	DefineAnimString("Attack6ISDown", AnimGroup_Attack6ISDown);
	DefineAnimString("Attack7", AnimGroup_Attack7);
	DefineAnimString("Attack7Up", AnimGroup_Attack7Up);
	DefineAnimString("Attack7Down", AnimGroup_Attack7Down);
	DefineAnimString("Attack7IS", AnimGroup_Attack7IS);
	DefineAnimString("Attack7ISUp", AnimGroup_Attack7ISUp);
	DefineAnimString("Attack7ISDown", AnimGroup_Attack7ISDown);
	DefineAnimString("Attack8", AnimGroup_Attack8);
	DefineAnimString("Attack8Up", AnimGroup_Attack8Up);
	DefineAnimString("Attack8Down", AnimGroup_Attack8Down);
	DefineAnimString("Attack8IS", AnimGroup_Attack8IS);
	DefineAnimString("Attack8ISUp", AnimGroup_Attack8ISUp);
	DefineAnimString("Attack8ISDown", AnimGroup_Attack8ISDown);
	DefineAnimString("AttackLoop", AnimGroup_AttackLoop);
	DefineAnimString("AttackLoopUp", AnimGroup_AttackLoopUp);
	DefineAnimString("AttackLoopDown", AnimGroup_AttackLoopDown);
	DefineAnimString("AttackLoopIS", AnimGroup_AttackLoopIS);
	DefineAnimString("AttackLoopISUp", AnimGroup_AttackLoopISUp);
	DefineAnimString("AttackLoopISDown", AnimGroup_AttackLoopISDown);
	DefineAnimString("AttackSpin", AnimGroup_AttackSpin);
	DefineAnimString("AttackSpinUp", AnimGroup_AttackSpinUp);
	DefineAnimString("AttackSpinDown", AnimGroup_AttackSpinDown);
	DefineAnimString("AttackSpinIS", AnimGroup_AttackSpinIS);
	DefineAnimString("AttackSpinISUp", AnimGroup_AttackSpinISUp);
	DefineAnimString("AttackSpinISDown", AnimGroup_AttackSpinISDown);
	DefineAnimString("AttackSpin2", AnimGroup_AttackSpin2);
	DefineAnimString("AttackSpin2Up", AnimGroup_AttackSpin2Up);
	DefineAnimString("AttackSpin2Down", AnimGroup_AttackSpin2Down);
	DefineAnimString("AttackSpin2IS", AnimGroup_AttackSpin2IS);
	DefineAnimString("AttackSpin2ISUp", AnimGroup_AttackSpin2ISUp);
	DefineAnimString("AttackSpin2ISDown", AnimGroup_AttackSpin2ISDown);
	DefineAnimString("AttackPower", AnimGroup_AttackPower);
	DefineAnimString("AttackForwardPower", AnimGroup_AttackForwardPower);
	DefineAnimString("AttackBackPower", AnimGroup_AttackBackPower);
	DefineAnimString("AttackLeftPower", AnimGroup_AttackLeftPower);
	DefineAnimString("AttackRightPower", AnimGroup_AttackRightPower);
	DefineAnimString("Holster", AnimGroup_Holster);
	DefineAnimString("Equip", AnimGroup_Equip);
	DefineAnimString("Unequip", AnimGroup_Unequip);

	DefineAnimString("JumpStart", AnimGroup_JumpStart);
	DefineAnimString("JumpLoop", AnimGroup_JumpLoop);
	DefineAnimString("JumpLand", AnimGroup_JumpLand);
	DefineAnimString("JumpLoopForward", AnimGroup_JumpLoopForward);
	DefineAnimString("JumpLoopBackward", AnimGroup_JumpLoopBackward);
	DefineAnimString("JumpLoopLeft", AnimGroup_JumpLoopLeft);
	DefineAnimString("JumpLoopRight", AnimGroup_JumpLoopRight);
	DefineAnimString("JumpLandForward", AnimGroup_JumpLandForward);
	DefineAnimString("JumpLandBackward", AnimGroup_JumpLandBackward);
	DefineAnimString("JumpLandLeft", AnimGroup_JumpLandLeft);
	DefineAnimString("JumpLandRight", AnimGroup_JumpLandRight);
	DefineAnimString("BlockIdle", AnimGroup_BlockIdle);
	DefineAnimString("BlockHit", AnimGroup_BlockHit);
	DefineAnimString("ReloadA", AnimGroup_ReloadA);
	DefineAnimString("ReloadB", AnimGroup_ReloadB);
	DefineAnimString("ReloadC", AnimGroup_ReloadC);
	DefineAnimString("ReloadD", AnimGroup_ReloadD);
	DefineAnimString("ReloadE", AnimGroup_ReloadE);
	DefineAnimString("ReloadF", AnimGroup_ReloadF);
	DefineAnimString("ReloadG", AnimGroup_ReloadG);
	DefineAnimString("ReloadH", AnimGroup_ReloadH);
	DefineAnimString("ReloadI", AnimGroup_ReloadI);
	DefineAnimString("ReloadJ", AnimGroup_ReloadJ);
	DefineAnimString("ReloadK", AnimGroup_ReloadK);

	DefineValueString("Aggression", ActorVal_Aggression);
	DefineValueString("Confidence", ActorVal_Confidence);
	DefineValueString("Energy", ActorVal_Energy);
	DefineValueString("Responsibility", ActorVal_Responsibility);
	DefineValueString("Mood", ActorVal_Mood);
	DefineValueString("Strength", ActorVal_Strength);
	DefineValueString("Perception", ActorVal_Perception);
	DefineValueString("Endurance", ActorVal_Endurance);
	DefineValueString("Charisma", ActorVal_Charisma);
	DefineValueString("Intelligence", ActorVal_Intelligence);
	DefineValueString("Agility", ActorVal_Agility);
	DefineValueString("Luck", ActorVal_Luck);
	DefineValueString("ActionPoints", ActorVal_ActionPoints);
	DefineValueString("CarryWeight", ActorVal_CarryWeight);
	DefineValueString("CritChance", ActorVal_CritChance);
	DefineValueString("HealRate", ActorVal_HealRate);
	DefineValueString("Health", ActorVal_Health);
	DefineValueString("MeleeDamage", ActorVal_MeleeDamage);
	DefineValueString("DamageResist", ActorVal_DamageResistance);
	DefineValueString("PoisonResist", ActorVal_PoisonResistance);
	DefineValueString("RadResist", ActorVal_RadResistance);
	DefineValueString("SpeedMult", ActorVal_SpeedMultiplier);
	DefineValueString("Fatigue", ActorVal_Fatigue);
	DefineValueString("Karma", ActorVal_Karma);
	DefineValueString("XP", ActorVal_XP);
	DefineValueString("PerceptionCondition", ActorVal_Head);
	DefineValueString("EnduranceCondition", ActorVal_Torso);
	DefineValueString("LeftAttackCondition", ActorVal_LeftArm);
	DefineValueString("RightAttackCondition", ActorVal_RightArm);
	DefineValueString("LeftMobilityCondition", ActorVal_LeftLeg);
	DefineValueString("RightMobilityCondition", ActorVal_RightLeg);
	DefineValueString("BrainCondition", ActorVal_Brain);
	DefineValueString("Barter", ActorVal_Barter);
	DefineValueString("BigGuns", ActorVal_BigGuns);
	DefineValueString("EnergyWeapons", ActorVal_EnergyWeapons);
	DefineValueString("Explosives", ActorVal_Explosives);
	DefineValueString("Lockpick", ActorVal_Lockpick);
	DefineValueString("Medicine", ActorVal_Medicine);
	DefineValueString("MeleeWeapons", ActorVal_MeleeWeapons);
	DefineValueString("Repair", ActorVal_Repair);
	DefineValueString("Science", ActorVal_Science);
	DefineValueString("SmallGuns", ActorVal_SmallGuns);
	DefineValueString("Sneak", ActorVal_Sneak);
	DefineValueString("Speech", ActorVal_Speech);
	//DefineValueString("Throwing", ActorVal_Throwing);
	DefineValueString("Unarmed", ActorVal_Unarmed);
	DefineValueString("InventoryWeight", ActorVal_InventoryWeight);
	DefineValueString("Paralysis", ActorVal_Paralysis);
	DefineValueString("Invisibility", ActorVal_Invisibility);
	DefineValueString("Chameleon", ActorVal_Chameleon);
	DefineValueString("NightEye", ActorVal_NightEye);
	DefineValueString("DetectLifeRange", ActorVal_DetectLifeRange);
	DefineValueString("FireResist", ActorVal_FireResistance);
	DefineValueString("WaterBreathing", ActorVal_WaterBreathing);
	DefineValueString("RadiationRads", ActorVal_RadLevel);
	DefineValueString("BloodyMess", ActorVal_BloodyMess);
	DefineValueString("UnarmedDamage", ActorVal_UnarmedDamage);
	DefineValueString("Assistance", ActorVal_Assistance);
	DefineValueString("EnergyResist", ActorVal_EnergyResistance);
	DefineValueString("EMPResist", ActorVal_EMPResistance);
	DefineValueString("Variable01", ActorVal_Var1Medical);
	DefineValueString("Variable02", ActorVal_Variable02);
	DefineValueString("Variable03", ActorVal_Variable03);
	DefineValueString("Variable04", ActorVal_Variable04);
	DefineValueString("Variable05", ActorVal_Variable05);
	DefineValueString("Variable06", ActorVal_Variable06);
	DefineValueString("Variable07", ActorVal_Variable07);
	DefineValueString("Variable08", ActorVal_Variable08);
	DefineValueString("Variable09", ActorVal_Variable09);
	DefineValueString("Variable10", ActorVal_Variable10);
	DefineValueString("IgnoreCrippledLimbs", ActorVal_IgnoreCrippledLimbs);

	DefineControl(ControlCode_Forward);
	DefineControl(ControlCode_Backward);
	DefineControl(ControlCode_Left);
	DefineControl(ControlCode_Right);
	DefineControl(ControlCode_Attack);
	DefineControl(ControlCode_Activate);
	DefineControl(ControlCode_Block);
	DefineControl(ControlCode_ReadyItem);
	DefineControl(ControlCode_Crouch);
	DefineControl(ControlCode_Run);
	DefineControl(ControlCode_AlwaysRun);
	DefineControl(ControlCode_AutoMove);
	DefineControl(ControlCode_Jump);
	DefineControl(ControlCode_TogglePOV);
	DefineControl(ControlCode_MenuMode);
	DefineControl(ControlCode_Rest);
	DefineControl(ControlCode_VATS);
	DefineControl(ControlCode_Hotkey1);
	DefineControl(ControlCode_Hotkey2);
	DefineControl(ControlCode_Hotkey3);
	DefineControl(ControlCode_Hotkey4);
	DefineControl(ControlCode_Hotkey5);
	DefineControl(ControlCode_Hotkey6);
	DefineControl(ControlCode_Hotkey7);
	DefineControl(ControlCode_Hotkey8);
	DefineControl(ControlCode_Quicksave);
	DefineControl(ControlCode_Quickload);
	DefineControl(ControlCode_Grab);

	DefineFunction("GetPos", "ra", Func_GetPos);
	DefineFunction("SetPos", "rad", Func_SetPos);
	DefineFunction("GetAngle", "ra", Func_GetAngle);
	DefineFunction("SetAngle", "rad", Func_SetAngle);
	DefineFunction("GetBaseActorValue", "rv", Func_GetBaseActorValue);
	DefineFunction("SetActorValue", "rvi", Func_SetActorValue);
	DefineFunction("GetActorValue", "rv", Func_GetActorValue);
	DefineFunction("ForceActorValue", "rvi", Func_ForceActorValue);
	DefineFunction("GetDead", "r", Func_GetDead);
	DefineFunction("MoveTo", "roDDD", Func_MoveTo);
	DefineFunction("PlaceAtMe", "rbIII", Func_PlaceAtMe);
	DefineFunction("PlaceAtMeHealthPercent", "rbdIII", Func_PlaceAtMeHealthPercent);
	DefineFunction("SetRestrained", "ri", Func_SetRestrained);
	DefineFunction("PlayGroup", "rgi", Func_PlayGroup);
	DefineFunction("SetAlert", "ri", Func_SetAlert);
	DefineFunction("RemoveAllItems", "rCI", Func_RemoveAllItems);
	DefineFunction("GetCombatTarget", "r", Func_GetCombatTarget);
	DefineFunction("SetForceSneak", "ri", Func_SetForceSneak);
	DefineFunction("GetActorState", "rI", Func_GetActorState);
	DefineFunction("ChatMessage", "s", Func_Chat);
	DefineFunction("ChatUpdate", "iidddd", Func_Chatbox);
	DefineFunction("Enable", "rI", Func_Enable);
	DefineFunction("Disable", "rI", Func_Disable);
	DefineFunction("EquipItem", "rjII", Func_EquipItem);
	DefineFunction("UnequipItem", "rjII", Func_UnequipItem);
	DefineFunction("AddItem", "rkiI", Func_AddItem);
	DefineFunction("AddItemHealthPercent", "rjidI", Func_AddItemHealthPercent);
	DefineFunction("RemoveItem", "rkiI", Func_RemoveItem);
	DefineFunction("Kill", "rQII", Func_Kill);
	DefineFunction("IsMoving", "r", Func_IsMoving);
	DefineFunction("MarkForDelete", "r", Func_MarkForDelete);
	DefineFunction("IsAnimPlaying", "rG", Func_IsAnimPlaying);
	DefineFunction("FireWeapon", "r$b", Func_FireWeapon);
	DefineFunction("GetCauseofDeath", "r", Func_GetCauseofDeath);
	DefineFunction("IsLimbGone", "ri", Func_IsLimbGone);
	DefineFunction("EnablePlayerControls", "IIIIIII", Func_EnablePlayerControls);
	DefineFunction("DisablePlayerControls", "$IIIIIII", Func_DisablePlayerControls); // $ required, else access violation...
	DefineFunction("DamageActorValue", "rvd", Func_DamageActorValue);
	DefineFunction("RestoreActorValue", "rvd", Func_RestoreActorValue);
	DefineFunction("PlayIdle", "rs", Func_PlayIdle);
	DefineFunction("AgeRace", "r$i", Func_AgeRace);
	DefineFunction("MatchRace", "r$y", Func_MatchRace); // has been patched to take Race
	DefineFunction("SexChange", "r$I", Func_SexChange);
	DefineFunction("ForceWeather", "nI", Func_ForceWeather);
	DefineFunction("ScanContainer", "r", Func_ScanContainer);
	DefineFunction("RemoveAllItemsEx", "r", Func_RemoveAllItemsEx);
	DefineFunction("ForceRespawn", "", Func_ForceRespawn);
	DefineFunction("SetGlobalValue", "ri", Func_SetGlobalValue);
	DefineFunction("UIMessage", "si", Func_UIMessage);
	DefineFunction("Lock", "rII", Func_Lock);
	DefineFunction("Unlock", "r", Func_Unlock);
	DefineFunction("SetOwnership", "rF", Func_SetOwnership);
	DefineFunction("GetLocked", "r", Func_GetLocked);
	DefineFunction("CenterOnCell", "$s", Func_CenterOnCell);
	DefineFunction("CenterOnExterior", "$ii", Func_CenterOnExterior);
	DefineFunction("SetINISetting", "$ss", Func_SetINISetting);

	DefineFunction("Load", "$s", Func_Load);
	DefineFunction("CenterOnWorld", "$wii", Func_CenterOnWorld);
	DefineFunction("SetName", "rsB", Func_SetName);
	DefineFunction("GetParentCell", "r", Func_GetParentCell);
	DefineFunction("GetFirstRef", "III", Func_GetFirstRef);
	DefineFunction("GetNextRef", "", Func_GetNextRef);
	DefineFunction("GetControl", "x", Func_GetControl);
	DefineFunction("DisableControl", "x", Func_DisableControl);
	DefineFunction("EnableControl", "x", Func_EnableControl);
	DefineFunction("DisableKey", "i", Func_DisableKey);
	DefineFunction("EnableKey", "i", Func_EnableKey);
	DefineFunction("GetRefCount", "r", Func_GetRefCount);
	DefineFunction("SetRefCount", "ri", Func_SetRefCount);
	DefineFunction("GetBaseObject", "r", Func_GetBaseObject);
	DefineFunction("SetCurrentHealth", "rd", Func_SetCurrentHealth);
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
				string str = Utils::str_replace(tokenizer, "|", " ");

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

void API::DefineFunction(const string& name, const string& def, unsigned short opcode)
{
	functions.emplace(name, pair<string, unsigned short>(def, opcode));
}

void API::DefineValueString(const string& name, unsigned char value)
{
	values.emplace(name, value);
}

void API::DefineAxisString(const string& name, unsigned char axis)
{
	API::axis.emplace(name, axis);
}

void API::DefineAnimString(const string& name, unsigned char anim)
{
	anims.emplace(name, anim);
}

void API::DefineControl(unsigned char control)
{
	controls.insert(control);
}

unsigned char API::RetrieveValue(const char* value)
{
	ValueMap::iterator it;
	it = values.find(string(value));

	if (it != values.end())
		return it->second;

	return 0xFF;
}

unsigned char API::RetrieveAxis(const char* axis)
{
	ValueMap::iterator it;
	it = API::axis.find(string(axis));

	if (it != API::axis.end())
		return it->second;

	return 0xFF;
}

unsigned char API::RetrieveAnim(const char* anim)
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

pair<string, unsigned short> API::RetrieveFunction(const string& name)
{
	auto it = functions.find(name);

	if (it != functions.end())
		return it->second;

	auto empty = pair<string, unsigned short>("", 0x00);

	return empty;
}

bool API::AnnounceFunction(const string& name)
{
	pair<string, unsigned short> func = RetrieveFunction(name);

	if (!func.first.empty())
		return true;

#ifdef VAULTMP_DEBUG
	debug.print("API function ", name.c_str(), " not found or not supported by the game");
#endif

	return false;
}

unsigned char* API::BuildCommandStream(vector<double>&& info, unsigned int key, unsigned char* command, unsigned int size)
{
	if (size + 5 > PIPE_LENGTH)
		throw VaultException("Error in API class; command size (%d bytes) exceeds the pipe length of %d bytes", size + 5, PIPE_LENGTH).stacktrace();

	unsigned char* data = new unsigned char[PIPE_LENGTH];
	ZeroMemory(data, sizeof(data));
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
		pair<string, unsigned short> func = RetrieveFunction(command.substr(0, command.find_first_of(' ')));

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
