#ifndef API_H
#define API_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <ctime>
#include <climits>
#include <string>
#include <list>
#include <map>
#include <queue>
#include <vector>

#include "vaultmp.h"
#include "Utils.h"
#include "VaultException.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;
using namespace Data;

typedef vector<char*> CommandParsed;
typedef pair<pair<pair<signed int, vector<double> >, double>, bool> CommandResult;
typedef map<string, pair<string, unsigned short> > FunctionList;
typedef map<string, unsigned char> ValueList;
typedef deque<pair<pair<unsigned int, vector<double> >, signed int> > CommandQueue;

namespace Values {

    enum AnimationGroups
    {
        AnimGroup_Idle                  = 0x00,
        AnimGroup_DynamicIdle           = 0x01,
        AnimGroup_SpecialIdle           = 0x02,
        AnimGroup_Forward               = 0x03,
        AnimGroup_Backward              = 0x04,
        AnimGroup_Left                  = 0x05,
        AnimGroup_Right                 = 0x06,
        AnimGroup_FastForward           = 0x07,
        AnimGroup_FastBackward          = 0x08,
        AnimGroup_FastLeft              = 0x09,
        AnimGroup_FastRight             = 0x0A,
        AnimGroup_DodgeForward          = 0x0B,
        AnimGroup_DodgeBack             = 0x0C,
        AnimGroup_DodgeLeft             = 0x0D,
        AnimGroup_DodgeRight            = 0x0E,
        AnimGroup_TurnLeft              = 0x0F,
        AnimGroup_TurnRight             = 0x10,
    };

    enum XYZ
    {
        Axis_X                          = 0x58,
        Axis_Y                          = 0x59,
        Axis_Z                          = 0x5A,
    };

    enum Functions
    {
        Func_GetPos                     = 0x1006,
        Func_SetPos                     = 0x1007,
        Func_GetAngle                   = 0x1008,
        Func_SetAngle                   = 0x1009,
        Func_GetBaseActorValue          = 0x1115,
        Func_SetActorValue              = 0x100F,
        Func_GetActorValue              = 0x100E,
        Func_ForceActorValue            = 0x110E,
        Func_GetDead                    = 0x102E,
        Func_MoveTo                     = 0x109E,
        Func_PlaceAtMe                  = 0x1025,
        Func_SetRestrained              = 0x10F3,
        Func_PlayGroup                  = 0x1013,
        Func_SetAlert                   = 0x105A,
        Func_RemoveAllItems             = 0x10AD,
        Func_Enable                     = 0x1021,
        Func_Disable                    = 0x1022,
        Func_EquipItem                  = 0x10EE,
        Func_UnequipItem                = 0x10EF,
        Func_AddItem                    = 0x1002,
        Func_RemoveItem                 = 0x1052,
        Func_Kill                       = 0x108B,
        Func_GetCombatTarget            = 0x10E8,

        Func_GetActorState              = 0x0001 | VAULTFUNCTION,
    };

    namespace Fallout {

        enum Functions
        {
            Func_IsMoving                   = 0x1019,
            Func_MarkForDelete              = 0x11BB,
            Func_IsAnimPlaying              = 0x1128,
        };

