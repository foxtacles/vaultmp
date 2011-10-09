#include "CriticalSection.h"
#include "VaultException.h"

#ifdef VAULTMP_DEBUG
Debug* CriticalSection::debug;
#endif

CriticalSection::CriticalSection()
{
	finalize = false;
	locks = 0;
#ifdef __WIN32__
	InitializeCriticalSection( &cs );
#else
	pthread_mutexattr_settype( &mta, PTHREAD_MUTEX_RECURSIVE );
	pthread_mutex_init( &cs, &mta );
#endif
#ifdef VAULTMP_DEBUG
	ndebug = true;
#endif
}

CriticalSection::~CriticalSection()
{
	// Throwing this exception is bad. But if it ever ever happens, this is a serious programming error somewhere else, so no problem to fuck up
	if ( locks != 0 )
		throw VaultException( "Lock count of CriticalSection object %08X is not zero, but some thread invoked a delete (%s)", this, typeid( *this ).name() );

#ifdef __WIN32__
	DeleteCriticalSection( &cs );
#else
	pthread_mutex_destroy( &cs );
#endif
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
#ifdef __WIN32__
	bool result = true; // on success
#else
	bool result = false; // on success
#endif
	int success = 0;
#ifdef __WIN32__

	for ( int i = 0;
#ifndef NO_CS_TIMEOUT
			( i < CS_TIMEOUT ) &&
#endif
			( !finalize ) && ( ( ( bool ) ( success = TryEnterCriticalSection( &cs ) ) ) != result );
#ifndef NO_CS_TIMEOUT
			i++
#endif
		)
		Sleep( 1 );

#else

	for ( int i = 0;
#ifndef NO_CS_TIMEOUT
			( i < CS_TIMEOUT ) &&
#endif
			( !finalize ) && ( ( ( bool ) ( success = pthread_mutex_trylock( &cs ) ) ) != result );
#ifndef NO_CS_TIMEOUT
			i++
#endif
		)
		sleep( 1 );

#endif

	if ( ( ( ( bool ) success ) == result ) && !finalize )
	{
		locks++;
#ifdef VAULTMP_DEBUG

		if ( !ndebug && debug != NULL )
			debug->PrintFormat( "CriticalSection object %08X (%s) locked", true, this, typeid( *this ).name() );

#endif
		return this;
	}

	else if ( finalize )
		return NULL;

	else
		throw VaultException( "Could not enter CriticalSection object %08X, timeout of %dms reached (%s)", this, CS_TIMEOUT, typeid( *this ).name() );
}

void CriticalSection::EndSession()
{
	if ( locks < 1 )
		throw VaultException( "Lock count of CriticalSection object %08X is zero, but some thread tried to leave (%s)", this, typeid( *this ).name() );

	locks--;
#ifdef __WIN32__
	LeaveCriticalSection( &cs );
#else
	pthread_mutex_unlock( &cs );
#endif

#ifdef VAULTMP_DEBUG

	if ( !ndebug && debug != NULL )
		debug->PrintFormat( "CriticalSection object %08X (%s) unlocked", true, this, typeid( *this ).name() );

#endif
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
	finalize = true;
#ifdef __WIN32__
	Sleep( 100 ); // give enough time to have other threads trying to enter notice that this will be destroyed
#else
	sleep( 100 );
#endif
	EndSession();
}

