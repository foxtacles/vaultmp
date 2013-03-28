#ifndef API_H
#define API_H

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <ctime>
#include <climits>
#include <string>
#include <set>
#include <map>
#include <unordered_map>
#include <queue>
#include <vector>

#include "vaultmp.h"
#include "Utils.h"
#include "VaultException.h"
#include "Data.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

/*
 * \brief This namespace contains key data of the game
 */

namespace Values
{
	/**
	 * \brief Animation values
	 */

	enum AnimationGroups
	{
		AnimGroup_Idle = 0x00,
		AnimGroup_DynamicIdle = 0x01,
		AnimGroup_SpecialIdle = 0x02,
		AnimGroup_Forward = 0x03,
		AnimGroup_Backward = 0x04,
		AnimGroup_Left = 0x05,
		AnimGroup_Right = 0x06,
		AnimGroup_FastForward = 0x07,
		AnimGroup_FastBackward = 0x08,
		AnimGroup_FastLeft = 0x09,
		AnimGroup_FastRight = 0x0A,
		AnimGroup_DodgeForward = 0x0B,
		AnimGroup_DodgeBack = 0x0C,
		AnimGroup_DodgeLeft = 0x0D,
		AnimGroup_DodgeRight = 0x0E,
		AnimGroup_TurnLeft = 0x0F,
		AnimGroup_TurnRight = 0x10,
		AnimGroup_Aim = 0x11,
		AnimGroup_AimUp = 0x12,
		AnimGroup_AimDown = 0x13,
		AnimGroup_AimIS = 0x14,
		AnimGroup_AimISUp = 0x15,
		AnimGroup_AimISDown = 0x16,
		AnimGroup_Holster = 0x17,
		AnimGroup_Equip = 0x18,
		AnimGroup_Unequip = 0x19,

		AnimGroup_AttackLeft = 0x1A,
		AnimGroup_AttackLeftUp = 0x1B,
		AnimGroup_AttackLeftDown = 0x1C,
		AnimGroup_AttackLeftIS = 0x1D,
		AnimGroup_AttackLeftISUp = 0x1E,
		AnimGroup_AttackLeftISDown = 0x1E,
		AnimGroup_AttackRight = 0x20,
		AnimGroup_AttackRightUp = 0x21,
		AnimGroup_AttackRightDown = 0x22,
		AnimGroup_AttackRightIS = 0x23,
		AnimGroup_AttackRightISUp = 0x24,
		AnimGroup_AttackRightISDown = 0x25,

		AnimGroup_Attack3 = 0x26,
		AnimGroup_Attack3Up = 0x27,
		AnimGroup_Attack3Down = 0x28,
		AnimGroup_Attack3IS = 0x29,
		AnimGroup_Attack3ISUp = 0x2A,
		AnimGroup_Attack3ISDown = 0x2B,
		AnimGroup_Attack4 = 0x2C,
		AnimGroup_Attack4Up = 0x2D,
		AnimGroup_Attack4Down = 0x2E,
		AnimGroup_Attack4IS = 0x2F,
		AnimGroup_Attack4ISUp = 0x30,
		AnimGroup_Attack4ISDown = 0x31,
		AnimGroup_Attack5 = 0x32,
		AnimGroup_Attack5Up = 0x33,
		AnimGroup_Attack5Down = 0x34,
		AnimGroup_Attack5IS = 0x35,
		AnimGroup_Attack5ISUp = 0x36,
		AnimGroup_Attack5ISDown = 0x37,
		AnimGroup_Attack6 = 0x38,
		AnimGroup_Attack6Up = 0x39,
		AnimGroup_Attack6Down = 0x3A,
		AnimGroup_Attack6IS = 0x3B,
		AnimGroup_Attack6ISUp = 0x3C,
		AnimGroup_Attack6ISDown = 0x3D,
		AnimGroup_Attack7 = 0x3E,
		AnimGroup_Attack7Up = 0x3F,
		AnimGroup_Attack7Down = 0x40,
		AnimGroup_Attack7IS = 0x41,
		AnimGroup_Attack7ISUp = 0x42,
		AnimGroup_Attack7ISDown = 0x43,
		AnimGroup_Attack8 = 0x44,
		AnimGroup_Attack8Up = 0x45,
		AnimGroup_Attack8Down = 0x46,
		AnimGroup_Attack8IS = 0x47,
		AnimGroup_Attack8ISUp = 0x48,
		AnimGroup_Attack8ISDown = 0x49,

