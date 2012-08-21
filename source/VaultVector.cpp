#include "VaultVector.h"

VaultVector::VaultVector(double X, double Y, double Z) : X(X), Y(Y), Z(Z)
{

}

bool VaultVector::IsNearPoint(const VaultVector& vvec, double R) const
{
	return (sqrt((abs(X - vvec.X) * abs(X - vvec.X)) + (abs(Y - vvec.Y) * abs(Y - vvec.Y)) + (abs(Z - vvec.Z) * abs(Z - vvec.Z))) <= R);
}

pair<double, double> VaultVector::GetOffset(double aZ, double N) const
{
	return make_pair(X + (N * sin((aZ / 360.0) * 2 * M_PI)), Y + (N * cos((aZ / 360.0) * 2 * M_PI)));
}
