#ifndef OBJECT_H
#define OBJECT_H

#define TYPECLASS
#include "GameFactory.h"

#include "vaultmp.h"
#include "Data.h"
#include "API.h"
#include "Reference.h"
#include "Value.h"
#include "PacketFactory.h"
#include "VaultVector.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

const unsigned int FLAG_ENABLED         = 0x00000001;
const unsigned int FLAG_DISABLED        = FLAG_ENABLED << 1;
const unsigned int FLAG_NOTSELF         = FLAG_DISABLED << 1;
const unsigned int FLAG_SELF            = FLAG_NOTSELF << 1;
const unsigned int FLAG_REFERENCE		= FLAG_SELF << 1;
const unsigned int FLAG_BASE			= FLAG_REFERENCE << 1;

/**
 * \brief Derives from Reference class and represents an object in-game
 *
 * Data specific to Objects are a name, XYZ position and angle, a cell and various states
 */

class Object : public Reference
{
		friend class GameFactory;

	private:
		static RawParameter param_Axis;

#ifdef VAULTMP_DEBUG
		static DebugInput<Object> debug;
#endif

		Value<std::string> object_Name;
		std::unordered_map<unsigned char, Value<double>> object_Game_Pos;
		std::unordered_map<unsigned char, Value<double>> object_Network_Pos;
		std::unordered_map<unsigned char, Value<double>> object_Angle;
		Value<unsigned int> cell_Game;
		Value<unsigned int> cell_Network;
		Value<bool> state_Enabled;

		static bool IsValidCoordinate(double C);
		static bool IsValidAngle(unsigned char axis, double A);

		void initialize();

		Object(const Object&);
		Object& operator=(const Object&);

	protected:
		Object(unsigned int refID, unsigned int baseID);
		Object(const pDefault* packet);
		Object(pPacket&& packet);

	public:
		virtual ~Object();

		/**
		 * \brief Retrieves a reference to a constant Parameter containing every available axis value string representation
		 *
		 * Used to pass axis values to the Interface
		 */
		static const RawParameter& Param_Axis();

		/**
		 * \brief Retrieves the Object's name
		 */
		std::string GetName() const;
		/**
		 * \brief Retrieves the Object's game coordinate on the specified axis (axis value hex code)
		 */
		double GetGamePos(unsigned char axis) const;
		/**
		 * \brief Retrieves the Object's network coordinate on the specified axis (axis value hex code)
		 */
		double GetNetworkPos(unsigned char axis) const;
		/**
		 * \brief Retrieves the Object's angle on the specified axis (axis value hex code)
		 */
		double GetAngle(unsigned char axis) const;
		/**
		 * \brief Retrieves the Object's enabled state
		 */
		bool GetEnabled() const;
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
		 * \brief Sets the Object's name
		 */
		Lockable* SetName(const std::string& name);
		/**
		 * \brief Sets the Object's game coordiante on the specified axis (axis value hex code)
		 */
		Lockable* SetGamePos(unsigned char axis, double pos);
		/**
		 * \brief Sets the Object's network coordiante on the specified axis (axis value hex code)
		 */
		Lockable* SetNetworkPos(unsigned char axis, double pos);
		/**
		 * \brief Sets the Object's angle on the specified axis (axis value hex code)
		 */
		Lockable* SetAngle(unsigned char axis, double angle);
		/**
		 * \brief Sets the Object's enabled state
		 */
		Lockable* SetEnabled(bool state);
		/**
		 * \brief Sets the Object's game cell
		 */
		Lockable* SetGameCell(unsigned int cell);
		/**
		 * \brief Sets the Object's network cell
		 */
		Lockable* SetNetworkCell(unsigned int cell);

		/**
		 * \brief Returns a vector representation of the game coordinates
		 */
		VaultVector vvec() const;

		/**
		 * \brief Returns true if the Object is in a given range
		 */
		bool IsNearPoint(double X, double Y, double Z, double R) const;
		/**
		 * \brief Returns true if the Object's coordinate sepcified by axis (axis value hex code) is in a given range
		 */
		bool IsCoordinateInRange(unsigned char axis, double value, double R) const;
		/**
		 * \brief Returns distant coordinates of the object using the Z-angle
		 */
		std::pair<double, double> GetOffset(double N) const;
		/**
		 * \brief Returns true if the Object's network coordinates are valid
		 */
		bool HasValidCoordinates() const;

		/**
		 * \brief For network transfer
		 */
		virtual pPacket toPacket() const;
};

class ObjectFunctor : public ReferenceFunctor
{
	public:
		ObjectFunctor(unsigned int flags, RakNet::NetworkID id) : ReferenceFunctor(flags, id) {}
		virtual ~ObjectFunctor() {}

		virtual std::vector<std::string> operator()();
		virtual bool filter(FactoryObject& reference);
};

#endif
