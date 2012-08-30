#include "CriticalSection.h"
#include "VaultException.h"

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

void CriticalSection::SetDebugHandler(Debug* debug)
{
	this->debug = debug;

	if (debug)
		debug->PrintFormat("Attached debug handler to CriticalSection object %08X (%s)", true, this, typeid(*this).name());
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
#ifdef VAULTMP_DEBUG
		if (debug)
			debug->PrintFormat("CriticalSection object %08X (%s) locked by thread %s", true, this, typeid(*this).name(), thread_id().c_str());
#endif
		return this;
	}
	else if (finalize)
	{
		if (success)
			cs.unlock();

		return nullptr;
	}
	else
#ifdef VAULTMP_DEBUG
		throw VaultException("Thread %s could not enter CriticalSection object %08X, timeout of %dms reached (%s)", thread_id().c_str(), this, CS_TIMEOUT, typeid(*this).name());
#else
		throw VaultException("Could not enter CriticalSection object %08X, timeout of %dms reached (%s)", this, CS_TIMEOUT, typeid(*this).name());
#endif
}

void CriticalSection::EndSession()
{
	cs.unlock();

#ifdef VAULTMP_DEBUG

	if (debug)
		debug->PrintFormat("CriticalSection object %08X (%s) unlocked by thread %s", true, this, typeid(*this).name(), thread_id().c_str());

#endif
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
	finalize = true;
	EndSession();
}

#ifdef VAULTMP_DEBUG
void CriticalSection::PrintStatus()
{
	if (debug)
		debug->PrintFormat("CriticalSection object %08X (%s), status call by thread %s", true, this, typeid(*this).name(), thread_id().c_str());
}
#endif
