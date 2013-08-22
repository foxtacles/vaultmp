#ifndef VAULTFUNCTOR_H
#define VAULTFUNCTOR_H

#include "vaultmp.hpp"

#include <string>
#include <vector>

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
		void _next(std::vector<std::string>& result);

	public:
		VaultFunctor() : next(nullptr) {}
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
		virtual std::vector<std::string> operator()() = 0;

};

#endif
