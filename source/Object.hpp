#ifndef OBJECT_H
#define OBJECT_H

#include "vaultmp.hpp"
#include "Reference.hpp"
#include "VaultVector.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

const unsigned int FLAG_ENABLED = 0x00000001;
const unsigned int FLAG_NOTSELF = FLAG_ENABLED << 1;
const unsigned int FLAG_SELF = FLAG_NOTSELF << 1;

/**
 * \brief Derives from Reference class and represents an object in-game
 *
 * Data specific to Objects are a name, XYZ position and angle, a cell and various states
 */

class Object : public Reference
{
		friend class GameFactory;

	private:
#ifdef VAULTMP_DEBUG
		static DebugInput<Object> debug;
#endif

		Value<std::string> object_Name;
		Value<std::tuple<float, float, float>> object_Game_Pos;
		Value<std::tuple<float, float, float>> object_Network_Pos;
		Value<std::tuple<float, float, float>> object_Angle;
		Value<unsigned int> cell_Game;
		Value<unsigned int> cell_Network;
		Value<bool> state_Enabled;
		Value<unsigned int> state_Lock;
		Value<unsigned int> state_Owner;

		void initialize();

		Object(const Object&) = delete;
		Object& operator=(const Object&) = delete;

#ifdef VAULTSERVER
		virtual void initializers() { this->SetBase(this->GetBase()); }
#endif

	protected:
		Object(unsigned int refID, unsigned int baseID);
		Object(unsigned int baseID) : Object(0x00000000, baseID) {}
		Object(const pPacket& packet);
		Object(pPacket&& packet) : Object(packet) {};

	public:
		virtual ~Object() noexcept;

#ifndef VAULTSERVER
		/**
		 * \brief Creates a Parameter containing a VaultFunctor initialized with the given flags
		 *
		 * Used to pass Object references matching the provided flags to the Interface
		 * Can also be used to pass data of a given Object to the Interface
		 */
		static FuncParameter CreateFunctor(unsigned int flags, RakNet::NetworkID id = 0);
#endif
		static bool IsValidCoordinate(float C);
		static bool IsValidAngle(unsigned char axis, float A);

		/**
		 * \brief Retrieves the Object's name
		 */
		const std::string& GetName() const;
		/**
		 * \brief Retrieves the Object's game coordinates
		 */
		const std::tuple<float, float, float>& GetGamePos() const;
		/**
		 * \brief Retrieves the Object's network coordinates
		 */
		const std::tuple<float, float, float>& GetNetworkPos() const;
		/**
		 * \brief Retrieves the Object's angles
		 */
		const std::tuple<float, float, float>& GetAngle() const;
		/**
		 * \brief Retrieves the Object's game cell
		 *
		 * This is the actual cell the object has in the game
		 */
		unsigned int GetGameCell() const;
		/**
		 * \brief Retrieves the Object's network cell
		 *
		 * This is the "real" cell which the object should be in
		 */
		unsigned int GetNetworkCell() const;
		/**
		 * \brief Retrieves the Object's enabled state
		 */
		bool GetEnabled() const;
		/**
		 * \brief Retrieves the Object's lock level
		 */
		unsigned int GetLockLevel() const;
		/**
		 * \brief Retrieves the Object's owner
		 */
		unsigned int GetOwner() const;

