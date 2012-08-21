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
		VaultVector(double X, double Y, double Z) : X(X), Y(Y), Z(Z) {}
		~VaultVector() = default;

		bool IsNearPoint(const VaultVector& vvec, double R) const
		{
			return (sqrt((abs(X - vvec.X) * abs(X - vvec.X)) + (abs(Y - vvec.Y) * abs(Y - vvec.Y)) + (abs(Z - vvec.Z) * abs(Z - vvec.Z))) <= R);
		}

		pair<double, double> GetOffset(double aZ, double N) const
		{
			return make_pair(X + (N * sin((aZ / 360.0) * 2 * M_PI)), Y + (N * cos((aZ / 360.0) * 2 * M_PI)));
		}
};

#endif
