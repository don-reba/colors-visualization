#include "FgtVolume.h"

#include <algorithm>
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

FgtVolume::FgtVolume(const char * path, int n)
	: N(n)
{
	ifstream f(path, ios::binary);
	if (!f)
		throw runtime_error("File could not be opened.");

	Read(f, Sigma);
	Read(f, Alpha);
	Read(f, PD);
	Read(f, Side);

	Read(f, DomainMin.x());
	Read(f, DomainMin.y());
	Read(f, DomainMin.z());
	Read(f, DomainMax.x());
	Read(f, DomainMax.y());
	Read(f, DomainMax.z());

	int valueCount;
	Read(f, valueCount);

	Values.resize(valueCount);
	for (int i = 0; i != valueCount; ++i)
		Read(f, Values[i]);

	// count = ceil((max - min) / side)
	Count.x() = (int)ceil((DomainMax.x() - DomainMin.x()) / Side);
	Count.y() = (int)ceil((DomainMax.y() - DomainMin.y()) / Side);
	Count.z() = (int)ceil((DomainMax.z() - DomainMin.z()) / Side);
	if (valueCount != Count.x() * Count.y() * Count.z() * PD)
		throw runtime_error("Inconsistent value count.");
}

float FgtVolume::operator [] (const Vector3f & p) const
{
	Vector3i kc = ((p - DomainMin) / Side).cast<int>();

	int kxmin = max(0, kc.x() - N), kxmax = min(kc.x() + N, Count.x() - 1);
	int kymin = max(0, kc.y() - N), kymax = min(kc.y() + N, Count.y() - 1);
	int kzmin = max(0, kc.z() - N), kzmax = min(kc.z() + N, Count.z() - 1);

	vector<float> prods(PD);

	float sum = 0.0f;

	for (int kz = kzmin; kz <= kzmax; ++kz)
	for (int ky = kymin; ky <= kymax; ++ky)
	for (int kx = kxmin; kx <= kxmax; ++kx)
	{
		Vector3f k((float)kx, (float)ky, (float)kz);

		Vector3f c = DomainMin + Side * (k + Vector3f(0.5f, 0.5f, 0.5f));

		Vector3f delta = (p - c) / Sigma;

		prods[0] = exp(-delta.squaredNorm());

		// gather the epolynomials
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
					prods[t] = delta[i] * prods[j];
			}
		}

		// add in the coefficients
		const float * value = &Values[PD * (kz * Count.y() * Count.x() + ky * Count.x() + kx)];
		for (float prod : prods)
			sum += *(value++) * prod;
	}

	return sum;
}
