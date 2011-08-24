#include "API.h"

using namespace std;
using namespace Values;

FunctionList API::functions;
ValueList API::values;
ValueList API::axis;
ValueList API::anims;
CommandQueue API::queue;
API::CommandCache API::cache;
int API::game = 0;

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
    unsigned int unk1; // console / script? pure console functions don't have this
    unsigned short opcode; // important
    unsigned short unk2; // varies but not read
    unsigned short numargs; // number of arguments passed
    unsigned char param1;
    unsigned char unk4;
    double param2;
    double param3;
    double param4;
    double param5;
    double param6;

    // Arguments are typed and identified (partially) with a preceding byte:
    // 6E - Integer, 4 byte
    // 7A - Double, 8 byte
    // 72 - followed by 2 byte, which tell something about the amount of reference parameters passed in Arg5? or the position of the reference in the stream

    op_Arg2()
    {
        unk1 = 0x0001001C; // this is the value when run from console
        param2 = 0x0000000000000000;
        param3 = 0x0000000000000000;
        param4 = 0x0000000000000000;
        param5 = 0x0000000000000000;
        param6 = 0x0000000000000000;
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
    unsigned int unk1;
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
    unsigned int unk12;
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
        unk1 = 0x00000000;
        unk2 = 0x00000000;
        unk3 = 0x0000400A;
        unk4 = 0x00000000;

        numargs = 0x00000000;

        param1_reftext = 0x00000000;
        param1_unk2 = 0x00000000;
        param1_reference = 0x00000000;
        param1_unk4 = 0x00000000;

        param2_reftext = 0x00000000;
        param2_unk2 = 0x00000000;
        param2_reference = 0x00000000;
        param2_unk4 = 0x00000000;

        param1 = (unsigned int*) ((unsigned) &param1_reftext - (unsigned) &unk1);
        param2 = (unsigned int**) ((unsigned) &param2_real - (unsigned) &unk1);
        param2_real = (unsigned int*) ((unsigned) &param2_reftext - (unsigned) &unk1);
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
    unsigned int unk1; // function type? usually 0x00000008, console functions have 0x00000004, is read

    op_Arg8()
    {
        unk1 = 0x00000008;
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
    unsigned int random;

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
        random = 0x00000000; // make sure the command gets a "unique" CRC
    }
};

#pragma pack(pop)

API::API()
{

}

