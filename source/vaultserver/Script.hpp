#ifndef SCRIPT_H
#define SCRIPT_H

#include "vaultserver.hpp"
#include "RakNet.hpp"
#include "ItemList.hpp"
#include "ScriptFunction.hpp"
#include "GameFactory.hpp"
#include "../Item.hpp"
#include "Player.hpp"
#include "Button.hpp"
#include "Text.hpp"
#include "Edit.hpp"
#include "Checkbox.hpp"
#include "RadioButton.hpp"
#include "List.hpp"
#include "Dedicated.hpp"
#include "PAWN.hpp"
#include "boost/any.hpp"

#ifdef __WIN32__
#include <winsock2.h>
#else
#include <dlfcn.h>
#endif

#include <vector>
#include <unordered_map>
#include <memory>
#include <chrono>
#include <regex>

template<typename T> struct sizeof_void { enum { value = sizeof(T) }; };
template<> struct sizeof_void<void> { enum { value = 0 }; };

template<typename T, size_t t> struct TypeChar { static_assert(!t, "Unsupported type in variadic type list"); };
template<typename T> struct TypeChar<T*, sizeof(void*)> { enum { value = 'p' }; };
template<> struct TypeChar<double*, sizeof(double*)> { enum { value = 'd' }; };
template<> struct TypeChar<RakNet::NetworkID**, sizeof(RakNet::NetworkID**)> { enum { value = 'n' }; };
template<typename T> struct TypeChar<T, sizeof(uint8_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint16_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint32_t)> { enum { value = std::is_signed<T>::value ? 'q' : 'i' }; };
template<typename T> struct TypeChar<T, sizeof(uint64_t)> { enum { value = std::is_signed<T>::value ? 'w' : 'l' }; };
template<> struct TypeChar<double, sizeof(double)> { enum { value = 'f' }; };
template<> struct TypeChar<char*, sizeof(char*)> { enum { value = 's' }; };
template<> struct TypeChar<const char*, sizeof(const char*)> { enum { value = 's' }; };
template<> struct TypeChar<void, sizeof_void<void>::value> { enum { value = 'v' }; };

template<const char t> struct CharType { static_assert(!t, "Unsupported type in variadic type list"); };
template<> struct CharType<'p'> { typedef void* type; };
template<> struct CharType<'d'> { typedef double* type; };
template<> struct CharType<'n'> { typedef RakNet::NetworkID** type; };
template<> struct CharType<'q'> { typedef signed int type; };
template<> struct CharType<'i'> { typedef unsigned int type; };
template<> struct CharType<'w'> { typedef signed long long type; };
template<> struct CharType<'l'> { typedef unsigned long long type; };
template<> struct CharType<'f'> { typedef double type; };
template<> struct CharType<'s'> { typedef const char* type; };
template<> struct CharType<'v'> { typedef void type; };

template<typename... Types>
struct TypeString {
	static constexpr char value[sizeof...(Types) + 1] = {
		TypeChar<Types, sizeof(Types)>::value...
	};
};

template<typename R, typename... Types>
using Function = R(*)(Types...);

template<typename R>
using FunctionEllipsis = R(*)(...);

struct ScriptIdentity
{
	const char* types;
	const char ret;
	const unsigned int numargs;

	constexpr bool matches(const char* types, const unsigned int N = 0) {
		return N < numargs ? this->types[N] == types[N] && matches(types, N + 1) : this->types[N] == types[N];
	}

	template<typename R, typename... Types>
	constexpr ScriptIdentity(Function<R, Types...>) : types(TypeString<Types...>::value), ret(TypeChar<R, sizeof_void<R>::value>::value), numargs(sizeof(TypeString<Types...>::value) - 1) {}
};

struct ScriptFunctionPointer : public ScriptIdentity
{
	Function<void> addr;

	template<typename R, typename... Types>
	constexpr ScriptFunctionPointer(Function<R, Types...> addr) : ScriptIdentity(addr), addr(reinterpret_cast<Function<void>>(addr)) {}
};

struct ScriptFunctionData
{
	const char* name;
	const ScriptFunctionPointer func;

	constexpr ScriptFunctionData(const char* name, ScriptFunctionPointer func) : name(name), func(func) {}
};

struct ScriptCallbackData
{
	const char* name;
	const unsigned int index;
	const ScriptIdentity callback;

	template<size_t N>
	constexpr ScriptCallbackData(const char(&name)[N], ScriptIdentity callback) : name(name), index(Utils::hash(name)), callback(callback) {}
};

template<typename R = void*>
struct SystemInterface
{
#ifdef __WIN32__
	typedef HMODULE lib_t;
#else
	typedef void* lib_t;
#endif

	union
	{
		R result;
#ifdef __WIN32__
		decltype(GetProcAddress(lib_t(), nullptr)) data;
#else
		decltype(dlsym(lib_t(), nullptr)) data;
#endif
	};