        enum ActorVals
        {
            ActorVal_Aggression			    = 0x00,
            ActorVal_Confidence			    = 0x01,
            ActorVal_Energy				    = 0x02,
            ActorVal_Responsibility		    = 0x03,
            ActorVal_Mood				    = 0x04,
            ActorVal_Strength			    = 0x05,
            ActorVal_Perception			    = 0x06,
            ActorVal_Endurance			    = 0x07,
            ActorVal_Charisma			    = 0x08,
            ActorVal_Intelligence		    = 0x09,
            ActorVal_Agility			    = 0x0A,
            ActorVal_Luck				    = 0x0B,
            ActorVal_ActionPoints		    = 0x0C,
            ActorVal_CarryWeight		    = 0x0D,
            ActorVal_CritChance			    = 0x0E,
            ActorVal_HealRate			    = 0x0F,
            ActorVal_Health				    = 0x10,
            ActorVal_MeleeDamage		    = 0x11,
            ActorVal_DamageResistance	    = 0x12,
            ActorVal_PoisonResistance	    = 0x13,
            ActorVal_RadResistance		    = 0x14,
            ActorVal_SpeedMultiplier	    = 0x15,
            ActorVal_Fatigue			    = 0x16,
            ActorVal_Karma				    = 0x17,
            ActorVal_XP					    = 0x18,
            ActorVal_Head				    = 0x19,
            ActorVal_Torso				    = 0x1A,
            ActorVal_LeftArm			    = 0x1B,
            ActorVal_RightArm			    = 0x1C,
            ActorVal_LeftLeg			    = 0x1D,
            ActorVal_RightLeg			    = 0x1E,
            ActorVal_Brain				    = 0x1F,
            ActorVal_Barter				    = 0x20,
            ActorVal_BigGuns			    = 0x21,
            ActorVal_EnergyWeapons		    = 0x22,
            ActorVal_Explosives			    = 0x23,
            ActorVal_Lockpick			    = 0x24,
            ActorVal_Medicine			    = 0x25,
            ActorVal_MeleeWeapons		    = 0x26,
            ActorVal_Repair				    = 0x27,
            ActorVal_Science			    = 0x28,
            ActorVal_SmallGuns			    = 0x29,
            ActorVal_Sneak				    = 0x2A,
            ActorVal_Speech				    = 0x2B,
            ActorVal_Throwing			    = 0x2C,
            ActorVal_Unarmed			    = 0x2D,
            ActorVal_InventoryWeight	    = 0x2E,
            ActorVal_Paralysis			    = 0x2F,
            ActorVal_Invisibility		    = 0x30,
            ActorVal_Chameleon			    = 0x31,
            ActorVal_NightEye			    = 0x32,
            ActorVal_DetectLifeRange	    = 0x33,
            ActorVal_FireResistance		    = 0x34,
            ActorVal_WaterBreathing		    = 0x35,
            ActorVal_RadLevel			    = 0x36,
            ActorVal_BloodyMess			    = 0x37,
            ActorVal_UnarmedDamage		    = 0x38,
            ActorVal_Assistance			    = 0x39,

            ActorVal_EnergyResistance	    = 0x3C,
            ActorVal_EMPResistance		    = 0x3D,
            ActorVal_Var1Medical		    = 0x3E,
            ActorVal_Variable02             = 0x3F,
            ActorVal_Variable03             = 0x40,
            ActorVal_Variable04             = 0x41,
            ActorVal_Variable05             = 0x42,
            ActorVal_Variable06             = 0x43,
            ActorVal_Variable07             = 0x44,
            ActorVal_Variable08             = 0x45,
            ActorVal_Variable09             = 0x46,
            ActorVal_Variable10             = 0x47,
            ActorVal_IgnoreCrippledLimbs    = 0x48,
        };
    };

    namespace Fallout3 {

        enum Functions
        {
            Func_Load                       = 0x014F,
            Func_SetName                    = 0x1485,
            Func_GetParentCell              = 0x1495,
            Func_GetFirstRef                = 0x14AF,
            Func_GetNextRef                 = 0x14B0,
        };

        enum AnimationGroups
        {
            AnimGroup_JumpStart             = 0xA8,
            AnimGroup_JumpLoop              = 0xA9,
            AnimGroup_JumpLand              = 0xAA,

            AnimGroup_JumpLoopForward       = 0xAE,
            AnimGroup_JumpLoopBackward      = 0xAF,
            AnimGroup_JumpLoopLeft          = 0xB0,
            AnimGroup_JumpLoopRight         = 0xB1,

            AnimGroup_JumpLandForward       = 0xB3,
            AnimGroup_JumpLandBackward      = 0xB4,
            AnimGroup_JumpLandLeft          = 0xB5,
            AnimGroup_JumpLandRight         = 0xB6,
        };
    };

    namespace FalloutNV {

        enum Functions
        {
            Func_Load                       = 0x014E,
            Func_SetName                    = 0x144C,
            Func_GetParentCell              = 0x146D,
            Func_GetFirstRef                = 0x1471,
            Func_GetNextRef                 = 0x1472,
        };

        enum AnimationGroups
        {
            AnimGroup_JumpStart             = 0xE3,
            AnimGroup_JumpLoop              = 0xE4,
            AnimGroup_JumpLand              = 0xE5,