void API::Initialize(int game)
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

    DefineAnimString("Equip", Oblivion::AnimGroup_Equip, OBLIVION);
    DefineAnimString("Unequip", Oblivion::AnimGroup_Unequip, OBLIVION);
    DefineAnimString("AttackBow", Oblivion::AnimGroup_AttackBow, OBLIVION);
    DefineAnimString("AttackLeft", Oblivion::AnimGroup_AttackLeft, OBLIVION);
    DefineAnimString("AttackRight", Oblivion::AnimGroup_AttackRight, OBLIVION);
    DefineAnimString("AttackPower", Oblivion::AnimGroup_AttackPower, OBLIVION);
    DefineAnimString("AttackForwardPower", Oblivion::AnimGroup_AttackForwardPower, OBLIVION);
    DefineAnimString("AttackBackPower", Oblivion::AnimGroup_AttackBackPower, OBLIVION);
    DefineAnimString("AttackLeftPower", Oblivion::AnimGroup_AttackLeftPower, OBLIVION);
    DefineAnimString("AttackRightPower", Oblivion::AnimGroup_AttackRightPower, OBLIVION);
    DefineAnimString("BlockIdle", Oblivion::AnimGroup_BlockIdle, OBLIVION);
    DefineAnimString("BlockHit", Oblivion::AnimGroup_BlockHit, OBLIVION);
    DefineAnimString("BlockAttack", Oblivion::AnimGroup_BlockAttack, OBLIVION);
    DefineAnimString("Recoil", Oblivion::AnimGroup_Recoil, OBLIVION);
    DefineAnimString("Stagger", Oblivion::AnimGroup_Stagger, OBLIVION);
    DefineAnimString("Death", Oblivion::AnimGroup_Death, OBLIVION);
    DefineAnimString("TorchIdle", Oblivion::AnimGroup_TorchIdle, OBLIVION);
    DefineAnimString("CastSelf", Oblivion::AnimGroup_CastSelf, OBLIVION);
    DefineAnimString("CastTouch", Oblivion::AnimGroup_CastTouch, OBLIVION);
    DefineAnimString("CastTarget", Oblivion::AnimGroup_CastTarget, OBLIVION);
    DefineAnimString("CastSelfAlt", Oblivion::AnimGroup_CastSelfAlt, OBLIVION);
    DefineAnimString("CastTouchAlt", Oblivion::AnimGroup_CastTouchAlt, OBLIVION);
    DefineAnimString("CastTargetAlt", Oblivion::AnimGroup_CastTargetAlt, OBLIVION);
    DefineAnimString("JumpStart", Oblivion::AnimGroup_JumpStart, OBLIVION);
    DefineAnimString("JumpLoop", Oblivion::AnimGroup_JumpLoop, OBLIVION);
    DefineAnimString("JumpLand", Oblivion::AnimGroup_JumpLand, OBLIVION);

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

    DefineFunction("Enable", "rI", Func_Enable, FALLOUT_GAMES);
    DefineFunction("Disable", "rI", Func_Disable, FALLOUT_GAMES);
    DefineFunction("EquipItem", "rjII", Func_EquipItem, FALLOUT_GAMES);
    DefineFunction("UnequipItem", "rjII", Func_UnequipItem, FALLOUT_GAMES);
    DefineFunction("AddItem", "rkiI", Func_AddItem, FALLOUT_GAMES);
    DefineFunction("RemoveItem", "rkiI", Func_RemoveItem, FALLOUT_GAMES);
    DefineFunction("Kill", "rQII", Func_Kill, FALLOUT_GAMES);
    DefineFunction("IsMoving", "r", Fallout::Func_IsMoving, FALLOUT_GAMES);
    DefineFunction("MarkForDelete", "r", Fallout::Func_MarkForDelete, FALLOUT_GAMES);

    DefineFunction("Load", "s", Fallout3::Func_Load, FALLOUT3);
    DefineFunction("SetName", "rsB", Fallout3::Func_SetName, FALLOUT3);
    DefineFunction("GetParentCell", "r", Fallout3::Func_GetParentCell, FALLOUT3);

    DefineFunction("Load", "s", FalloutNV::Func_Load, NEWVEGAS);
    DefineFunction("SetName", "rsB", FalloutNV::Func_SetName, NEWVEGAS);
    DefineFunction("GetParentCell", "r", FalloutNV::Func_GetParentCell, NEWVEGAS);

    DefineFunction("Enable", "r", Func_Enable, OBLIVION);
    DefineFunction("Disable", "r", Func_Disable, OBLIVION);
    DefineFunction("EquipItem", "rjI", Func_EquipItem, OBLIVION);
    DefineFunction("UnequipItem", "rjI", Func_UnequipItem, OBLIVION);
    DefineFunction("AddItem", "rki", Func_AddItem, OBLIVION);
    DefineFunction("RemoveItem", "rki", Func_RemoveItem, OBLIVION);
    DefineFunction("Kill", "rQ", Func_Kill, OBLIVION);
    DefineFunction("Load", "s", Oblivion::Func_Load, OBLIVION);
    DefineFunction("SetName", "rsB", Oblivion::Func_SetName, OBLIVION);
    DefineFunction("GetParentCell", "r", Oblivion::Func_GetParentCell, OBLIVION);
}

void API::Terminate()
{
    values.clear();
    axis.clear();
    anims.clear();
    functions.clear();
    queue.clear();
    cache.clear();
}