	static_assert(sizeof(result) == sizeof(data), "R should have the same size");

	SystemInterface() : data(nullptr) {}
	explicit operator bool() { return data; }

#ifdef __WIN32__
	SystemInterface(lib_t handle, const char* name) : data(GetProcAddress(handle, name)) {}
#else
	SystemInterface(lib_t handle, const char* name) : data(dlsym(handle, name)) {}
#endif
};

/**
 * \brief Maintains communication with a script
 *
 * A script can be either a C++ or PAWN script
 */

class Script
{
	private:
		Script(const char* path);

		union
		{
			SystemInterface<>::lib_t lib;
			AMX* amx;
		};

		bool cpp_script;
		std::unordered_map<unsigned int, FunctionEllipsis<void>> callbacks_;

		static void GetArguments(std::vector<boost::any>& params, va_list args, const std::string& def);

		template<typename R>
		R GetScript(const char* name)
		{
			if (cpp_script)
				return SystemInterface<R>(lib, name).result;
			else
				return reinterpret_cast<R>(PAWN::IsCallbackPresent(amx, name));
		}

		template<typename R>
		bool SetScript(const char* name, R value)
		{
			if (!cpp_script)
				return false;

			SystemInterface<R*> result(lib, name);

			if (result)
				*result.result = value;

			return result.operator bool();
		}

		typedef std::vector<std::unique_ptr<Script>> ScriptList;
		typedef std::unordered_map<unsigned int, std::vector<unsigned int>> DeletedObjects;
		typedef std::pair<std::chrono::system_clock::time_point, double> GameTime;
		typedef unsigned int GameWeather;

		static ScriptList scripts;
		static DeletedObjects deletedStatic;
		static GameTime time;
		static GameWeather weather;

		Script(const Script&) = delete;
		Script& operator=(const Script&) = delete;

	public:
		~Script();

		static void LoadScripts(char* scripts, char* base);
		static void Initialize();
		static void UnloadScripts();

		static RakNet::NetworkID CreateTimer(ScriptFunc timer, unsigned int interval) noexcept;
		static RakNet::NetworkID CreateTimerEx(ScriptFunc timer, unsigned int interval, const char* def, ...) noexcept;
		static RakNet::NetworkID CreateTimerPAWN(ScriptFuncPAWN timer, AMX* amx, unsigned int interval) noexcept;
		static RakNet::NetworkID CreateTimerPAWNEx(ScriptFuncPAWN timer, AMX* amx, unsigned int interval, const char* def, const std::vector<boost::any>& args) noexcept;
		static void SetupObject(Object* object, unsigned int cell, float X, float Y, float Z) noexcept;
		static void SetupItem(Item* item, unsigned int cell, float X, float Y, float Z) noexcept;
		static void SetupContainer(Container* container, unsigned int cell, float X, float Y, float Z) noexcept;
		static void SetupActor(Actor* actor, unsigned int cell, float X, float Y, float Z) noexcept;
		static void KillTimer(RakNet::NetworkID id = 0) noexcept;
		static void MakePublic(ScriptFunc _public, const char* name, const char* def) noexcept;
		static void MakePublicPAWN(ScriptFuncPAWN _public, AMX* amx, const char* name, const char* def) noexcept;
		static unsigned long long CallPublic(const char* name, ...) noexcept;
		static unsigned long long CallPublicPAWN(const char* name, const std::vector<boost::any>& args) noexcept;
		static bool IsPAWN(const char* name) noexcept;
		static const DeletedObjects& GetDeletedStatic() noexcept { return deletedStatic; }

		static unsigned long long Timer_Respawn(RakNet::NetworkID id) noexcept;
		static unsigned long long Timer_GameTime() noexcept;

		static const char* ValueToString(unsigned char index) noexcept;
		static const char* AxisToString(unsigned char index) noexcept;
		static const char* AnimToString(unsigned char index) noexcept;
		static const char* BaseToString(unsigned int baseID) noexcept;
		static const char* BaseToType(unsigned int baseID) noexcept;

