#include "CriticalSection.hpp"
#include "VaultException.hpp"

#ifdef VAULTMP_DEBUG
#include <sstream>
#endif

using namespace std;

#ifdef VAULTMP_DEBUG
string CriticalSection::thread_id(thread& t)
{
	string id;
	stringstream s;

	s << t.get_id();
	s >> id;

	return id;
}

string CriticalSection::thread_id()
{
	string id;
	stringstream s;

	s << this_thread::get_id();
	s >> id;

	return id;
}
#endif
