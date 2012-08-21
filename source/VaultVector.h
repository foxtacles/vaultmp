#ifndef VAULTVECTOR_H
#define VAULTVECTOR_H

#include "vaultmp.h"

#define _USE_MATH_DEFINES
#include <cmath>

#include <utility>

using namespace std;

/**
 * \brief The vector class of vaultmp
 */

class VaultVector
{
	private:
		double X, Y, Z;

	public:
		VaultVector(double X, double Y, double Z);
		~VaultVector() = default;

		bool IsNearPoint(const VaultVector& vvec, double R) const;

		pair<double, double> GetOffset(double aZ, double N) const;
};

#endif