		static bool Kick(RakNet::NetworkID id) noexcept;
		static bool UIMessage(RakNet::NetworkID id, const char* message, unsigned char emoticon) noexcept;
		static bool ChatMessage(RakNet::NetworkID id, const char* message) noexcept;
		static void SetSpawnCell(unsigned int cell) noexcept;
		static void SetGameWeather(unsigned int weather) noexcept;
		static void SetGameTime(signed long long time) noexcept;
		static void SetGameYear(unsigned int year) noexcept;
		static void SetGameMonth(unsigned int month) noexcept;
		static void SetGameDay(unsigned int day) noexcept;
		static void SetGameHour(unsigned int hour) noexcept;
		static void SetTimeScale(double scale) noexcept;
		static bool IsValid(RakNet::NetworkID id) noexcept;
		static bool IsReference(RakNet::NetworkID id) noexcept;
		static bool IsObject(RakNet::NetworkID id) noexcept;
		static bool IsItem(RakNet::NetworkID id) noexcept;
		static bool IsContainer(RakNet::NetworkID id) noexcept;
		static bool IsActor(RakNet::NetworkID id) noexcept;
		static bool IsPlayer(RakNet::NetworkID id) noexcept;
		static bool IsInterior(unsigned int cell) noexcept;
		static bool IsItemList(RakNet::NetworkID id) noexcept;
		static bool IsWindow(RakNet::NetworkID id) noexcept;
		static bool IsButton(RakNet::NetworkID id) noexcept;
		static bool IsText(RakNet::NetworkID id) noexcept;
		static bool IsEdit(RakNet::NetworkID id) noexcept;
		static bool IsCheckbox(RakNet::NetworkID id) noexcept;
		static bool IsRadioButton(RakNet::NetworkID id) noexcept;
		static bool IsListItem(RakNet::NetworkID id) noexcept;
		static bool IsList(RakNet::NetworkID id) noexcept;
		static bool IsChatbox(RakNet::NetworkID id) noexcept;
		static unsigned int GetConnection(RakNet::NetworkID id) noexcept;
		static unsigned int GetList(unsigned int type, RakNet::NetworkID** data) noexcept;
		static unsigned int GetGameWeather() noexcept;
		static signed long long GetGameTime() noexcept;
		static unsigned int GetGameYear() noexcept;
		static unsigned int GetGameMonth() noexcept;
		static unsigned int GetGameDay() noexcept;
		static unsigned int GetGameHour() noexcept;
		static double GetTimeScale() noexcept;

		static RakNet::NetworkID GetID(unsigned int refID) noexcept;
		static unsigned int GetReference(RakNet::NetworkID id) noexcept;
		static unsigned int GetBase(RakNet::NetworkID id) noexcept;
		static void GetPos(RakNet::NetworkID id, double* X, double* Y, double* Z) noexcept;
		static void GetAngle(RakNet::NetworkID id, double* X, double* Y, double* Z) noexcept;
		static unsigned int GetCell(RakNet::NetworkID id) noexcept;
		static unsigned int GetLock(RakNet::NetworkID id) noexcept;
		static unsigned int GetOwner(RakNet::NetworkID id) noexcept;
		static const char* GetBaseName(RakNet::NetworkID id) noexcept;
		static bool IsNearPoint(RakNet::NetworkID id, double X, double Y, double Z, double R) noexcept;
		static RakNet::NetworkID GetItemContainer(RakNet::NetworkID id) noexcept;
		static unsigned int GetItemCount(RakNet::NetworkID id) noexcept;
		static double GetItemCondition(RakNet::NetworkID id) noexcept;
		static bool GetItemEquipped(RakNet::NetworkID id) noexcept;
		static bool GetItemSilent(RakNet::NetworkID id) noexcept;
		static bool GetItemStick(RakNet::NetworkID id) noexcept;
		static unsigned int GetContainerItemCount(RakNet::NetworkID id, unsigned int baseID) noexcept;
		static unsigned int GetContainerItemList(RakNet::NetworkID id, RakNet::NetworkID** data) noexcept;
		static double GetActorValue(RakNet::NetworkID id, unsigned char index) noexcept;
		static double GetActorBaseValue(RakNet::NetworkID id, unsigned char index) noexcept;
		static unsigned int GetActorIdleAnimation(RakNet::NetworkID id) noexcept;
		static unsigned char GetActorMovingAnimation(RakNet::NetworkID id) noexcept;
		static unsigned char GetActorWeaponAnimation(RakNet::NetworkID id) noexcept;
		static bool GetActorAlerted(RakNet::NetworkID id) noexcept;
		static bool GetActorSneaking(RakNet::NetworkID id) noexcept;
		static bool GetActorDead(RakNet::NetworkID id) noexcept;
		static unsigned int GetActorBaseRace(RakNet::NetworkID id) noexcept;
		static bool GetActorBaseSex(RakNet::NetworkID id) noexcept;
		static bool IsActorJumping(RakNet::NetworkID id) noexcept;
		static unsigned int GetPlayerRespawnTime(RakNet::NetworkID id) noexcept;
		static unsigned int GetPlayerSpawnCell(RakNet::NetworkID id) noexcept;
		static bool GetPlayerConsoleEnabled(RakNet::NetworkID id) noexcept;
		static unsigned int GetPlayerWindowCount(RakNet::NetworkID id) noexcept;
		static unsigned int GetPlayerWindowList(RakNet::NetworkID id, RakNet::NetworkID** data) noexcept;
		static RakNet::NetworkID GetPlayerChatboxWindow(RakNet::NetworkID id) noexcept;

