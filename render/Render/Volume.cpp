#include "Volume.h"

#include <algorithm>
#include <cassert>
#include <cmath>
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

float Volume::operator [] (const Vector3f & lab) const
{
	const float fx(lFactor * (lab.x() - minL));
	const float fy(aFactor * (lab.y() - minA));
	const float fz(bFactor * (lab.z() - minB));

	const int x((int)(fx));
	const int y((int)(fy));
	const int z((int)(fz));

	if (x < 0 || x > Nx - 1) return 0.0f;
	if (y < 0 || y > Ny - 1) return 0.0f;
	if (z < 0 || z > Nz - 1) return 0.0f;

	return Values[x + Nx * y + Nx * Ny * z];
}

/*
float Volume::operator [] (const Vector3f & lab) const
{
	// trlinear interpolation

	const float fx(lFactor * (lab.x() - minL));
	const float fy(aFactor * (lab.y() - minA));
	const float fz(bFactor * (lab.z() - minB));

	const int x((int)(fx));
	const int y((int)(fy));
	const int z((int)(fz));

	if (x < 0 || x > Nx - 2) return 0.0f;
	if (y < 0 || y > Ny - 2) return 0.0f;
	if (z < 0 || z > Nz - 2) return 0.0f;

	const float dx(fx - floor(fx));
	const float dy(fy - floor(fy));
	const float dz(fz - floor(fz));

	const float v000(Values[(x + 0) + Nx * (y + 0) + Nx * Ny * (z + 0)]);
	const float v001(Values[(x + 0) + Nx * (y + 0) + Nx * Ny * (z + 1)]);
	const float v010(Values[(x + 0) + Nx * (y + 1) + Nx * Ny * (z + 0)]);
	const float v011(Values[(x + 0) + Nx * (y + 1) + Nx * Ny * (z + 1)]);
	const float v100(Values[(x + 1) + Nx * (y + 0) + Nx * Ny * (z + 0)]);
	const float v101(Values[(x + 1) + Nx * (y + 0) + Nx * Ny * (z + 1)]);
	const float v110(Values[(x + 1) + Nx * (y + 1) + Nx * Ny * (z + 0)]);
	const float v111(Values[(x + 1) + Nx * (y + 1) + Nx * Ny * (z + 1)]);

	const float v00((1.0f - dx) * v000 + dx * v001);
	const float v01((1.0f - dx) * v010 + dx * v011);
	const float v10((1.0f - dx) * v100 + dx * v101);
	const float v11((1.0f - dx) * v110 + dx * v111);

	const float v0((1.0f - dy) * v00 + dy * v01);
	const float v1((1.0f - dy) * v10 + dy * v11);

	return (1.0f - dz) * v0 + dz * v1;
}
*/

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