#ifdef VAULTMP_DEBUG
void API::SetDebugHandler(Debug* debug)
{
    API::debug = debug;

    if (debug != NULL)
        debug->Print("Attached debug handler to API class", true);
}
#endif

pair<vector<double>, API::op_default*> API::ParseCommand(char* cmd, char* def, unsigned short opcode)
{
    pair<vector<double>, op_default*> result_data = pair<vector<double>, op_default*>(vector<double>(), NULL);

    if (*cmd == 0x00 || *def == 0x00 || opcode == 0x00)
        throw VaultException("Invalid call to API::ParseCommand, one or more arguments are NULL");

    op_default* result = new op_default();
    unsigned int crc32 = Utils::crc32buf(cmd, strlen(cmd));
    CommandCache::iterator it = cache.find(crc32);

    if (it != cache.end())
    {
        *result = it->second.second;
        result->random = rand();
        result_data.first = it->second.first;
        result_data.second = result;
        return result_data;
    }

    try
    {
        result->random = rand();
        char* tokenizer = NULL;
        unsigned int reference = 0x00;
        result->arg2.opcode = opcode;
        result_data.first.push_back(opcode);

        if (opcode == 0x014E || opcode == 0x014F || opcode == 0x0148) // Some commands, such as LoadGame, require the calling thread to be the games main thread, delegate flag allows this
            result->delegate = true;

        if (*def == 'r')
        {
            char* dot = strchr(cmd, '.');

            if (dot != NULL)
                *dot = ' ';

            tokenizer = strtok(cmd, " ");

            if (tokenizer == NULL)
                throw VaultException("API::ParseCommand expected a reference base operand, which could not be found");

            reference = ExtractReference(tokenizer);

            if (reference == 0x00)
                throw VaultException("API::ParseCommand reference base operand is NULL");

            result->arg3.reference = reference;
            result_data.first.push_back(reference);
            def++;
        }

        // Skip the function name

        if (reference == 0x00)
            tokenizer = strtok(cmd, " ");
        else
            tokenizer = strtok(NULL, " ");

        if (tokenizer == NULL)
            throw VaultException("API::ParseCommand could not find the function name");

        unsigned short numargs = 0x00;
        unsigned int refparam = 0x00;
        char* arg1_pos = (char*) &result->arg1.unk1;
        char* arg2_pos = (char*) &result->arg2.param1;

        while (*def != '\0' && numargs < 4) // We don't support more than 4 args yet
        {
            char type = *def;
            def++;

            if (isupper(type))
                *((unsigned int*) (arg1_pos + 4)) = 0x00000001;

            switch (tolower(type))
            {
            case 's': // String
                *((unsigned int*) arg1_pos) = 0x00000000;
                break;
            case 'i': // Integer
                *((unsigned int*) arg1_pos) = 0x00000001;
                break;
            case 'd': // Double
                *((unsigned int*) arg1_pos) = 0x00000002;
                break;
            case 'j': // Object ID item
                *((unsigned int*) arg1_pos) = 0x00000003;
                break;
            case 'o': // Object Reference ID
                *((unsigned int*) arg1_pos) = 0x00000004;
                break;
            case 'v': // Actor Value
                *((unsigned int*) arg1_pos) = 0x00000005;
                break;
            case 'q': // Actor
                *((unsigned int*) arg1_pos) = 0x00000006;
                break;
            case 'a': // Axis
                *((unsigned int*) arg1_pos) = 0x00000008;
                break;
            case 'g': // Animation Group
                *((unsigned int*) arg1_pos) = 0x0000000A;
                break;
            case 'b': // Object ID
                *((unsigned int*) arg1_pos) = 0x00000015;
                break;
            case 'c': // Container
                *((unsigned int*) arg1_pos) = 0x0000001A;
                break;
            case 'k': // Object ID base item
                *((unsigned int*) arg1_pos) = 0x00000032;
                break;
            default:
                throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
            }

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
                    throw VaultException("API::ParseCommand failed parsing command (end of input reached, not all required arguments could be found)");
            }

            /* Types:
                a (Axis, 2 byte) - 0x00000008
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
                r (Reference)

                upper case means optional
            */

            switch (tolower(type))
            {
            case 'i': // Integer
            {
                int integer = atoi(tokenizer);
                *((unsigned char*) arg2_pos) = 0x6E;
                *((int*) (arg2_pos + 1)) = integer;
                result_data.first.push_back(integer);
                arg2_pos += 5;
                break;
            }
            case 'd': // Double
            {
                double floating = atof(tokenizer);
                *((unsigned char*) arg2_pos) = 0x7A;
                *((double*) (arg2_pos + 1)) = floating;
                result_data.first.push_back(floating);
                arg2_pos += 9;
                break;
            }
            case 'b': // Object ID
            case 'j': // Object ID item
            case 'k': // Object ID base item
            case 'o': // Object Reference ID
            case 'q': // Actor
            case 'c': // Container
            {
                if (refparam != 0x00) // We don't support more than one refparam yet
                    throw VaultException("API::ParseCommand does only support one reference argument up until now");

                refparam = ExtractReference(tokenizer);

                if (!refparam)
                    throw VaultException("API::ParseCommand reference argument is NULL");

                *((unsigned char*) arg2_pos) = 0x72;
                *((unsigned short*) (arg2_pos + 1)) = (refparam == reference) ? 0x0001 : 0x0002;
                result_data.first.push_back(refparam);
                arg2_pos += 3;
                break;
            }
            case 'v': // Actor Value
            {
                unsigned char value = RetrieveValue(tokenizer);

                if (value == 0xFF)
                    throw VaultException("API::ParseCommand could not find an Actor Value identifier for input %s", tokenizer);

                *((unsigned short*) arg2_pos) = (unsigned short) value;
                result_data.first.push_back(value);
                arg2_pos += 2;
                break;
            }
            case 'a': // Axis
            {
                unsigned char axis = RetrieveAxis(tokenizer);

                if (axis == 0xFF)
                    throw VaultException("API::ParseCommand could not find an Axis identifier for input %s", tokenizer);

                *((unsigned short*) arg2_pos) = (unsigned short) axis;
                result_data.first.push_back(axis);
                arg2_pos += 2;
                break;
            }
            case 'g': // Animation Group
            {
                unsigned char anim = RetrieveAnim(tokenizer);

                if (anim == 0xFF)
                    throw VaultException("API::ParseCommand could not find an Animation identifier for input %s", tokenizer);

                *((unsigned short*) arg2_pos) = (unsigned short) anim;
                result_data.first.push_back(anim);
                arg2_pos += 2;
                break;
            }
            case 's': // String
            {
                unsigned short length = (unsigned short) strlen(tokenizer);

                if (length > 32)
                    throw VaultException("API::ParseCommand string argument exceeds the limit of 32 characters", tokenizer);

                *((unsigned short*) arg2_pos) = length;
                memcpy(arg2_pos + 2, tokenizer, length);
                result_data.first.push_back(length);
                arg2_pos += 2;
                arg2_pos += length;
                break;
            }
            default:
                throw VaultException("API::ParseCommand could not recognize argument identifier %02X", (unsigned int) tolower(type));
            }

            numargs++;
        }

        result->arg2.numargs = numargs;

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

        result_data.second = result;
    }
    catch (...)
    {
        delete result;
        throw;
    }

    cache.insert(pair<unsigned int, pair<vector<double>, op_default> >(crc32, pair<vector<double>, op_default>(result_data.first, *result_data.second)));

    return result_data;
}

