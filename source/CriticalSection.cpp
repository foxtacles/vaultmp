#include "CriticalSection.h"
#include "VaultException.h"

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

CriticalSection* CriticalSection::StartSession()
{
	if (finalize)
		return nullptr;

	bool success = true;
	cs.lock();

	if (success && !finalize)
	{
		return this;
	}
	else if (finalize)
	{
		if (success)
			cs.unlock();

		return nullptr;
	}
	else
		throw VaultException("Could not enter CriticalSection object %08X, timeout of %dms reached (%s)", this, CS_TIMEOUT, typeid(*this).name()).stacktrace();
}

void CriticalSection::EndSession()
{
	cs.unlock();
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
	finalize = true;
	EndSession();
}
