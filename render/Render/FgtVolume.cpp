#include "FgtVolume.h"

#include "Pow.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
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
	if (valueCount != Count.x() * Count.y() * Count.z() * PD * 8)
		throw runtime_error("Inconsistent value count.");

	neighbors = PrecomputeNeighbors(4);
}

float FgtVolume::operator [] (const Vector3f & p) const
{
	Vector3f dpVector = (p - ClusterOrigin(p)) / Sigma;
	int      ci       = CellIndex(p);

	__m256 dp[3] =
		{ _mm256_set1_ps(dpVector.x())
		, _mm256_set1_ps(dpVector.y())
		, _mm256_set1_ps(dpVector.z())
		};

	assert(PD == 35);
	__declspec(align(32)) __m256 polynomial[35];

	__m256 sum = _mm256_setzero_ps();

	const float * pv = neighbors.vOffsets.data();

	for (int iOffset : neighbors.iOffsets)
	{
		// compute delta as dp - vOffset (prescaled by sigma)
		__m256 deltas[3];
		deltas[0] = _mm256_sub_ps(dp[0], _mm256_load_ps(pv)); pv += 8;
		deltas[1] = _mm256_sub_ps(dp[1], _mm256_load_ps(pv)); pv += 8;
		deltas[2] = _mm256_sub_ps(dp[2], _mm256_load_ps(pv)); pv += 8;

		// initialize the polynomial to exp(-delta^2)
		__m256 dxx = _mm256_mul_ps(deltas[0], deltas[0]);
		__m256 dyy = _mm256_mul_ps(deltas[1], deltas[1]);
		__m256 dzz = _mm256_mul_ps(deltas[2], deltas[2]);

		__m256 norm = _mm256_xor_ps
			( _mm256_add_ps(_mm256_add_ps(dxx, dyy), dzz)
			, _mm256_set1_ps(-0.0f)
			);

		polynomial[0] = expf8(norm);

		// compute the terms of the polynomial
		int heads[3] = { 0, 0, 0 };
		int t = 1;
		for (int a = 1; a != Alpha; ++a)
		{
			int tail = t;
			for (int i = 0; i != 3; ++i)
			{
				int head = heads[i];
				heads[i] = t;
				for (int j = head; j != tail; ++j, ++t)
					polynomial[t] = _mm256_mul_ps(deltas[i], polynomial[j]);
			}
		}

		// add in the coefficients
		const float * pCoef = &Coefficients[ci + iOffset];
		for (const __m256 & term : polynomial)
		{
			__m256 coef = _mm256_load_ps(pCoef);
			sum = _mm256_fmadd_ps(coef, term, sum);
			pCoef += 8;
		}
	}

	// retrieve the sums
	__declspec(align(32)) float sumValues[8];
	_mm256_store_ps(sumValues, sum);

	// add up the 8 partial sums
	for (size_t i = 1; i != 7; ++i)
		sumValues[0] += sumValues[i];
	return sumValues[0];
}

FgtVolume::Neighbors FgtVolume::PrecomputeNeighbors(int r) const
{
	const int n = (int)ceil(sqrt((float)r));

	aligned_vector_f32 vOffsets;
	vector<int>        iOffsets;

	for (int z = -n; z <= n; ++z)
	for (int y = -n; y <= n; ++y)
	for (int x = -n; x <= n; ++x)
	{
		if (x * x + y * y + z * z <= r) // sphere
		{
			// dx
			for (int kz = 0; kz != 2; ++kz)
			for (int ky = 0; ky != 2; ++ky)
			for (int kx = 0; kx != 2; ++kx)
				vOffsets.push_back((2 * x + kx + 0.5f) * Side / Sigma);
			// dy
			for (int kz = 0; kz != 2; ++kz)
			for (int ky = 0; ky != 2; ++ky)
			for (int kx = 0; kx != 2; ++kx)
				vOffsets.push_back((2 * y + ky + 0.5f) * Side / Sigma);
			// dz
			for (int kz = 0; kz != 2; ++kz)
			for (int ky = 0; ky != 2; ++ky)
			for (int kx = 0; kx != 2; ++kx)
				vOffsets.push_back((2 * z + kz + 0.5f) * Side / Sigma);
			// di
			iOffsets.push_back(((z * Count.y() + y) * Count.x() + x) * PD * 8);
		}
	}

	return { vOffsets, iOffsets };
}

Vector3f FgtVolume::ClusterOrigin(const Eigen::Vector3f & p) const
{
	const float clusterSide = 2.0f * Side;
	return Vector3f
		( floor((p.x() - DomainMin.x()) / clusterSide)
		, floor((p.y() - DomainMin.y()) / clusterSide)
		, floor((p.z() - DomainMin.z()) / clusterSide)
		) * clusterSide + DomainMin;
}

int FgtVolume::CellIndex(const Eigen::Vector3f & p) const
{
	const float clusterSide = 2.0f * Side;
	Vector3i k = ((p - DomainMin) / clusterSide).cast<int>();
	return ((k.z() * Count.y() + k.y()) * Count.x() + k.x()) * PD * 8;
}