void API::DefineFunction(string name, string def, unsigned short opcode, unsigned short games)
{
    functions.insert(pair<string, pair<string, pair<unsigned short, unsigned short> > >(name, pair<string, pair<unsigned short, unsigned short> >(def, pair<unsigned short, unsigned short>(opcode, games))));
}

void API::DefineValueString(string name, unsigned char value, unsigned short games)
{
    values.insert(pair<string, pair<unsigned char, unsigned short> >(name, pair<unsigned char, unsigned short>(value, games)));
}

void API::DefineAxisString(string name, unsigned char axis, unsigned short games)
{
    API::axis.insert(pair<string, pair<unsigned char, unsigned short> >(name, pair<unsigned char, unsigned short>(axis, games)));
}

void API::DefineAnimString(string name, unsigned char anim, unsigned short games)
{
    anims.insert(pair<string, pair<unsigned char, unsigned short> >(name, pair<unsigned char, unsigned short>(anim, games)));
}

unsigned long API::ExtractReference(char* reference)
{
    unsigned long reference_value;

    reference_value = strtoul(reference, NULL, 16);

    return reference_value;
}

unsigned char API::RetrieveValue(char* value)
{
    ValueList::iterator it;
    pair<ValueList::iterator, ValueList::iterator> it2;
    it2 = values.equal_range(string(value));

    for (it = it2.first; it != it2.second; ++it)
    {
        if ((it->second.second & game) == game)
            return it->second.first;
    }

    return 0xFF;
}

