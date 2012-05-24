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

	protected:
		void _next(vector<string>& result);

	public:
		VaultFunctor() : next(NULL) {}
		virtual ~VaultFunctor();

		/**
		 * \brief Connects this VaultFunctor with another one
		 *
		 * This is to build a chain of functors
		 * Returns the connected VaultFunctor on success
		 */
		VaultFunctor* connect(VaultFunctor* next);
		/**
		 * \brief The functor call
		 */
		virtual vector<string> operator()() = 0;

};

#endif