		static RakNet::NetworkID CreateObject(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept;
		static bool CreateVolatile(RakNet::NetworkID id, unsigned int baseID, double aX, double aY, double aZ) noexcept;
		static bool DestroyObject(RakNet::NetworkID id) noexcept;
		static bool Activate(RakNet::NetworkID id, RakNet::NetworkID actor) noexcept;
		static bool SetPos(RakNet::NetworkID id, double X, double Y, double Z) noexcept;
		static bool SetAngle(RakNet::NetworkID id, double X, double Y, double Z) noexcept;
		static bool SetCell(RakNet::NetworkID id, unsigned int cell, double X, double Y, double Z) noexcept;
		static bool SetLock(RakNet::NetworkID id, RakNet::NetworkID actor, unsigned int lock) noexcept;
		static bool SetOwner(RakNet::NetworkID id, unsigned int owner) noexcept;
		static bool SetBaseName(RakNet::NetworkID id, const char* name) noexcept;
		static bool PlaySound(RakNet::NetworkID id, unsigned int sound);
		static RakNet::NetworkID CreateItem(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept;
		static RakNet::NetworkID SetItemContainer(RakNet::NetworkID id, RakNet::NetworkID container) noexcept;
		static bool SetItemCount(RakNet::NetworkID id, unsigned int count) noexcept;
		static bool SetItemCondition(RakNet::NetworkID id, double condition) noexcept;
		static bool SetItemEquipped(RakNet::NetworkID id, bool equipped, bool silent, bool stick) noexcept;
		static RakNet::NetworkID CreateContainer(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept;
		static RakNet::NetworkID CreateItemList(RakNet::NetworkID source, unsigned int baseID) noexcept;
		static bool DestroyItemList(RakNet::NetworkID id) noexcept;
		static RakNet::NetworkID AddItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, double condition, bool silent) noexcept;
		static void AddItemList(RakNet::NetworkID id, RakNet::NetworkID source, unsigned int baseID) noexcept;
		static unsigned int RemoveItem(RakNet::NetworkID id, unsigned int baseID, unsigned int count, bool silent) noexcept;
		static void RemoveAllItems(RakNet::NetworkID id) noexcept;
		static RakNet::NetworkID CreateActor(unsigned int baseID, unsigned int cell, double X, double Y, double Z) noexcept;
		static void SetActorValue(RakNet::NetworkID id, unsigned char index, double value) noexcept;
		static void SetActorBaseValue(RakNet::NetworkID id, unsigned char index, double value) noexcept;
		static bool EquipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick) noexcept;
		static bool UnequipItem(RakNet::NetworkID id, unsigned int baseID, bool silent, bool stick) noexcept;
		static bool PlayIdle(RakNet::NetworkID id, unsigned int idle) noexcept;
		static bool SetActorMovingAnimation(RakNet::NetworkID id, unsigned char anim) noexcept;
		static bool SetActorWeaponAnimation(RakNet::NetworkID id, unsigned char anim) noexcept;
		static bool SetActorAlerted(RakNet::NetworkID id, bool alerted) noexcept;
		static bool SetActorSneaking(RakNet::NetworkID id, bool sneaking) noexcept;
		static bool FireWeapon(RakNet::NetworkID id) noexcept;
		static void KillActor(RakNet::NetworkID id, RakNet::NetworkID actor, unsigned short limbs, signed char cause) noexcept;
		static bool SetActorBaseRace(RakNet::NetworkID id, unsigned int race) noexcept;
		static bool AgeActorBaseRace(RakNet::NetworkID id, signed int age) noexcept;
		static bool SetActorBaseSex(RakNet::NetworkID id, bool female) noexcept;
		static void SetPlayerRespawnTime(RakNet::NetworkID id, unsigned int respawn) noexcept;
		static void SetPlayerSpawnCell(RakNet::NetworkID id, unsigned int cell) noexcept;
		static void SetPlayerConsoleEnabled(RakNet::NetworkID id, bool enabled) noexcept;
		static bool AttachWindow(RakNet::NetworkID id, RakNet::NetworkID window) noexcept;
		static bool DetachWindow(RakNet::NetworkID id, RakNet::NetworkID window) noexcept;
		static void ForceWindowMode(RakNet::NetworkID id, bool enabled) noexcept;

		static RakNet::NetworkID GetWindowParent(RakNet::NetworkID id) noexcept;
		static RakNet::NetworkID GetWindowRoot(RakNet::NetworkID id) noexcept;
		static unsigned int GetWindowChildCount(RakNet::NetworkID id) noexcept;
		static unsigned int GetWindowChildList(RakNet::NetworkID id, RakNet::NetworkID** data) noexcept;
		static void GetWindowPos(RakNet::NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y) noexcept;
		static void GetWindowSize(RakNet::NetworkID id, double* X, double* Y, double* offset_X, double* offset_Y) noexcept;
		static bool GetWindowVisible(RakNet::NetworkID id) noexcept;
		static bool GetWindowLocked(RakNet::NetworkID id) noexcept;
		static const char* GetWindowText(RakNet::NetworkID id) noexcept;
		static unsigned int GetEditMaxLength(RakNet::NetworkID id) noexcept;
		static const char* GetEditValidation(RakNet::NetworkID id) noexcept;
		static bool GetCheckboxSelected(RakNet::NetworkID id) noexcept;
		static bool GetRadioButtonSelected(RakNet::NetworkID id) noexcept;
		static unsigned int GetRadioButtonGroup(RakNet::NetworkID id) noexcept;
		static bool GetListMultiSelect(RakNet::NetworkID id) noexcept;
		static unsigned int GetListItemCount(RakNet::NetworkID id) noexcept;
		static unsigned int GetListItemList(RakNet::NetworkID id, RakNet::NetworkID** data) noexcept;
		static unsigned int GetListSelectedItemCount(RakNet::NetworkID id) noexcept;
		static unsigned int GetListSelectedItemList(RakNet::NetworkID id, RakNet::NetworkID** data) noexcept;
		static RakNet::NetworkID GetListItemContainer(RakNet::NetworkID id) noexcept;
		static bool GetListItemSelected(RakNet::NetworkID id) noexcept;
		static const char* GetListItemText(RakNet::NetworkID id) noexcept;

		static RakNet::NetworkID (CreateWindow)(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static bool DestroyWindow(RakNet::NetworkID id) noexcept;
		static bool AddChildWindow(RakNet::NetworkID id, RakNet::NetworkID child) noexcept;
		static bool RemoveChildWindow(RakNet::NetworkID id, RakNet::NetworkID child) noexcept;
		static bool SetWindowPos(RakNet::NetworkID id, double X, double Y, double offset_X, double offset_Y) noexcept;
		static bool SetWindowSize(RakNet::NetworkID id, double X, double Y, double offset_X, double offset_Y) noexcept;
		static bool SetWindowVisible(RakNet::NetworkID id, bool visible) noexcept;
		static bool SetWindowLocked(RakNet::NetworkID id, bool locked) noexcept;
		static bool SetWindowText(RakNet::NetworkID id, const char* text) noexcept;
		static RakNet::NetworkID CreateButton(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static RakNet::NetworkID CreateText(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static RakNet::NetworkID CreateEdit(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static bool SetEditMaxLength(RakNet::NetworkID id, unsigned int length) noexcept;
		static bool SetEditValidation(RakNet::NetworkID id, const char* validation) noexcept;
		static RakNet::NetworkID CreateCheckbox(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static bool SetCheckboxSelected(RakNet::NetworkID id, bool selected) noexcept;
		static RakNet::NetworkID CreateRadioButton(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static bool SetRadioButtonSelected(RakNet::NetworkID id, bool selected) noexcept;
		static bool SetRadioButtonGroup(RakNet::NetworkID id, unsigned int group);
		static RakNet::NetworkID CreateList(double posX, double posY, double sizeX, double sizeY, bool visible, bool locked, const char* text) noexcept;
		static bool SetListMultiSelect(RakNet::NetworkID id, bool multiselect);
		static RakNet::NetworkID AddListItem(RakNet::NetworkID id, const char* text) noexcept;
		static bool RemoveListItem(RakNet::NetworkID id) noexcept;
		static RakNet::NetworkID SetListItemContainer(RakNet::NetworkID id, RakNet::NetworkID container) noexcept;
		static bool SetListItemSelected(RakNet::NetworkID id, bool selected) noexcept;
		static bool SetListItemText(RakNet::NetworkID id, const char* text) noexcept;

		static constexpr ScriptFunctionData functions[] {
			{"timestamp", Utils::timestamp},
			{"CreateTimer", Script::CreateTimer},
			{"CreateTimerEx", reinterpret_cast<Function<void>>(Script::CreateTimerEx)},
			{"KillTimer", Script::KillTimer},
			{"MakePublic", Script::MakePublic},
			{"CallPublic", reinterpret_cast<Function<void>>(Script::CallPublic)},
			{"IsPAWN", Script::IsPAWN},

			{"SetServerName", Dedicated::SetServerName},
			{"SetServerMap", Dedicated::SetServerMap},
			{"SetServerRule", Dedicated::SetServerRule},
			{"GetMaximumPlayers", Dedicated::GetMaximumPlayers},
			{"GetCurrentPlayers", Dedicated::GetCurrentPlayers},

			{"ValueToString", Script::ValueToString},
			{"AxisToString", Script::AxisToString},
			{"AnimToString", Script::AnimToString},
			{"BaseToString", Script::BaseToString},
			{"BaseToType", Script::BaseToType},

			{"Kick", Script::Kick},
			{"UIMessage", Script::UIMessage},
			{"ChatMessage", Script::ChatMessage},
			{"SetRespawnTime", Player::SetRespawnTime},
			{"SetSpawnCell", Script::SetSpawnCell},
			{"SetConsoleEnabled", Player::SetConsoleEnabled},
			{"SetGameWeather", Script::SetGameWeather},
			{"SetGameTime", Script::SetGameTime},
			{"SetGameYear", Script::SetGameYear},
			{"SetGameMonth", Script::SetGameMonth},
			{"SetGameDay", Script::SetGameDay},
			{"SetGameHour", Script::SetGameHour},
			{"SetTimeScale", Script::SetTimeScale},
			{"IsValid", Script::IsValid},
			{"IsReference", Script::IsReference},
			{"IsObject", Script::IsObject},
			{"IsItem", Script::IsItem},
			{"IsContainer", Script::IsContainer},
			{"IsActor", Script::IsActor},
			{"IsPlayer", Script::IsPlayer},
			{"IsCell", DB::Record::IsValidCell},
			{"IsInterior", Script::IsInterior},
			{"IsItemList", Script::IsItemList},
			{"IsWindow", Script::IsWindow},
			{"IsButton", Script::IsButton},
			{"IsText", Script::IsText},
			{"IsEdit", Script::IsEdit},
			{"IsCheckbox", Script::IsCheckbox},
			{"IsRadioButton", Script::IsRadioButton},
			{"IsListItem", Script::IsListItem},
			{"IsList", Script::IsList},
			{"IsChatbox", Script::IsChatbox},
			{"GetType", (unsigned int(*)(RakNet::NetworkID)) GameFactory::GetType},
			{"GetConnection", Script::GetConnection},
			{"GetCount", GameFactory::GetCount},
			{"GetList", Script::GetList},
			{"GetRespawnTime", Player::GetRespawnTime},
			{"GetSpawnCell", Player::GetSpawnCell},
			{"GetConsoleEnabled", Player::GetConsoleEnabled},
			{"GetGameWeather", Script::GetGameWeather},
			{"GetGameTime", Script::GetGameTime},
			{"GetGameYear", Script::GetGameYear},
			{"GetGameMonth", Script::GetGameMonth},
			{"GetGameDay", Script::GetGameDay},
			{"GetGameHour", Script::GetGameHour},
			{"GetTimeScale", Script::GetTimeScale},

			{"GetID", Script::GetID},
			{"GetReference", Script::GetReference},
			{"GetBase", Script::GetBase},
			{"GetPos", Script::GetPos},
			{"GetAngle", Script::GetAngle},
			{"GetCell", Script::GetCell},
			{"GetLock", Script::GetLock},
			{"GetOwner", Script::GetOwner},
			{"GetBaseName", Script::GetBaseName},
			{"IsNearPoint", Script::IsNearPoint},
			{"GetItemContainer", Script::GetItemContainer},
			{"GetItemCount", Script::GetItemCount},
			{"GetItemCondition", Script::GetItemCondition},
			{"GetItemEquipped", Script::GetItemEquipped},
			{"GetItemSilent", Script::GetItemSilent},
			{"GetItemStick", Script::GetItemStick},
			{"GetContainerItemCount", Script::GetContainerItemCount},
			{"GetContainerItemList", Script::GetContainerItemList},
			{"GetActorValue", Script::GetActorValue},
			{"GetActorBaseValue", Script::GetActorBaseValue},
			{"GetActorIdleAnimation", Script::GetActorIdleAnimation},
			{"GetActorMovingAnimation", Script::GetActorMovingAnimation},
			{"GetActorWeaponAnimation", Script::GetActorWeaponAnimation},
			{"GetActorAlerted", Script::GetActorAlerted},
			{"GetActorSneaking", Script::GetActorSneaking},
			{"GetActorDead", Script::GetActorDead},
			{"GetActorBaseRace", Script::GetActorBaseRace},
			{"GetActorBaseSex", Script::GetActorBaseSex},
			{"IsActorJumping", Script::IsActorJumping},
			{"GetPlayerRespawnTime", Script::GetPlayerRespawnTime},
			{"GetPlayerSpawnCell", Script::GetPlayerSpawnCell},
			{"GetPlayerConsoleEnabled", Script::GetPlayerConsoleEnabled},
			{"GetPlayerWindowCount", Script::GetPlayerWindowCount},
			{"GetPlayerWindowList", Script::GetPlayerWindowList},
			{"GetPlayerChatboxWindow", Script::GetPlayerChatboxWindow},

			{"CreateVolatile", Script::CreateVolatile},
			{"CreateObject", Script::CreateObject},
			{"DestroyObject", Script::DestroyObject},
			{"Activate", Script::Activate},
			{"SetPos", Script::SetPos},
			{"SetAngle", Script::SetAngle},
			{"SetCell", Script::SetCell},
			{"SetLock", Script::SetLock},
			{"SetOwner", Script::SetOwner},
			{"SetBaseName", Script::SetBaseName},
			{"PlaySound", Script::PlaySound},
			{"CreateItem", Script::CreateItem},
			{"SetItemContainer", Script::SetItemContainer},
			{"SetItemCount", Script::SetItemCount},
			{"SetItemCondition", Script::SetItemCondition},
			{"SetItemEquipped", Script::SetItemEquipped},
			{"CreateContainer", Script::CreateContainer},
			{"CreateItemList", Script::CreateItemList},
			{"DestroyItemList", Script::DestroyItemList},
			{"AddItem", Script::AddItem},
			{"RemoveItem", Script::RemoveItem},
			{"RemoveAllItems", Script::RemoveAllItems},
			{"AddItemList", Script::AddItemList},
			{"CreateActor", Script::CreateActor},
			{"SetActorValue", Script::SetActorValue},
			{"SetActorBaseValue", Script::SetActorBaseValue},
			{"EquipItem", Script::EquipItem},
			{"UnequipItem", Script::UnequipItem},
			{"PlayIdle", Script::PlayIdle},
			{"SetActorMovingAnimation", Script::SetActorMovingAnimation},
			{"SetActorWeaponAnimation", Script::SetActorWeaponAnimation},
			{"SetActorAlerted", Script::SetActorAlerted},
			{"SetActorSneaking", Script::SetActorSneaking},
			{"FireWeapon", Script::FireWeapon},
			{"KillActor", Script::KillActor},
			{"SetActorBaseRace", Script::SetActorBaseRace},
			{"AgeActorBaseRace", Script::AgeActorBaseRace},
			{"SetActorBaseSex", Script::SetActorBaseSex},
			{"SetPlayerRespawnTime", Script::SetPlayerRespawnTime},
			{"SetPlayerSpawnCell", Script::SetPlayerSpawnCell},
			{"SetPlayerConsoleEnabled", Script::SetPlayerConsoleEnabled},
			{"AttachWindow", Script::AttachWindow},
			{"DetachWindow", Script::DetachWindow},
			{"ForceWindowMode", Script::ForceWindowMode},

			{"GetWindowParent", Script::GetWindowParent},
			{"GetWindowRoot", Script::GetWindowRoot},
			{"GetWindowChildCount", Script::GetWindowChildCount},
			{"GetWindowChildList", Script::GetWindowChildList},
			{"GetWindowPos", Script::GetWindowPos},
			{"GetWindowSize", Script::GetWindowSize},
			{"GetWindowVisible", Script::GetWindowVisible},
			{"GetWindowLocked", Script::GetWindowLocked},
			{"GetWindowText", Script::GetWindowText},
			{"GetEditMaxLength", Script::GetEditMaxLength},
			{"GetEditValidation", Script::GetEditValidation},
			{"GetCheckboxSelected", Script::GetCheckboxSelected},
			{"GetRadioButtonSelected", Script::GetRadioButtonSelected},
			{"GetRadioButtonGroup", Script::GetRadioButtonGroup},
			{"GetListMultiSelect", Script::GetListMultiSelect},
			{"GetListItemCount", Script::GetListItemCount},
			{"GetListItemList", Script::GetListItemList},
			{"GetListSelectedItemCount", Script::GetListSelectedItemCount},
			{"GetListSelectedItemList", Script::GetListSelectedItemList},
			{"GetListItemContainer", Script::GetListItemContainer},
			{"GetListItemSelected", Script::GetListItemSelected},
			{"GetListItemText", Script::GetListItemText},

			{"CreateWindow", Script::CreateWindow},
			{"DestroyWindow", Script::DestroyWindow},
			{"AddChildWindow", Script::AddChildWindow},
			{"RemoveChildWindow", Script::RemoveChildWindow},
			{"SetWindowPos", Script::SetWindowPos},
			{"SetWindowSize", Script::SetWindowSize},
			{"SetWindowVisible", Script::SetWindowVisible},
			{"SetWindowLocked", Script::SetWindowLocked},
			{"SetWindowText", Script::SetWindowText},
			{"CreateButton", Script::CreateButton},
			{"CreateText", Script::CreateText},
			{"CreateEdit", Script::CreateEdit},
			{"SetEditMaxLength", Script::SetEditMaxLength},
			{"SetEditValidation", Script::SetEditValidation},
			{"CreateCheckbox", Script::CreateCheckbox},
			{"SetCheckboxSelected", Script::SetCheckboxSelected},
			{"CreateRadioButton", Script::CreateRadioButton},
			{"SetRadioButtonSelected", Script::SetRadioButtonSelected},
			{"SetRadioButtonGroup", Script::SetRadioButtonGroup},
			{"CreateList", Script::CreateList},
			{"SetListMultiSelect", Script::SetListMultiSelect},
			{"AddListItem", Script::AddListItem},
			{"RemoveListItem", Script::RemoveListItem},
			{"SetListItemContainer", Script::SetListItemContainer},
			{"SetListItemSelected", Script::SetListItemSelected},
			{"SetListItemText", Script::SetListItemText},
		};

		static constexpr ScriptCallbackData callbacks[] {
			{"OnCreate", Function<void, RakNet::NetworkID>()},
			{"OnDestroy", Function<void, RakNet::NetworkID>()},
			{"OnSpawn", Function<void, RakNet::NetworkID>()},
			{"OnActivate", Function<void, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnCellChange", Function<void, RakNet::NetworkID, unsigned int>()},
			{"OnLockChange", Function<void, RakNet::NetworkID, RakNet::NetworkID, unsigned int>()},
			{"OnItemCountChange", Function<void, RakNet::NetworkID, unsigned int>()},
			{"OnItemConditionChange", Function<void, RakNet::NetworkID, double>()},
			{"OnItemEquippedChange", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorValueChange", Function<void, RakNet::NetworkID, unsigned char, double>()},
			{"OnActorBaseValueChange", Function<void, RakNet::NetworkID, unsigned char, double>()},
			{"OnActorAlert", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorSneak", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorDeath", Function<void, RakNet::NetworkID, RakNet::NetworkID, unsigned short, signed char>()},
			{"OnActorPunch", Function<void, RakNet::NetworkID, bool>()},
			{"OnActorFireWeapon", Function<void, RakNet::NetworkID, unsigned int>()},
			{"OnPlayerDisconnect", Function<void, RakNet::NetworkID, Reason>()},
			{"OnPlayerRequestGame", Function<unsigned int, RakNet::NetworkID>()},
			{"OnPlayerChat", Function<bool, RakNet::NetworkID, char*>()},
			{"OnWindowMode", Function<void, RakNet::NetworkID, bool>()},
			{"OnWindowClick", Function<void, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnWindowReturn", Function<void, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnWindowTextChange", Function<void, RakNet::NetworkID, RakNet::NetworkID, const char*>()},
			{"OnCheckboxSelect", Function<void, RakNet::NetworkID, RakNet::NetworkID, bool>()},
			{"OnRadioButtonSelect", Function<void, RakNet::NetworkID, RakNet::NetworkID, RakNet::NetworkID>()},
			{"OnListItemSelect", Function<void, RakNet::NetworkID, RakNet::NetworkID, bool>()},
			{"OnClientAuthenticate", Function<bool, const char*, const char*>()},
			{"OnGameTimeChange", Function<void, unsigned int, unsigned int, unsigned int, unsigned int>()},
			{"OnServerInit", Function<void>()},
			{"OnServerExit", Function<void, bool>()},
		};

		static constexpr ScriptCallbackData const& CBD(const unsigned int I, const unsigned int N = 0) {
			return callbacks[N].index == I ? callbacks[N] : CBD(I, N + 1);
		}

		template<unsigned int I>
		using CBR = typename CharType<CBD(I).callback.ret>::type;

		template<size_t N>
		static constexpr unsigned int CBI(const char(&str)[N]) {
			return Utils::hash(str);
		}

		template<unsigned int I, bool B = false, typename... Args>
		static unsigned int Call(CBR<I>& result, Args&&... args) {
			constexpr ScriptCallbackData const& data = CBD(I);
			static_assert(data.callback.matches(TypeString<typename std::remove_reference<Args>::type...>::value), "Wrong number or types of arguments");

			unsigned int count = 0;

			for (auto& script : scripts)
			{
				if (!script->callbacks_.count(I))
					script->callbacks_.emplace(I, script->GetScript<FunctionEllipsis<void>>(data.name));

				auto callback = script->callbacks_[I];

				if (!callback)
					continue;

				if (script->cpp_script)
					result = reinterpret_cast<FunctionEllipsis<CBR<I>>>(callback)(std::forward<Args>(args)...);
				else
					result = static_cast<CBR<I>>(PAWN::Call(script->amx, data.name, data.callback.types, B, std::forward<Args>(args)...));

				++count;
			}

			return count;
		}

		template<unsigned int I, bool B = false, typename... Args>
		static unsigned int Call(Args&&... args) {
			constexpr ScriptCallbackData const& data = CBD(I);
			static_assert(data.callback.matches(TypeString<typename std::remove_reference<Args>::type...>::value), "Wrong number or types of arguments");

			unsigned int count = 0;

			for (auto& script : scripts)
			{
				if (!script->callbacks_.count(I))
					script->callbacks_.emplace(I, script->GetScript<FunctionEllipsis<void>>(data.name));

				auto callback = script->callbacks_[I];

				if (!callback)
					continue;

				if (script->cpp_script)
					reinterpret_cast<FunctionEllipsis<CBR<I>>>(callback)(std::forward<Args>(args)...);
				else
					PAWN::Call(script->amx, data.name, data.callback.types, B, std::forward<Args>(args)...);

				++count;
			}

			return count;
		}
};

#endif