            AnimGroup_JumpLoopForward       = 0xEC,
            AnimGroup_JumpLoopBackward      = 0xED,
            AnimGroup_JumpLoopLeft          = 0xEE,
            AnimGroup_JumpLoopRight         = 0xEF,

            AnimGroup_JumpLandForward       = 0xF1,
            AnimGroup_JumpLandBackward      = 0xF2,
            AnimGroup_JumpLandLeft          = 0xF3,
            AnimGroup_JumpLandRight         = 0xF4,
        };
    };

    namespace Oblivion {

        enum Functions
        {
            Func_Load                       = 0x0148,
            Func_SetName                    = 0x14A3,
            Func_GetParentCell              = 0x1412,
            Func_IsAnimGroupPlaying         = 0x16C1, // equivalent to IsAnimPlaying in Fallout games
            Func_GetFirstRef                = 0x1608,
            Func_GetNextRef                 = 0x1609,
        };

        enum ActorVals
        {
            ActorVal_Strength               = 0x00,
            ActorVal_Intelligence           = 0x01,
            ActorVal_Willpower              = 0x02,
            ActorVal_Agility                = 0x03,
            ActorVal_Speed                  = 0x04,
            ActorVal_Endurance              = 0x05,
            ActorVal_Personality            = 0x06,
            ActorVal_Luck                   = 0x07,
            ActorVal_Health                 = 0x08,
            ActorVal_Magicka                = 0x09,
            ActorVal_Fatigue                = 0x0A,
            ActorVal_Encumbrance            = 0x0B,
            ActorVal_Armorer                = 0x0C,
            ActorVal_Athletics              = 0x0D,
            ActorVal_Blade                  = 0x0E,
            ActorVal_Block                  = 0x0F,
            ActorVal_Blunt                  = 0x10,
            ActorVal_HandToHand             = 0x11,
            ActorVal_HeavyArmor             = 0x12,
            ActorVal_Alchemy                = 0x13,
            ActorVal_Alteration             = 0x14,
            ActorVal_Conjuration            = 0x15,
            ActorVal_Destruction            = 0x16,
            ActorVal_Illusion               = 0x17,
            ActorVal_Mysticism              = 0x18,
            ActorVal_Restoration            = 0x19,
            ActorVal_Acrobatics             = 0x1A,
            ActorVal_LightArmor             = 0x1B,
            ActorVal_Marksman               = 0x1C,
            ActorVal_Mercantile             = 0x1D,
            ActorVal_Security               = 0x1E,
            ActorVal_Sneak                  = 0x1F,
            ActorVal_Speechcraft            = 0x20,
            ActorVal_Aggression             = 0x21,
            ActorVal_Confidence             = 0x22,
            ActorVal_Energy                 = 0x23,
            ActorVal_Responsibility         = 0x24,
            ActorVal_Bounty                 = 0x25,
            ActorVal_Fame                   = 0x26,
            ActorVal_Infamy                 = 0x27,
            ActorVal_MagickaMultiplier      = 0x28,
            ActorVal_NightEyeBonus          = 0x29,
            ActorVal_AttackBonus            = 0x2A,
            ActorVal_DefendBonus            = 0x2B,
            ActorVal_CastingPenalty         = 0x2C,
            ActorVal_Blindness              = 0x2D,
            ActorVal_Chameleon              = 0x2E,
            ActorVal_Invisibility           = 0x2F,
            ActorVal_Paralysis              = 0x30,
            ActorVal_Silence                = 0x31,
            ActorVal_Confusion              = 0x32,
            ActorVal_DetectItemRange        = 0x33,
            ActorVal_SpellAbsorbChance      = 0x34,
            ActorVal_SpellReflectChance     = 0x35,
            ActorVal_SwimSpeedMultiplier    = 0x36,
            ActorVal_WaterBreathing         = 0x37,
            ActorVal_WaterWalking           = 0x38,
            ActorVal_StuntedMagicka         = 0x39,
            ActorVal_DetectLifeRange        = 0x3A,
            ActorVal_ReflectDamage          = 0x3B,
            ActorVal_Telekinesis            = 0x3C,
            ActorVal_ResistFire             = 0x3D,
            ActorVal_ResistFrost            = 0x3E,
            ActorVal_ResistDisease          = 0x3F,
            ActorVal_ResistMagic            = 0x40,
            ActorVal_ResistNormalWeapons    = 0x41,
            ActorVal_ResistParalysis        = 0x42,
            ActorVal_ResistPoison           = 0x43,
            ActorVal_ResistShock            = 0x44,
            ActorVal_Vampirism              = 0x45,
            ActorVal_Darkness               = 0x46,
            ActorVal_ResistWaterDamage      = 0x47,
        };

