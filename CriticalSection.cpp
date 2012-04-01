#include "CriticalSection.h"
#include "VaultException.h"

#ifdef VAULTMP_DEBUG
Debug* CriticalSection::debug;
#endif

CriticalSection::CriticalSection()
{
	finalize = false;
	locks = 0;

#ifdef VAULTMP_DEBUG
	ndebug = true;
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
		throw VaultException( "Lock count of CriticalSection object %08X is not zero, but some thread invoked a delete (%s)", this, typeid( *this ).name() );
}

#ifdef VAULTMP_DEBUG
void CriticalSection::ToggleSectionDebug( bool toggle )
{
	ndebug = !toggle;
}

void CriticalSection::SetDebugHandler( Debug* debug )
{
	CriticalSection::debug = debug;

	if ( debug != NULL )
		debug->Print( "Attached debug handler to CriticalSection class", true );
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

		if ( !ndebug && debug != NULL )
			debug->PrintFormat( "CriticalSection object %08X (%s) locked", true, this, typeid( *this ).name() );

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
		throw VaultException( "Could not enter CriticalSection object %08X, timeout of %dms reached (%s)", this, CS_TIMEOUT, typeid( *this ).name() );
}

void CriticalSection::EndSession()
{
	if ( locks < 1 )
		throw VaultException( "Lock count of CriticalSection object %08X is zero, but some thread tried to leave (%s)", this, typeid( *this ).name() );

    cs.unlock();
	--locks;

#ifdef VAULTMP_DEBUG

	if ( !ndebug && debug != NULL )
		debug->PrintFormat( "CriticalSection object %08X (%s) unlocked", true, this, typeid( *this ).name() );

#endif
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
	finalize = true;
	EndSession();
}

