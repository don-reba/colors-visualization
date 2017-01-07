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

__m256 FgtVolume::operator [] (const Vector3f256 & p) const
{
	__m256 side = _mm256_set1_ps(Side);

	// get domain origin
	__m256 minX = _mm256_set1_ps(DomainMin.x());
	__m256 minY = _mm256_set1_ps(DomainMin.y());
	__m256 minZ = _mm256_set1_ps(DomainMin.z());

	// get cell coordinates
	__m256 kx = _mm256_floor_ps(_mm256_div_ps(_mm256_sub_ps(p.x, minX), side));
	__m256 ky = _mm256_floor_ps(_mm256_div_ps(_mm256_sub_ps(p.y, minY), side));
	__m256 kz = _mm256_floor_ps(_mm256_div_ps(_mm256_sub_ps(p.z, minZ), side));

	// get coefficient addresses
	__m256i indices = _mm256_cvtps_epi32
		( _mm256_mul_ps
			( _mm256_fmadd_ps
					( _mm256_fmadd_ps(kz, _mm256_set1_ps((float)Count.y()), ky)
					, _mm256_set1_ps((float)Count.x())
					, kx
					)
			, _mm256_set1_ps((float)PD)
			)
		);

	// get cell center's position
	__m256 cx = _mm256_fmadd_ps(side, _mm256_add_ps(kx, _mm256_set1_ps(0.5f)), minX);
	__m256 cy = _mm256_fmadd_ps(side, _mm256_add_ps(ky, _mm256_set1_ps(0.5f)), minY);
	__m256 cz = _mm256_fmadd_ps(side, _mm256_add_ps(kz, _mm256_set1_ps(0.5f)), minZ);

	// compute distance from cell center in sigmas
	__m256 delta[3] =
		{ _mm256_mul_ps(_mm256_sub_ps(p.x, cx), _mm256_set1_ps(1.0f / Sigma))
		, _mm256_mul_ps(_mm256_sub_ps(p.y, cy), _mm256_set1_ps(1.0f / Sigma))
		, _mm256_mul_ps(_mm256_sub_ps(p.z, cz), _mm256_set1_ps(1.0f / Sigma))
		};

	__m256 dxx = _mm256_mul_ps(delta[0], delta[0]);
	__m256 dyy = _mm256_mul_ps(delta[1], delta[1]);
	__m256 dzz = _mm256_mul_ps(delta[2], delta[2]);

	__m256 dnorm = _mm256_add_ps(_mm256_add_ps(dxx, dyy), dzz);

	// gather the polynomial
	assert(PD == 56);
	__m256 polynomial[56];
	polynomial[0] = expf8(_mm256_sub_ps(_mm256_setzero_ps(), dnorm));

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
				polynomial[t] = _mm256_mul_ps(delta[i], polynomial[j]);
		}
	}

	__declspec(align(32)) int indicesi32[8];
	_mm256_store_si256((__m256i*)indicesi32, indices);
	bool allEqual = true;
	for (size_t i = 1; i != 8; ++i)
		allEqual = allEqual && (indicesi32[0] == indicesi32[i]);

	// normalizationFactor * coefficients · polynomial
	__m256 sum = _mm256_setzero_ps();
	if (allEqual)
	{
		// fast path (all indices are the same)
		const float * cp = &Coefficients[indicesi32[0]];
		for (size_t i = 0; i != 56; ++i)
		{
			__m256 coefficients = _mm256_set1_ps(cp[i]);
			sum = _mm256_fmadd_ps(polynomial[i], coefficients, sum);
		}
	}
	else
	{
		// slow path
		for (size_t i = 0; i != 56; ++i)
		{
			__m256 coefficients = _mm256_i32gather_ps(&Coefficients[i], indices, sizeof(float));
			sum = _mm256_fmadd_ps(polynomial[i], coefficients, sum);
		}
	}
	return _mm256_mul_ps(sum, _mm256_set1_ps(normalizationFactor));
}