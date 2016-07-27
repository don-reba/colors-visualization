#include "Volume.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <vector>

using namespace Eigen;
using namespace std;


namespace
{
	template <typename T>
	void Read(ifstream & stream, T * data)
	{
		stream.read(reinterpret_cast<char*>(data), sizeof(T));
	}

	//--------------------------------------
	// lab ranges
	//--------------------------------------
	const float minX( 0.0f  ), maxX(100.0f);
	const float minY(-86.0f ), maxY(99.0f );
	const float minZ(-108.0f), maxZ(95.0f );
	//--------------------------------------
}

Volume::Volume(size_t nx, size_t ny, size_t nz)
	: Nx(nx), Ny(ny), Nz(nz)
	, xFactor(static_cast<float>(nx) / (maxX - minX))
	, yFactor(static_cast<float>(ny) / (maxY - minY))
	, zFactor(static_cast<float>(nz) / (maxZ - minZ))
{
}

Volume::Volume(Volume && other)
	: Nx(other.Nx)
	, Ny(other.Ny)
	, Nz(other.Nz)
	, xFactor(other.xFactor)
	, yFactor(other.yFactor)
	, zFactor(other.zFactor)
	, Values(other.Values)
{
	swap(Values, other.Values);
}


float Volume::operator [] (const Vector3f & p) const
{
	const float fx(xFactor * (p.x() - minX));
	const float fy(yFactor * (p.y() - minY));
	const float fz(zFactor * (p.z() - minZ));

	const int x((int)(fx));
	const int y((int)(fy));
	const int z((int)(fz));

	if (x < 0 || x >= Nx) return 0.0f;
	if (y < 0 || y >= Ny) return 0.0f;
	if (z < 0 || z >= Nz) return 0.0f;

	return Values[x + Nx * y + Nx * Ny * z];
}

Volume Volume::MakeTest()
{
	float stepSize(30.0f);

	size_t nx(static_cast<size_t>((maxX - minX + 0.5f) / stepSize));
	size_t ny(static_cast<size_t>((maxY - minY + 0.5f) / stepSize));
	size_t nz(static_cast<size_t>((maxZ - minZ + 0.5f) / stepSize));

	Volume v(nx, ny, nz);
	v.Values.resize(nx * ny * nz);
	for (size_t x = 0; x != nx; ++x)
	for (size_t y = 0; y != ny; ++y)
	for (size_t z = 0; z != nz; ++z)
		if ((x + y + z) % 2 == 0)
		v.Values[x + nx * y + nx * ny * z] = 0.5;
	return v;
}

void PrintStats(const Volume & volume, const char * path)
{
	ofstream f(path);
	for (float x : volume.Values)
		f << x << '\n';
	f.flush();
}

Volume Volume::Load(const char * path, Volume::Postprocess method)
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
	vector<double> values(n);
	f.read(reinterpret_cast<char*>(values.data()), n * sizeof(double));
	if (f.fail())
		throw runtime_error("File could not be read");

	// compute alpha volume
	Volume v(nx, ny, nz);
	v.Values.reserve(n);
	switch (method)
	{
	case PostprocessNone:
		for (size_t i(0); i != n; ++i)
			v.Values.push_back(static_cast<float>(values[i]));
		break;
	case PostprocessLog:
		for (size_t i(0); i != n; ++i)
			v.Values.push_back(log(static_cast<float>(values[i]) + 1.0f));
		break;
	}

	//PrintStats(v, "D:\\Programming\\Colours visualization\\analysis\\volume values.txt");

	return v;
}