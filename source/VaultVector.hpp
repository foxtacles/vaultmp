#ifndef VAULTVECTOR_H
#define VAULTVECTOR_H

#include "vaultmp.hpp"

#include <utility>
#define _USE_MATH_DEFINES
#include <cmath>

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
			return (sqrt((std::abs(X - vvec.X) * std::abs(X - vvec.X)) + (std::abs(Y - vvec.Y) * std::abs(Y - vvec.Y)) + (std::abs(Z - vvec.Z) * std::abs(Z - vvec.Z))) <= R);
		}

		std::pair<double, double> GetOffset(double aZ, double N) const
		{
			return std::make_pair(X + (N * sin((aZ / 360.0) * 2 * M_PI)), Y + (N * cos((aZ / 360.0) * 2 * M_PI)));
		}
};

#endif
