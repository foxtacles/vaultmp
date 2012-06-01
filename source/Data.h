#ifndef DATA_H
#define DATA_H

#include <string>
#include <list>
#include <future>
#include <vector>
#include <map>

#include "RakNet/RakPeerInterface.h"
#include "RakNet/RakString.h"
#include "RakNet/MessageIdentifiers.h"

#include "Utils.h"
#include "VaultException.h"
#include "VaultFunctor.h"

static const unsigned int PLAYER_REFERENCE  = 0x00000014;
static const unsigned int PLAYER_BASE       = 0x00000007;

using namespace std;
using namespace RakNet;

/* Shared data structures and tables */

namespace Data
{
	typedef void (*ResultHandler)(unsigned int, vector<double>&, double, bool);

	class _Parameter {

		protected:
			_Parameter() = default;
			_Parameter(const _Parameter&) = default;
			_Parameter(_Parameter&&) = default;
			_Parameter& operator= (_Parameter&&) = default;

		public:
			virtual ~_Parameter() {}

			virtual const vector<string>& get() const = 0;
			virtual void reset() const = 0;
	};

	class RawParameter : public _Parameter {

		private:
			vector<string> data;

			static vector<string> make(const vector<unsigned char>& str)
			{
				vector<string> convert;

				for (unsigned char param : str)
					convert.push_back(Utils::toString(param));

				return convert;
			}

			static vector<string> make(unsigned int str)
			{
				return vector<string>{Utils::toString(str)};
			}

			static vector<string> make(double str)
			{
				return vector<string>{Utils::toString(str)};
			}

			static vector<string> make(bool str)
			{
				return vector<string>{str ? "1" : "0"};
			}

		public:
			RawParameter(string str) : data(vector<string>{str}) {}
			RawParameter(const vector<string>& str) : data(str) {}
			RawParameter(const vector<unsigned char>& str) : data(make(str)) {}
			RawParameter(unsigned int str) : data(make(str)) {}
			RawParameter(double str) : data(make(str)) {}
			RawParameter(bool str) : data(make(str)) {}
			RawParameter(const RawParameter&) = default;
			RawParameter(RawParameter&&) = default;
			RawParameter& operator= (RawParameter&&) = default;
			virtual ~RawParameter() {}

			virtual const vector<string>& get() const { return data; }
			virtual void reset() const {}
	};

	class FuncParameter : public _Parameter {

		private:
			unique_ptr<VaultFunctor> func;
			mutable vector<string> data;
			mutable bool initialized;

		public:
			FuncParameter(unique_ptr<VaultFunctor>&& func) : func(move(func)), initialized(false) {}
			FuncParameter(FuncParameter&&) = default;
			FuncParameter& operator= (FuncParameter&&) = default;
			virtual ~FuncParameter() {}

			virtual const vector<string>& get() const
			{
				if (!initialized)
				{
					data = (*func.get())();
					initialized = true;
				}

				return data;
			}

			virtual void reset() const { initialized = false; }
	};

	class Parameter {

		private:
			unique_ptr<const _Parameter, void(*)(const _Parameter*)> param;

			static void no_delete(const _Parameter* param) {}
			static void def_delete(const _Parameter* param) { delete param; }

		public:
			Parameter(RawParameter& param) : param(new RawParameter(param), def_delete) {}
			Parameter(RawParameter&& param) : param(new RawParameter(move(param)), def_delete) {}
			Parameter(FuncParameter&& param) : param(new FuncParameter(move(param)), def_delete) {}
			Parameter(const RawParameter& param) : param(&param, no_delete) {}
			Parameter(const FuncParameter& param) : param(&param, no_delete) {}

			Parameter(Parameter&&) = default;
			Parameter& operator= (Parameter&&) = default;

			// hack: initializer lists reference static memory... this ctor enables moving the unique_ptr
			// NOT A COPY CTOR
			Parameter(const Parameter& param) : Parameter(move(const_cast<Parameter&>(param))) {}

			~Parameter() = default;

			const vector<string>& get() const { return param->get(); }
			void reset() { param->reset(); }
	};

	typedef vector<Parameter> ParamContainer;
	typedef const map<const unsigned int, const char*> LegacyDatabase;
	typedef pair<future<void>, chrono::milliseconds> AsyncPack;

	template <typename R, typename T>
	inline static R storeIn(T t)
	{
		R r;
		*reinterpret_cast<T*>(&r) = t;
		return r;
	}
	template <typename R, typename T>
	inline static T getFrom(R r)
	{
		return *reinterpret_cast<T*>(&r);
	}

	enum
	{
		ID_MASTER_QUERY = ID_USER_PACKET_ENUM,
		ID_MASTER_ANNOUNCE,
		ID_MASTER_UPDATE,
		ID_GAME_FIRST,
	};

	enum
	{
		CHANNEL_SYSTEM,
		CHANNEL_GAME,
		CHANNEL_CHAT,
	};
}

#endif
