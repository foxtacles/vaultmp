/*
 *  vaultscript.h
 *  Don't change anything here
 */

#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <cstdint>

#ifndef __WIN32__
#ifndef __cdecl
#define __cdecl __attribute__((__cdecl__))
#endif
#define VAULTSCRIPT __attribute__ ((__visibility__("default"))) __cdecl
#else
#define VAULTSCRIPT __declspec(dllexport) __cdecl
#endif

namespace vaultmp
{
	enum Reason : uint8_t;

	enum State : bool
	{
		True    =   true,
		False   =   false,
	};

	enum Reference : uint32_t;
	enum Base : uint32_t;
	enum Cell : uint32_t;

	typedef uint32_t UCount;
	typedef int32_t Count;

	enum Interval : uint32_t
	{
		DEFAULT_PLAYER_RESPAWN  =   8000,
	};

	enum ID : uint64_t;
	enum Timer : uint64_t;
	enum Result : uint64_t;

	struct _hash_ID { inline size_t operator() (const ID& x) const { return std::hash<uint64_t>()((uint64_t) x); }};

	typedef void Void;
	typedef double Value;
	typedef Result(__cdecl* Function)();
	typedef std::string String;
	typedef std::vector<ID> IDVector;
	typedef std::unordered_set<ID, _hash_ID> IDSet;

	template <typename V>
	using IDHash = std::unordered_map<ID, V, _hash_ID>;

	enum class Index : uint8_t
	{
		FALLOUT3            =   0x01,
		NEWVEGAS            =   FALLOUT3 << 1,
		FALLOUT_GAMES       =   FALLOUT3 | NEWVEGAS,
		ALL_GAMES           =   FALLOUT_GAMES,

		MAX_PLAYER_NAME     =   16,
		MAX_PASSWORD_SIZE   =   16,
		MAX_MESSAGE_LENGTH  =   64,
	};

	enum class Type : uint8_t
	{
		ID_REFERENCE        =   0x01,
		ID_OBJECT           =   ID_REFERENCE << 1,
		ID_ITEM             =   ID_OBJECT << 1,
		ID_CONTAINER        =   ID_ITEM << 1,
		ID_ACTOR            =   ID_CONTAINER << 1,
		ID_PLAYER           =   ID_ACTOR << 1,

