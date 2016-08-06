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
	const float minY(-87.0f ), maxY(99.0f );
	const float minZ(-108.0f), maxZ(95.0f );
	//--------------------------------------
}

Volume::Volume(const char * path, Postprocess method)
{
	ifstream f(path, ios::binary);
	if (!f)
		throw runtime_error("File could not be opened.");

	// read data
	Read(f, &Nx);
	Read(f, &Ny);
	Read(f, &Nz);

	xFactor = static_cast<float>(Nx) / (maxX - minX);
	yFactor = static_cast<float>(Ny) / (maxY - minY);
	zFactor = static_cast<float>(Nz) / (maxZ - minZ);

	const size_t n(Nx * Ny * Nz);
	vector<double> values(n);
	f.read(reinterpret_cast<char*>(values.data()), n * sizeof(double));
	if (f.fail())
		throw runtime_error("File could not be read");

	// compute alpha volume
	Values.reserve(n);
	switch (method)
	{
	case PostprocessNone:
		for (double x : values)
			Values.push_back(static_cast<float>(x));
		break;
	case PostprocessLog:
		for (double x : values)
			Values.push_back(log(static_cast<float>(x) + 1.0f));
		break;
	}
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

	return Values[(z * Ny + y) * Nx + x];
}