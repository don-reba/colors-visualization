#include "Volume.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <fstream>
#include <stdexcept>
#include <vector>

using namespace Eigen;
using namespace std;

//--------------------------------------
// lab ranges
//--------------------------------------
const float minL( 0.0f  ), maxL(100.0f);
const float minA(-86.0f ), maxA(99.0f );
const float minB(-108.0f), maxB(95.0f );
//--------------------------------------

Volume::Volume(size_t nx, size_t ny, size_t nz)
	: Nx(nx), Ny(ny), Nz(nz)
	, lFactor(static_cast<float>(nx) / (maxL - minL))
	, aFactor(static_cast<float>(ny) / (maxA - minA))
	, bFactor(static_cast<float>(nz) / (maxB - minB))
{
}

Volume::Volume(Volume && other)
	: Nx(other.Nx)
	, Ny(other.Ny)
	, Nz(other.Nz)
	, lFactor(other.lFactor)
	, aFactor(other.aFactor)
	, bFactor(other.bFactor)
	, Values(other.Values)
{
	swap(Values, other.Values);
}

template <typename T>
void Read(ifstream & stream, T * data)
{
	stream.read(reinterpret_cast<char*>(data), sizeof(T));
}

inline int FastFloatToInt(float f)
{
	int n;
	__asm
	{
		fld    f
		fisttp n
	}
	return n;
}

float Volume::operator [] (const Vector3f & lab) const
{
	const int x = FastFloatToInt(lFactor * (lab.x() - minL));
	const int y = FastFloatToInt(aFactor * (lab.y() - minA));
	const int z = FastFloatToInt(bFactor * (lab.z() - minB));

	return Values[x + Nx * y + Nx * Ny * z];
}

Volume LoadVolume(const char * path)
{
	ifstream f(path, ios::binary);
	if (!f)
		throw runtime_error("File could not be opened.");

	// read data
	int32_t nx, ny, nz;
	Read(f, &nx);
	Read(f, &ny);
	Read(f, &nz);

	const size_t n(nx * ny * nz);
	vector<int32_t> counts(n);
	f.read(reinterpret_cast<char*>(counts.data()), n * sizeof(int32_t));
	if (f.fail())
		throw runtime_error("File could not be read");

	// compute alpha volume
	Volume v(nx, ny, nz);
	v.Values.reserve(n);
	for (size_t i(0); i != n; ++i)
		//v.Values.push_back(counts[i] ? log(static_cast<float>(counts[i])) : 0.0f);
		v.Values.push_back(static_cast<float>(counts[i]));

	const float max    (*max_element(v.Values.begin(), v.Values.end()));
	const float factor (max == 0.0f ? 0.0f : (1.0f / static_cast<float>(max)));
	for (size_t i(0); i != n; ++i)
		v.Values[i] *= factor;

	return v;
}