		AnimGroup_AttackLoop = 0x4A,
		AnimGroup_AttackLoopUp = 0x4B,
		AnimGroup_AttackLoopDown = 0x4C,
		AnimGroup_AttackLoopIS = 0x4D,
		AnimGroup_AttackLoopISUp = 0x4E,
		AnimGroup_AttackLoopISDown = 0x4F,
		AnimGroup_AttackSpin = 0x50,
		AnimGroup_AttackSpinUp = 0x51,
		AnimGroup_AttackSpinDown = 0x52,
		AnimGroup_AttackSpinIS = 0x53,
		AnimGroup_AttackSpinISUp = 0x54,
		AnimGroup_AttackSpinISDown = 0x55,
		AnimGroup_AttackSpin2 = 0x56,
		AnimGroup_AttackSpin2Up = 0x57,
		AnimGroup_AttackSpin2Down = 0x58,
		AnimGroup_AttackSpin2IS = 0x59,
		AnimGroup_AttackSpin2ISUp = 0x5A,
		AnimGroup_AttackSpin2ISDown = 0x5B,

		AnimGroup_AttackPower = 0x5C,
		AnimGroup_AttackForwardPower = 0x5D,
		AnimGroup_AttackBackPower = 0x5E,
		AnimGroup_AttackLeftPower = 0x5F,
		AnimGroup_AttackRightPower = 0x60,

		AnimGroup_JumpStart = 0xA8,
		AnimGroup_JumpLoop = 0xA9,
		AnimGroup_JumpLand = 0xAA,

		AnimGroup_JumpLoopForward = 0xAE,
		AnimGroup_JumpLoopBackward = 0xAF,
		AnimGroup_JumpLoopLeft = 0xB0,
		AnimGroup_JumpLoopRight = 0xB1,

		AnimGroup_JumpLandForward = 0xB3,
		AnimGroup_JumpLandBackward = 0xB4,
		AnimGroup_JumpLandLeft = 0xB5,
		AnimGroup_JumpLandRight = 0xB6,

		AnimGroup_BlockIdle = 0x8B,
		AnimGroup_BlockHit = 0x8C,

		AnimGroup_ReloadA = 0x8E,
		AnimGroup_ReloadB = 0x8F,
		AnimGroup_ReloadC = 0x90,
		AnimGroup_ReloadD = 0x91,
		AnimGroup_ReloadE = 0x92,
		AnimGroup_ReloadF = 0x93,
		AnimGroup_ReloadG = 0x94,
		AnimGroup_ReloadH = 0x95,
		AnimGroup_ReloadI = 0x96,
		AnimGroup_ReloadJ = 0x97,
		AnimGroup_ReloadK = 0x98,
	};

	/**
	 * \brief Axis values
	 */

	enum XYZ
	{
		Axis_X = 0x58,
		Axis_Y = 0x59,
		Axis_Z = 0x5A,
	};

	/**
	 * \brief Limb values
	 */

	enum Limbs
	{
		Limb_None = -1,
		Limb_Torso = 0,
		Limb_Head1 = 1,
		Limb_Head2 = 2,
		Limb_LeftArm1 = 3,
		Limb_LeftArm2 = 4,
		Limb_RightArm1 = 5,
		Limb_RightArm2 = 6,
		Limb_LeftLeg1 = 7,
		Limb_LeftLeg2 = 8,
		Limb_LeftLeg3 = 9,
		Limb_RightLeg1 = 10,
		Limb_RightLeg2 = 11,
		Limb_RightLeg3 = 12,
		Limb_Brain = 13,
		Limb_Weapon = 14,
	};

	/**
	 * \brief Cause of death values
	 */

	enum Death
	{
		Death_None = -1,
		Death_Explosion = 0,
		Death_Gun = 2,
		Death_BluntWeapon = 3,
		Death_HandToHand = 4,
		Death_ObjectImpact = 5,
		Death_Poison = 6,
		Death_Radiation = 7,
	};

	/**
	 * \brief Function opcodes
	 */

	enum Functions
	{
		Func_CenterOnCell = 0x0123,
		Func_CenterOnExterior = 0x0127,
		Func_SetINISetting = 0x0125,