unsigned char API::RetrieveAxis(char* axis)
{
    ValueList::iterator it;
    pair<ValueList::iterator, ValueList::iterator> it2;
    it2 = API::axis.equal_range(string(axis));

    for (it = it2.first; it != it2.second; ++it)
    {
        if ((it->second.second & game) == game)
            return it->second.first;
    }

    return 0xFF;
}

unsigned char API::RetrieveAnim(char* anim)
{
    ValueList::iterator it;
    pair<ValueList::iterator, ValueList::iterator> it2;
    it2 = anims.equal_range(string(anim));

    for (it = it2.first; it != it2.second; ++it)
    {
        if ((it->second.second & game) == game)
            return it->second.first;
    }

    return 0xFF;
}

string API::RetrieveValue_Reverse(unsigned char value)
{
    ValueList::iterator it;

    for (it = values.begin(); it != values.end(); ++it)
    {
        if (it->second.first == value && (it->second.second & game) == game)
            return it->first;
    }

    return string();
}

string API::RetrieveAxis_Reverse(unsigned char axis)
{
    ValueList::iterator it;

    for (it = API::axis.begin(); it != API::axis.end(); ++it)
    {
        if (it->second.first == axis && (it->second.second & game) == game)
            return it->first;
    }

    return string();
}

string API::RetrieveAnim_Reverse(unsigned char anim)
{
    ValueList::iterator it;

    for (it = anims.begin(); it != anims.end(); ++it)
    {
        if (it->second.first == anim && (it->second.second & game) == game)
            return it->first;
    }

    return string();
}

vector<unsigned char> API::RetrieveAllValues()
{
    vector<unsigned char> result;
    ValueList::iterator it;

    for (it = values.begin(); it != values.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->second.first);
    }

    return result;
}

vector<unsigned char> API::RetrieveAllAxis()
{
    vector<unsigned char> result;
    ValueList::iterator it;

    for (it = axis.begin(); it != axis.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->second.first);
    }

    return result;
}

vector<unsigned char> API::RetrieveAllAnims()
{
    vector<unsigned char> result;
    ValueList::iterator it;

    for (it = anims.begin(); it != anims.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->second.first);
    }

    return result;
}

vector<string> API::RetrieveAllValues_Reverse()
{
    vector<string> result;
    ValueList::iterator it;

    for (it = values.begin(); it != values.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->first);
    }

    return result;
}

vector<string> API::RetrieveAllAxis_Reverse()
{
    vector<string> result;
    ValueList::iterator it;

    for (it = axis.begin(); it != axis.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->first);
    }

    return result;
}

