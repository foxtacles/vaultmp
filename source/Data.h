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

		public:
			virtual ~_Parameter() {}

			virtual const vector<string>& get() = 0;
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

		public:
			RawParameter(string str) : data(vector<string>{str}) {}
			RawParameter(vector<string> str) : data(str) {}
			RawParameter(const vector<unsigned char>& str) : data(make(str)) {}
			RawParameter(unsigned int str) : data(make(str)) {}
			RawParameter(double str) : data(make(str)) {}

			virtual const vector<string>& get() { return data; }

			virtual ~RawParameter() {}
	};

	class FuncParameter : public _Parameter {

		private:
			vector<string> data;
			shared_ptr<VaultFunctor> func;

		public:
			FuncParameter(shared_ptr<VaultFunctor> func) : func(func) {}

			virtual const vector<string>& get() { data = (*func.get())(); return data; }

			virtual ~FuncParameter() {}
	};

	class Parameter {

		private:
			shared_ptr<_Parameter> param;
			const _Parameter* const_param = NULL;

		public:
			Parameter(RawParameter& param) : param(new RawParameter(param)) {}
			Parameter(FuncParameter& param) : param(new FuncParameter(param)) {}

			Parameter(const RawParameter& param) : const_param(&param) {}
			Parameter(const FuncParameter& param) : const_param(&param) {}
			~Parameter() = default;

	};

	typedef list<Parameter> ParamContainer;
	typedef const map<const unsigned int, const char*> Database;
	typedef map<const unsigned char, const unsigned char> IndexLookup;
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

	static const RawParameter Param_True(1u);
	static const RawParameter Param_False(0u);

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
