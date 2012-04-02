#include "CriticalSection.h"
#include "VaultException.h"

CriticalSection::CriticalSection()
{
	finalize = false;
	locks = 0;

#ifdef VAULTMP_DEBUG
	debug = NULL;
#endif
}

CriticalSection::~CriticalSection()
{
    if (locks)
    {
        chrono::steady_clock::time_point till = chrono::steady_clock::now() + chrono::milliseconds(CS_TIMEOUT);

        while (chrono::steady_clock::now() < till && locks)
            this_thread::sleep_for(chrono::milliseconds(100));
    }

	// Throwing this exception is bad. But if it ever ever happens, this is a serious programming error somewhere else, so no problem to fuck up
    if ( locks != 0 )
#ifdef VAULTMP_DEBUG
		throw VaultException( "Lock count of CriticalSection object %08X is not zero, but some thread (ID: %s) invoked a delete (%s)", this, thread_id().c_str(), typeid( *this ).name() );
#else
        throw VaultException( "Lock count of CriticalSection object %08X is not zero, but some thread invoked a delete (%s)", this, typeid( *this ).name() );
#endif
}

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

void CriticalSection::SetDebugHandler( Debug* debug )
{
	this->debug = debug;

	if ( debug != NULL )
		debug->PrintFormat( "Attached debug handler to CriticalSection object %08X (%s)", true, this, typeid( *this ).name());
}
#endif

CriticalSection* CriticalSection::StartSession()
{
    if (finalize)
        return NULL;

    bool success = cs.try_lock_for(chrono::milliseconds(CS_TIMEOUT));

	if (success && !finalize)
	{
		++locks;

#ifdef VAULTMP_DEBUG

		if ( debug != NULL )
			debug->PrintFormat( "CriticalSection object %08X (%s) locked by thread %s", true, this, typeid( *this ).name(), thread_id().c_str() );

#endif
		return this;
	}
	else if ( finalize )
	{
	    if (success)
            cs.unlock();

	    return NULL;
	}
	else
#ifdef VAULTMP_DEBUG
		throw VaultException( "Thread %s could not enter CriticalSection object %08X, timeout of %dms reached (%s)", thread_id().c_str(), this, CS_TIMEOUT, typeid( *this ).name() );
#else
		throw VaultException( "Could not enter CriticalSection object %08X, timeout of %dms reached (%s)", this, CS_TIMEOUT, typeid( *this ).name() );
#endif
}

void CriticalSection::EndSession()
{
	if ( locks < 1 )
#ifdef VAULTMP_DEBUG
		throw VaultException( "Lock count of CriticalSection object %08X is zero, but some thread (ID: %s) tried to leave (%s)", this, thread_id().c_str(), typeid( *this ).name() );
#else
		throw VaultException( "Lock count of CriticalSection object %08X is zero, but some thread tried to leave (%s)", this, typeid( *this ).name() );
#endif

	--locks;
    cs.unlock();

#ifdef VAULTMP_DEBUG

	if ( debug != NULL )
		debug->PrintFormat( "CriticalSection object %08X (%s) unlocked by thread %s", true, this, typeid( *this ).name(), thread_id().c_str() );

#endif
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
	finalize = true;
	EndSession();
}