		ALL_OBJECTS         = (ID_OBJECT | ID_ITEM | ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_CONTAINERS      = (ID_CONTAINER | ID_ACTOR | ID_PLAYER),
		ALL_ACTORS          = (ID_ACTOR | ID_PLAYER),
	};
};

extern "C" {
	VAULTSCRIPT vaultmp::Void exec();

	VAULTSCRIPT vaultmp::Void OnSpawn(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void OnCellChange(vaultmp::ID, vaultmp::Cell);
	VAULTSCRIPT vaultmp::Void OnContainerItemChange(vaultmp::ID, vaultmp::Base, vaultmp::Count, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorBaseValueChange(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorAlert(vaultmp::ID, vaultmp::State);
	VAULTSCRIPT vaultmp::Void OnActorSneak(vaultmp::ID, vaultmp::State);
	VAULTSCRIPT vaultmp::Void OnActorDeath(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void OnActorEquipItem(vaultmp::ID, vaultmp::Base, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnActorUnequipItem(vaultmp::ID, vaultmp::Base, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void OnPlayerDisconnect(vaultmp::ID, vaultmp::Reason);
	VAULTSCRIPT vaultmp::Base OnPlayerRequestGame(vaultmp::ID);
	VAULTSCRIPT vaultmp::State OnClientAuthenticate(vaultmp::String, vaultmp::String);

	VAULTSCRIPT vaultmp::Void (*timestamp)();
	VAULTSCRIPT vaultmp::Timer (*CreateTimer)(vaultmp::Function, vaultmp::Interval);
	VAULTSCRIPT vaultmp::Timer (*CreateTimerEx)(vaultmp::Function, vaultmp::Interval, vaultmp::String, ...);
	VAULTSCRIPT vaultmp::Void (*KillTimer)(vaultmp::Timer);
	VAULTSCRIPT vaultmp::Void (*MakePublic)(vaultmp::Function, vaultmp::String, vaultmp::String);
	VAULTSCRIPT vaultmp::Result (*CallPublic)(vaultmp::String, ...);

	VAULTSCRIPT vaultmp::Void (*SetServerName)(vaultmp::String);
	VAULTSCRIPT vaultmp::Void (*SetServerMap)(vaultmp::String);
	VAULTSCRIPT vaultmp::Void (*SetServerRule)(vaultmp::String, vaultmp::String);
	VAULTSCRIPT vaultmp::Index (*GetGameCode)();
	VAULTSCRIPT vaultmp::UCount (*GetMaximumPlayers)();
	VAULTSCRIPT vaultmp::UCount (*GetCurrentPlayers)();

	VAULTSCRIPT vaultmp::String (*ValueToString)(vaultmp::Index);
	VAULTSCRIPT vaultmp::String (*AxisToString)(vaultmp::Index);
	VAULTSCRIPT vaultmp::String (*AnimToString)(vaultmp::Index);

	VAULTSCRIPT vaultmp::State (*UIMessage)(vaultmp::ID, vaultmp::String);
	VAULTSCRIPT vaultmp::Void (*SetRespawn)(vaultmp::Interval);
	VAULTSCRIPT vaultmp::State (*IsValid)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsObject)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsItem)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsContainer)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsActor)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsPlayer)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Type (*GetType)(vaultmp::ID);
	VAULTSCRIPT vaultmp::UCount (*GetCount)(vaultmp::Type);
	VAULTSCRIPT vaultmp::IDVector (*GetList)(vaultmp::Type);

	VAULTSCRIPT vaultmp::Reference (*GetReference)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Base (*GetBase)(vaultmp::ID);
	VAULTSCRIPT vaultmp::String (*GetName)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void (*GetPos)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
	VAULTSCRIPT vaultmp::Void (*GetAngle)(vaultmp::ID, vaultmp::Value&, vaultmp::Value&, vaultmp::Value&);
	VAULTSCRIPT vaultmp::Cell (*GetCell)(vaultmp::ID);
	VAULTSCRIPT vaultmp::UCount (*GetContainerItemCount)(vaultmp::ID, vaultmp::Base);
	VAULTSCRIPT vaultmp::Value (*GetActorValue)(vaultmp::ID, vaultmp::Index);
	VAULTSCRIPT vaultmp::Value (*GetActorBaseValue)(vaultmp::ID, vaultmp::Index);
	VAULTSCRIPT vaultmp::Index (*GetActorMovingAnimation)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*GetActorAlerted)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*GetActorSneaking)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*GetActorDead)(vaultmp::ID);
	VAULTSCRIPT vaultmp::State (*IsActorJumping)(vaultmp::ID);

	VAULTSCRIPT vaultmp::Void (*AddItem)(vaultmp::ID, vaultmp::Base, vaultmp::UCount, vaultmp::Value, vaultmp::State);
	VAULTSCRIPT vaultmp::UCount (*RemoveItem)(vaultmp::ID, vaultmp::Base, vaultmp::UCount, vaultmp::State);
	VAULTSCRIPT vaultmp::Void (*RemoveAllItems)(vaultmp::ID);
	VAULTSCRIPT vaultmp::Void (*SetActorValue)(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::Void (*SetActorBaseValue)(vaultmp::ID, vaultmp::Index, vaultmp::Value);
	VAULTSCRIPT vaultmp::State (*EquipItem)(vaultmp::ID, vaultmp::Base, vaultmp::State, vaultmp::State);
	VAULTSCRIPT vaultmp::State (*UnequipItem)(vaultmp::ID, vaultmp::Base, vaultmp::State, vaultmp::State);
	VAULTSCRIPT vaultmp::Void (*KillActor)(vaultmp::ID);

	VAULTSCRIPT vaultmp::Void (*SetPlayerRespawn)(vaultmp::ID, vaultmp::Interval);

}