		Func_GetPos = 0x1006,
		Func_SetPos = 0x1007,
		Func_GetAngle = 0x1008,
		Func_SetAngle = 0x1009,
		Func_GetBaseActorValue = 0x1115,
		Func_SetActorValue = 0x100F,
		Func_GetActorValue = 0x100E,
		Func_ForceActorValue = 0x110E,
		Func_GetDead = 0x102E,
		Func_MoveTo = 0x109E,
		Func_PlaceAtMe = 0x1025,
		Func_PlaceAtMeHealthPercent = 0x11BD,
		Func_SetRestrained = 0x10F3,
		Func_PlayGroup = 0x1013,
		Func_SetAlert = 0x105A,
		Func_RemoveAllItems = 0x10AD,
		Func_Enable = 0x1021,
		Func_Disable = 0x1022,
		Func_EquipItem = 0x10EE,
		Func_UnequipItem = 0x10EF,
		Func_AddItem = 0x1002,
		Func_AddItemHealthPercent = 0x11BC,
		Func_RemoveItem = 0x1052,
		Func_Kill = 0x108B,
		Func_GetCombatTarget = 0x10E8,
		Func_SetForceSneak = 0x10D3,
		Func_IsMoving = 0x1019,
		Func_MarkForDelete = 0x11BB,
		Func_IsAnimPlaying = 0x1128,
		Func_FireWeapon = 0x11E2,
		Func_GetCauseofDeath = 0x118D,
		Func_IsLimbGone = 0x118E,
		Func_EnablePlayerControls = 0x1060,
		Func_DisablePlayerControls = 0x1061,
		Func_DamageActorValue = 0x1181,
		Func_RestoreActorValue = 0x1182,
		Func_PlayIdle = 0x1190,
		Func_AgeRace = 0x11E4,
		Func_MatchRace = 0x11E5,
		Func_SexChange = 0x11E7,
		Func_ForceWeather = 0x112D,
		Func_Lock = 0x1072,
		Func_Unlock = 0x1073,
		Func_SetOwnership = 0x1117,
		Func_GetLocked = 0x1005,

		Func_Load = 0x014F,
		Func_CenterOnWorld = 0x0143,

		Func_SetName = 0x1485,
		Func_GetParentCell = 0x1495,
		Func_GetFirstRef = 0x14AF,
		Func_GetNextRef = 0x14B0,
		Func_GetControl = 0x144E,
		Func_DisableControl = 0x145D,
		Func_EnableControl = 0x145E,
		Func_DisableKey = 0x143E,
		Func_EnableKey = 0x143F,
		Func_GetRefCount = 0x14C3,
		Func_SetRefCount = 0x14C4,
		Func_GetBaseObject = 0x1416,
		Func_SetCurrentHealth = 0x14BF,

		Func_GetActorState = 0x0001 | VAULTFUNCTION,
		Func_Chat = 0x0002 | VAULTFUNCTION,
		Func_ScanContainer = 0x0003 | VAULTFUNCTION,
		Func_UIMessage = 0x0004 | VAULTFUNCTION,
		Func_RemoveAllItemsEx = 0x0005 | VAULTFUNCTION,
		Func_ForceRespawn = 0x0006 | VAULTFUNCTION,
		Func_SetGlobalValue = 0x0007 | VAULTFUNCTION,
		Func_Chatbox = 0x0008 | VAULTFUNCTION,
	};

