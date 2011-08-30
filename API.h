#ifndef API_H
#define API_H

#include <windows.h>
#include <time.h>
#include <limits.h>
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
typedef pair<pair<signed int, vector<double> >, double> CommandResult;
typedef multimap<string, pair<string, pair<unsigned short, unsigned short> > > FunctionList;
typedef multimap<string, pair<unsigned char, unsigned short> > ValueList;
typedef deque<pair<unsigned int, vector<double> > > CommandQueue;

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
    };

    namespace Fallout {

        enum Functions
        {
            Func_IsMoving                   = 0x1019,
            Func_MarkForDelete              = 0x11BB,
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
        };

    };

    namespace FalloutNV {

        enum Functions
        {
            Func_Load                       = 0x014E,
            Func_SetName                    = 0x144C,
            Func_GetParentCell              = 0x146D,
        };

    };

    namespace Oblivion {

        enum Functions
        {
            Func_Load                       = 0x0148,
            Func_SetName                    = 0x14A3,
            Func_GetParentCell              = 0x1412,
        };

/*
  0:    Strength
  1:    Intelligence
  2:    Willpower
  3:    Agility
  4:    Speed
  5:    Endurance
  6:    Personality
  7:    Luck
  8:    Health
  9:    Magicka
 10:    Fatigue
 11:    Encumbrance
 12:    Armorer
 13:    Athletics
 14:    Blade
 15:    Block
 16:    Blunt
 17:    HandToHand
 18:    HeavyArmor
 19:    Alchemy
 20:    Alteration
 21:    Conjuration
 22:    Destruction
 23:    Illusion
 24:    Mysticism
 25:    Restoration
 26:    Acrobatics
 27:    LightArmor
 28:    Marksman
 29:    Mercantile
 30:    Security
 31:    Sneak
 32:    Speechcraft
 33:    Aggression
 34:    Confidence
 35:    Energy
 36:    Responsibility
 37:    Bounty
 38:    Fame
 39:    Infamy
 40:    MagickaMultiplier
 41:    NightEyeBonus
 42:    AttackBonus
 43:    DefendBonus
 44:    CastingPenalty
 45:    Blindness
 46:    Chameleon
 47:    Invisibility
 48:    Paralysis
 49:    Silence
 50:    Confusion
 51:    DetectItemRange
 52:    SpellAbsorbChance
 53:    SpellReflectChance
 54:    SwimSpeedMultiplier
 55:    WaterBreathing
 56:    WaterWalking
 57:    StuntedMagicka
 58:    DetectLifeRange
 59:    ReflectDamage
 60:    Telekinesis
 61:    ResistFire
 62:    ResistFrost
 63:    ResistDisease
 64:    ResistMagic
 65:    ResistNormalWeapons
 68:    ResistParalysis
 67:    ResistPoison
 68:    ResistShock
 69:    Vampirism
 70:    Darkness
 71:    ResistWaterDamage
*/

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
    static int game;

    static void DefineFunction(string name, string def, unsigned short opcode, unsigned short games);
    static void DefineValueString(string name, unsigned char value, unsigned short games);
    static void DefineAxisString(string name, unsigned char axis, unsigned short games);
    static void DefineAnimString(string name, unsigned char anim, unsigned short games);

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
    static CommandResult Translate(char* stream);

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

    API();

public:

    static void Initialize(int game);
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

};

#endif