vector<string> API::RetrieveAllAnims_Reverse()
{
    vector<string> result;
    ValueList::iterator it;

    for (it = anims.begin(); it != anims.end(); ++it)
    {
        if ((it->second.second & game) == game)
            result.push_back(it->first);
    }

    return result;
}

pair<string, unsigned short> API::RetrieveFunction(string name)
{
    FunctionList::iterator it;
    pair<FunctionList::iterator, FunctionList::iterator> it2;
    it2 = functions.equal_range(name);

    for (it = it2.first; it != it2.second; ++it)
    {
        if ((it->second.second.second & game) == game)
            return pair<string, unsigned short>(it->second.first, it->second.second.first);
    }

    pair<string, unsigned short> empty = pair<string, unsigned short>("", 0x00);

    return empty;
}

bool API::AnnounceFunction(string name)
{
    pair<string, unsigned short> func = RetrieveFunction(name);

    if (!func.first.empty())
        return true;

#ifdef VAULTMP_DEBUG
    if (debug != NULL)
        debug->PrintFormat("API function %s not found or not supported by the game", true, name.c_str());
#endif

    return false;
}

char* API::BuildCommandStream(char* command, vector<double> info, unsigned int size, signed int key)
{
    char* data = new char[PIPE_LENGTH];
    ZeroMemory(data, sizeof(data));
    data[0] = PIPE_OP_COMMAND;
    *((signed int*) ((unsigned) data + 5)) = key;

    if (size + 9 > PIPE_LENGTH)
    {
        delete[] data;
        throw VaultException("Error in API class; command size (%d bytes) exceeds the pipe length of %d bytes", size + 9, PIPE_LENGTH);
    }

    memcpy(data + 9, command, size);

    unsigned int crc = Utils::crc32buf(data + 5, PIPE_LENGTH - 5);
    *((unsigned int*) ((unsigned) data + 1)) = crc;
    queue.push_front(pair<unsigned int, vector<double> >(crc, info));

    return data;
}

CommandParsed API::Translate(multimap<string, string>& cmd, signed int key)
{
    CommandParsed stream;
    multimap<string, string>::iterator it;

    for (it = cmd.begin(); it != cmd.end(); ++it)
    {
        string name = it->first;
        multimap<string, pair<string, unsigned short> >::iterator it2;
        pair<string, unsigned short> func = RetrieveFunction(name);

        if (func.first.empty())
        {
#ifdef VAULTMP_DEBUG
            if (debug != NULL)
                debug->PrintFormat("API was not able to find function %s", true, name.c_str());
#endif
            continue;
        }

        char def[func.first.length()];
        char content[it->second.length()];
        strcpy(def, func.first.c_str());
        strcpy(content, it->second.c_str());

        pair<vector<double>, op_default*> command = ParseCommand(content, def, func.second);

        char* data = BuildCommandStream((char*) command.second, command.first, sizeof(op_default), key);

        stream.push_back(data);

        delete command.second;
    }

    return stream;
}

CommandResult API::Translate(char* stream)
{
    CommandResult result;
    result.first.first = 0;
    result.first.second = vector<double>();

    if (stream[0] != PIPE_OP_RETURN)
        return result;

    unsigned int crc = *((unsigned int*) ((unsigned) stream + 1));

    while (!queue.empty() && queue.back().first != crc)
    {
#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("API did not retrieve the result of command with CRC32 %08X", true, queue.back().first);
#endif
        queue.pop_back();
    }

    if (queue.empty())
    {
#ifdef VAULTMP_DEBUG
        if (debug != NULL)
            debug->PrintFormat("API did not retrieve the result of command with CRC32 %08X", true, crc);
#endif
        return result;
    }

    result.first.first = *((signed int*) ((unsigned) stream + 5));
    result.first.second = queue.back().second;
    result.second = *((double*) ((unsigned) stream + 9));

    queue.pop_back();

    return result;
}