        enum AnimationGroups
        {
            AnimGroup_Equip                 = 0x11,
            AnimGroup_Unequip               = 0x12,
            AnimGroup_AttackBow             = 0x13,
            AnimGroup_AttackLeft            = 0x14,
            AnimGroup_AttackRight           = 0x15,
            AnimGroup_AttackPower           = 0x16,
            AnimGroup_AttackForwardPower    = 0x17,
            AnimGroup_AttackBackPower       = 0x18,
            AnimGroup_AttackLeftPower       = 0x19,
            AnimGroup_AttackRightPower      = 0x1A,
            AnimGroup_BlockIdle             = 0x1B,
            AnimGroup_BlockHit              = 0x1C,
            AnimGroup_BlockAttack           = 0x1D,
            AnimGroup_Recoil                = 0x1E,
            AnimGroup_Stagger               = 0x1F,
            AnimGroup_Death                 = 0x20,
            AnimGroup_TorchIdle             = 0x21,
            AnimGroup_CastSelf              = 0x22,
            AnimGroup_CastTouch             = 0x23,
            AnimGroup_CastTarget            = 0x24,
            AnimGroup_CastSelfAlt           = 0x25,
            AnimGroup_CastTouchAlt          = 0x26,
            AnimGroup_CastTargetAlt         = 0x27,
            AnimGroup_JumpStart             = 0x28,
            AnimGroup_JumpLoop              = 0x29,
            AnimGroup_JumpLand              = 0x2A,
        };
    };
};

class API
{

private:
    struct op_Arg1;
    struct op_Arg2;
    struct op_Arg3;
    struct op_Arg4;
    struct op_Arg5;
    struct op_Arg6;
    struct op_Arg7;
    struct op_Arg8;
    struct op_default;
    typedef map<unsigned int, pair<vector<double>, API::op_default> > CommandCache;

    static FunctionList functions;
    static ValueList values;
    static ValueList axis;
    static ValueList anims;
    static CommandQueue queue;
    static CommandCache cache;
    static unsigned char game;

    static void DefineFunction(string name, string def, unsigned short opcode, unsigned char games);
    static void DefineValueString(string name, unsigned char value, unsigned char games);
    static void DefineAxisString(string name, unsigned char axis, unsigned char games);
    static void DefineAnimString(string name, unsigned char anim, unsigned char games);

    static unsigned long ExtractReference(char* reference);
    static pair<string, unsigned short> RetrieveFunction(string name);
    static char* BuildCommandStream(char* stream, vector<double> info, unsigned int size, signed int key = 0);

    static pair<vector<double>, op_default*> ParseCommand(char* cmd, char* def, unsigned short opcode);

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

protected:

    static bool AnnounceFunction(string name);
    static CommandParsed Translate(multimap<string, string>& cmd, int key = 0);
    static vector<CommandResult> Translate(char* stream);

    API();

public:

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    static void Initialize(unsigned char game);
    static void Terminate();

    static unsigned char RetrieveValue(char* value);
    static unsigned char RetrieveAxis(char* axis);
    static unsigned char RetrieveAnim(char* anim);
    static string RetrieveValue_Reverse(unsigned char value);
    static string RetrieveAxis_Reverse(unsigned char axis);
    static string RetrieveAnim_Reverse(unsigned char anim);
    static vector<unsigned char> RetrieveAllValues();
    static vector<unsigned char> RetrieveAllAxis();
    static vector<unsigned char> RetrieveAllAnims();
    static vector<string> RetrieveAllValues_Reverse();
    static vector<string> RetrieveAllAxis_Reverse();
    static vector<string> RetrieveAllAnims_Reverse();

    static unsigned char GetGameCode();

};

#endif
