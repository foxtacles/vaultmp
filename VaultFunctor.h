#ifndef VAULTFUNCTOR_H
#define VAULTFUNCTOR_H

#include <string>
#include <vector>
#include <algorithm>
#include "vaultmp.h"

using namespace std;

/**
 * \brief The functor class of vaultmp
 *
 * Mainly used in Parameters to provide an arbitrary result of string values to the Interface
 */

class VaultFunctor
{
	private:
		VaultFunctor* next;
		vector<string>( *func )();

	protected:
		unsigned int flags;
		void _next( vector<string>& result );
		VaultFunctor( unsigned int flags );

	public:
		VaultFunctor();
		VaultFunctor( vector<string>( *func )() );
		virtual ~VaultFunctor();

		/**
		 * \brief Connects this VaultFunctor with another one
		 *
		 * This is to build a chain of functors
		 * Returns the connected VaultFunctor on success
		 */
		VaultFunctor* connect( VaultFunctor* next );
		/**
		 * \brief The functor call
		 */
		virtual vector<string> operator()();

};

#endif
