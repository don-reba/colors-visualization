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

__m256 FgtVolume::operator [] (const Vector3f256 & lab) const
{
	__m256 side = _mm256_set1_ps(Side);

	// get domain-relative coordinates
	__m256 px = _mm256_add_ps(lab.x, _mm256_set1_ps(-DomainMin.x()));
	__m256 py = _mm256_add_ps(lab.y, _mm256_set1_ps(-DomainMin.y()));
	__m256 pz = _mm256_add_ps(lab.z, _mm256_set1_ps(-DomainMin.z()));

	// get cell coordinates
	__m256 kx = _mm256_floor_ps(_mm256_div_ps(px, side));
	__m256 ky = _mm256_floor_ps(_mm256_div_ps(py, side));
	__m256 kz = _mm256_floor_ps(_mm256_div_ps(pz, side));

	// get cell center's position
	__m256 cx = _mm256_mul_ps(side, _mm256_add_ps(kx, _mm256_set1_ps(0.5f)));
	__m256 cy = _mm256_mul_ps(side, _mm256_add_ps(ky, _mm256_set1_ps(0.5f)));
	__m256 cz = _mm256_mul_ps(side, _mm256_add_ps(kz, _mm256_set1_ps(0.5f)));

	// compute distance from cell center in sigmas
	__m256 delta[3] =
		{ _mm256_mul_ps(_mm256_sub_ps(px, cx), _mm256_set1_ps(1.0f / Sigma))
		, _mm256_mul_ps(_mm256_sub_ps(py, cy), _mm256_set1_ps(1.0f / Sigma))
		, _mm256_mul_ps(_mm256_sub_ps(pz, cz), _mm256_set1_ps(1.0f / Sigma))
		};

	__m256 dxx = _mm256_mul_ps(delta[0], delta[0]);
	__m256 dyy = _mm256_mul_ps(delta[1], delta[1]);
	__m256 dzz = _mm256_mul_ps(delta[2], delta[2]);

	__m256 dnorm = _mm256_add_ps(_mm256_add_ps(dxx, dyy), dzz);

	// gather the polynomial
	assert(PD == 56);
	__m256 polynomial[56];
	polynomial[0] = exp2f8(_mm256_sub_ps(_mm256_setzero_ps(), dnorm));

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
	__declspec(align(32)) int indexValues[8];
	_mm256_store_si256((__m256i*)indexValues, indices);

	// load the coefficients
	__declspec(align(32)) float coefficients[56 * 8];
	for (size_t i = 0; i != 56; ++i)
		for (size_t j = 0; j != 8; ++j)
			coefficients[i * 8 + j] = Coefficients[indexValues[j] + i];

	// normalizationFactor * coefficients · polynomial
	__m256 sum = _mm256_setzero_ps();
	for (size_t i = 0; i != 56; ++i)
		sum = _mm256_fmadd_ps(polynomial[i], _mm256_load_ps(coefficients + i * 8), sum);
	return _mm256_mul_ps(sum, _mm256_set1_ps(normalizationFactor));
}