		/**
		 * \brief Sets the Object's name
		 */
		Lockable* SetName(const std::string& name);
		/**
		 * \brief Sets the Object's game coordiantes
		 */
		Lockable* SetGamePos(const std::tuple<float, float, float>& pos);
		/**
		 * \brief Sets the Object's network coordiantes
		 */
		Lockable* SetNetworkPos(const std::tuple<float, float, float>& pos);
		/**
		 * \brief Sets the Object's angles
		 */
		Lockable* SetAngle(const std::tuple<float, float, float>& angle);
		/**
		 * \brief Sets the Object's game cell
		 */
		Lockable* SetGameCell(unsigned int cell);
		/**
		 * \brief Sets the Object's network cell
		 */
#ifdef VAULTSERVER
		virtual Lockable* SetNetworkCell(unsigned int cell);
#else
		Lockable* SetNetworkCell(unsigned int cell);
#endif
		/**
		 * \brief Sets the Object's enabled state
		 */
		Lockable* SetEnabled(bool state);
		/**
		 * \brief Sets the Object's lock level
		 */
		Lockable* SetLockLevel(unsigned int lock);
		/**
		 * \brief Sets the Object's owner
		 */
		Lockable* SetOwner(unsigned int owner);

		/**
		 * \brief Returns a vector representation of the game coordinates
		 */
		VaultVector vvec() const;
		/**
		 * \brief Returns true if the Object is in a given range
		 */
		bool IsNearPoint(float X, float Y, float Z, float R) const;
		/**
		 * \brief Returns true if the Object's coordinate sepcified by axis (axis value hex code) is in a given range
		 */
		bool IsCoordinateInRange(unsigned char axis, float value, float R) const;
		/**
		 * \brief Returns distant coordinates of the object using the Z-angle
		 */
		std::pair<float, float> GetOffset(float N) const;
		/**
		 * \brief Returns true if the Object's network coordinates are valid
		 */
		bool HasValidCoordinates() const;

#ifdef VAULTSERVER
		/**
		 * \brief Sets the Object's base ID
		 */
		virtual Lockable* SetBase(unsigned int baseID);
#endif

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

#ifndef VAULTSERVER
class ObjectFunctor : public ReferenceFunctor
{
	public:
		ObjectFunctor(unsigned int flags, RakNet::NetworkID id) : ReferenceFunctor(flags, id) {}
		virtual ~ObjectFunctor() {}

		virtual std::vector<std::string> operator()();
		virtual bool filter(FactoryWrapper<Reference>& reference);
};
#endif

GF_TYPE_WRAPPER(Object, Reference, ID_OBJECT, ALL_OBJECTS)

PF_MAKE(ID_OBJECT_NEW, pGeneratorReferenceExtend, std::string, std::tuple<float, float, float>, std::tuple<float, float, float>, unsigned int, bool, unsigned int, unsigned int)
template<>
inline const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type* PacketFactory::Cast_<pTypes::ID_OBJECT_NEW>::Cast(const pPacket* packet) {
	pTypes type = packet->type();
	return (
		type == pTypes::ID_OBJECT_NEW ||
		type == pTypes::ID_ITEM_NEW ||
		type == pTypes::ID_CONTAINER_NEW ||
		type == pTypes::ID_ACTOR_NEW ||
		type == pTypes::ID_PLAYER_NEW
	) ? static_cast<const typename pTypesMap<pTypes::ID_OBJECT_NEW>::type*>(packet) : nullptr;
}
PF_MAKE(ID_VOLATILE_NEW, pGeneratorReference, unsigned int, float, float, float)
PF_MAKE(ID_OBJECT_REMOVE, pGeneratorReference, bool)
PF_MAKE(ID_UPDATE_NAME, pGeneratorReference, std::string)
PF_MAKE(ID_UPDATE_POS, pGeneratorReference, float, float, float)
PF_MAKE(ID_UPDATE_ANGLE, pGeneratorReference, float, float)
PF_MAKE(ID_UPDATE_CELL, pGeneratorReference, unsigned int, float, float, float)
PF_MAKE(ID_UPDATE_LOCK, pGeneratorReference, unsigned int)
PF_MAKE(ID_UPDATE_OWNER, pGeneratorReference, unsigned int)
PF_MAKE(ID_UPDATE_ACTIVATE, pGeneratorReference, RakNet::NetworkID)
PF_MAKE(ID_UPDATE_SOUND, pGeneratorReference, unsigned int)

#endif