	enum ActorVals
	{
		ActorVal_Aggression = 0x00,
		ActorVal_Confidence = 0x01,
		ActorVal_Energy = 0x02,
		ActorVal_Responsibility = 0x03,
		ActorVal_Mood = 0x04,
		ActorVal_Strength = 0x05,
		ActorVal_Perception = 0x06,
		ActorVal_Endurance = 0x07,
		ActorVal_Charisma = 0x08,
		ActorVal_Intelligence = 0x09,
		ActorVal_Agility = 0x0A,
		ActorVal_Luck = 0x0B,
		ActorVal_ActionPoints = 0x0C,
		ActorVal_CarryWeight = 0x0D,
		ActorVal_CritChance = 0x0E,
		ActorVal_HealRate = 0x0F,
		ActorVal_Health = 0x10,
		ActorVal_MeleeDamage = 0x11,
		ActorVal_DamageResistance = 0x12,
		ActorVal_PoisonResistance = 0x13,
		ActorVal_RadResistance = 0x14,
		ActorVal_SpeedMultiplier = 0x15,
		ActorVal_Fatigue = 0x16,
		ActorVal_Karma = 0x17,
		ActorVal_XP = 0x18,
		ActorVal_Head = 0x19,
		ActorVal_Torso = 0x1A,
		ActorVal_LeftArm = 0x1B,
		ActorVal_RightArm = 0x1C,
		ActorVal_LeftLeg = 0x1D,
		ActorVal_RightLeg = 0x1E,
		ActorVal_Brain = 0x1F,
		ActorVal_Barter = 0x20,
		ActorVal_BigGuns = 0x21,
		ActorVal_EnergyWeapons = 0x22,
		ActorVal_Explosives = 0x23,
		ActorVal_Lockpick = 0x24,
		ActorVal_Medicine = 0x25,
		ActorVal_MeleeWeapons = 0x26,
		ActorVal_Repair = 0x27,
		ActorVal_Science = 0x28,
		ActorVal_SmallGuns = 0x29,
		ActorVal_Sneak = 0x2A,
		ActorVal_Speech = 0x2B,
		ActorVal_Throwing = 0x2C,
		ActorVal_Unarmed = 0x2D,
		ActorVal_InventoryWeight = 0x2E,
		ActorVal_Paralysis = 0x2F,
		ActorVal_Invisibility = 0x30,
		ActorVal_Chameleon = 0x31,
		ActorVal_NightEye = 0x32,
		ActorVal_DetectLifeRange = 0x33,
		ActorVal_FireResistance = 0x34,
		ActorVal_WaterBreathing = 0x35,
		ActorVal_RadLevel = 0x36,
		ActorVal_BloodyMess = 0x37,
		ActorVal_UnarmedDamage = 0x38,
		ActorVal_Assistance = 0x39,

		ActorVal_EnergyResistance = 0x3C,
		ActorVal_EMPResistance = 0x3D,
		ActorVal_Var1Medical = 0x3E,
		ActorVal_Variable02 = 0x3F,
		ActorVal_Variable03 = 0x40,
		ActorVal_Variable04 = 0x41,
		ActorVal_Variable05 = 0x42,
		ActorVal_Variable06 = 0x43,
		ActorVal_Variable07 = 0x44,
		ActorVal_Variable08 = 0x45,
		ActorVal_Variable09 = 0x46,
		ActorVal_Variable10 = 0x47,
		ActorVal_IgnoreCrippledLimbs = 0x48,
	};

	enum ControlCodes
	{
		ControlCode_Forward = 0,
		ControlCode_Backward = 1,
		ControlCode_Left = 2,
		ControlCode_Right = 3,
		ControlCode_Attack = 4,
		ControlCode_Activate = 5,
		ControlCode_Block = 6,
		ControlCode_ReadyItem = 7,
		ControlCode_Crouch = 8,
		ControlCode_Run = 9,
		ControlCode_AlwaysRun = 10,
		ControlCode_AutoMove = 11,
		ControlCode_Jump = 12,
		ControlCode_TogglePOV = 13,
		ControlCode_MenuMode = 14,
		ControlCode_Rest = 15,
		ControlCode_VATS = 16,
		ControlCode_Hotkey1 = 17,
		ControlCode_Hotkey2 = 18,
		ControlCode_Hotkey3 = 19,
		ControlCode_Hotkey4 = 20,
		ControlCode_Hotkey5 = 21,
		ControlCode_Hotkey6 = 22,
		ControlCode_Hotkey7 = 23,
		ControlCode_Hotkey8 = 24,
		ControlCode_Quicksave = 25,
		ControlCode_Quickload = 26,
		ControlCode_Grab = 27,
	};

	enum ScanCodes
	{
		ScanCode_Escape = 1,
		ScanCode_Console = 41,
	};

	enum FormTypes
	{
		// reference walking
		FormType_Actor = 200,
		FormType_Inventory = 201,
	};

	enum GlobalValues
	{
		Global_GameYear = 0x35,
		Global_GameMonth = 0x36,
		Global_GameDay = 0x37,
		Global_GameHour = 0x38,

		Global_TimeScale = 0x3A,
	};
}

/**
 * \brief The programming interface to the game
 *
 * The API class provides facilities to translate an engine command to a stream of bytes which can be interpreted by vaultmp DLL.
 * It also takes the results (data which has previously been retrieved from vaultmp DLL) and translates them into a usable form.
 */

class API
{
	public:
		typedef std::tuple<unsigned int, std::vector<double>, double, bool> CommandResult;
		typedef std::vector<std::unique_ptr<unsigned char[]>> CommandParsed;

