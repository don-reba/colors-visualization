#include "FgtVolume.h"

#include "Pow.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <stdexcept>

using namespace Eigen;
using namespace std;

namespace
{
	template <typename T>
	void Read(ifstream & stream, T & data)
	{
		stream.read(reinterpret_cast<char*>(&data), sizeof(T));
	}
}

FgtVolume::FgtVolume(const char * path)
{
	ifstream f(path, ios::binary);
	if (!f)
		throw runtime_error("File could not be opened.");

	Read(f, Sigma);
	Read(f, Alpha);
	Read(f, PD);
	Read(f, Side);

	Read(f, Count.x());
	Read(f, Count.y());
	Read(f, Count.z());

	Read(f, DomainMin.x());
	Read(f, DomainMin.y());
	Read(f, DomainMin.z());

	int valueCount;
	Read(f, valueCount);

	Coefficients.resize(valueCount);
	for (int i = 0; i != valueCount; ++i)
		Read(f, Coefficients[i]);
	if (valueCount != Count.x() * Count.y() * Count.z() * PD)
		throw runtime_error("Inconsistent value count.");

	normalizationFactor = pow(Sigma * sqrt((float)M_PI), -3.0f);
}

float FgtVolume::operator [] (const Vector3f & p) const
{
	// get cell coordinates
	Vector3i k = ((p - DomainMin) / Side).cast<int>();

	// compute distance from cell center
	Vector3f c     = DomainMin + Side * (k.cast<float>() + Vector3f(0.5f, 0.5f, 0.5f));
	Vector3f delta = (p - c) / Sigma;

	// gather the polynomial
	assert(PD == 56);
	float polynomial[56];
	polynomial[0] = exp(-delta.squaredNorm());

	Vector3i heads(0, 0, 0);
	int t = 1;
	for (int a = 1; a != Alpha; ++a)
	{
		int tail = t;
		for (int i = 0; i != 3; ++i)
		{
			int head = heads[i];
			heads[i] = t;
			for (int j = head; j != tail; ++j, ++t)
				polynomial[t] = delta[i] * polynomial[j];
		}
	}

	// normalizationFactor * coefficients · polynomial
	float sum = 0.0f;
	const float * value = &Coefficients[PD * (k.x() + Count.x() * (k.y() + Count.y() * k.z()))];
	for (float prod : polynomial)
		sum += *(value++) * prod;
	return normalizationFactor * sum;
}