	private:
		typedef std::deque<std::tuple<unsigned int, std::vector<double>, unsigned int>> CommandQueue;
		typedef std::unordered_map<std::string, std::pair<std::string, unsigned short>> FunctionMap;
		typedef std::unordered_map<std::string, unsigned char> ValueMap;
		typedef std::set<unsigned char> ValueList;

#ifdef VAULTMP_DEBUG
		static DebugInput<API> debug;
#endif

		struct op_Arg1;
		struct op_Arg2;
		struct op_Arg3;
		struct op_Arg4;
		struct op_Arg5;
		struct op_Arg6;
		struct op_Arg7;
		struct op_Arg8;
		struct op_default;

		static FunctionMap functions;
		static ValueMap values;
		static ValueMap axis;
		static ValueMap anims;
		static ValueList controls;
		static CommandQueue queue;

		static const std::pair<std::string, unsigned short>& RetrieveFunction(const std::string& name);
		static unsigned char* BuildCommandStream(std::vector<double>&& info, unsigned int key, unsigned char* command, unsigned int size);

		static std::vector<double> ParseCommand(const char* cmd_, const char* def, op_default* result, unsigned short opcode);

	protected:
		/**
		 * \brief Translates commands to a stream of bytes
		 *
		 * Takes a STL vector as an argument
		 * Also takes an optional unsigned key (usually comes from the Lockable extension class) which will automatically associated with each parsed command.
		 * Returns a STL vector containing the parsed commands (allocated on the heap; you are required to free them when you don't need them anymore).
		 */
		static CommandParsed Translate(const std::vector<std::string>& cmd, unsigned int key = 0);

		/**
		 * \brief Translates a result from vaultmp DLL
		 *
		 * Given the retrieved byte stream, will attempt to translate it to CommandResult's.
		 * CommandResult is of the form pair<pair<pair<unsigned int, vector<double> >, double>, bool>
		 * bool indicates if the command was successful.
		 * double is the result value of the command.
		 * unsigned int is the key provided to the corresponding Translate call.
		 * vector<double> is the argument list of the executed command; the first element is always the opcode of the function
		 */
		static std::vector<CommandResult> Translate(unsigned char* stream);

		API() = delete;

	public:
		/**
		 * \brief Initializes the API. Must be called before the API can be used
		 */
		static void Initialize();

		/**
		 * \brief Must be called when you are finished with the current API environment.
		 */
		static void Terminate();


		/**
		 * \brief Given the string representation of an actor value, returns the hex code. 0xFF indicates an error
		 */
		static unsigned char RetrieveValue(const char* value);
		/**
		 * \brief Given the string representation of an axis value, returns the hex code. 0xFF indicates an error
		 */
		static unsigned char RetrieveAxis(const char* axis);
		/**
		 * \brief Given the string representation of an animation value, returns the hex code. 0xFF indicates an error
		 */
		static unsigned char RetrieveAnim(const char* anim);
		/**
		 * \brief Returns true if the given value is a valid control code
		 */
		static bool IsControl(unsigned char control);
		/**
		 * \brief Given the hex code of an actor value, returns the string representation. An empty string indicates an error
		 */
		static std::string RetrieveValue_Reverse(unsigned char value);
		/**
		 * \brief Given the hex code of an axis value, returns the string representation. An empty string indicates an error
		 */
		static std::string RetrieveAxis_Reverse(unsigned char axis);
		/**
		 * \brief Given the hex code of an animation value, returns the string representation. An empty string indicates an error
		 */
		static std::string RetrieveAnim_Reverse(unsigned char anim);
		/**
		 * \brief Returns a STL vector containing every available actor value hex code
		 */
		static std::vector<unsigned char> RetrieveAllValues();
		/**
		 * \brief Returns a STL vector containing every available axis value hex code
		 */
		static std::vector<unsigned char> RetrieveAllAxis();
		/**
		 * \brief Returns a STL vector containing every available animation value hex code
		 */
		static std::vector<unsigned char> RetrieveAllAnims();
		/**
		 * \brief Returns a STL vector containing every available control code
		 */
		static std::vector<unsigned char> RetrieveAllControls();
		/**
		 * \brief Returns a STL vector containing every available actor value string representation
		 */
		static std::vector<std::string> RetrieveAllValues_Reverse();
		/**
		 * \brief Returns a STL vector containing every available axis value string representation
		 */
		static std::vector<std::string> RetrieveAllAxis_Reverse();
		/**
		 * \brief Returns a STL vector containing every available animation value string representation
		 */
		static std::vector<std::string> RetrieveAllAnims_Reverse();
};

#endif
