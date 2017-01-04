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

}

Volume::Volume(const char * path)
{
	ifstream f(path, ios::binary);
	if (!f)
		throw runtime_error("File could not be opened.");

	// read data

	float cellSize;
	Read(f, &cellSize);

	Read(f, &nx);
	Read(f, &ny);
	Read(f, &nz);

	float minX, minY, minZ;
	Read(f, &minX);
	Read(f, &minY);
	Read(f, &minZ);

	offset = Vector3f(-minX, -minY, -minZ);
	factor = 1.0f / cellSize;

	values.resize(nx * ny * nz);
	f.read(reinterpret_cast<char*>(values.data()), values.size() * sizeof(float));
	if (f.fail())
		throw runtime_error("File could not be read");
}


float Volume::operator [] (const Vector3f & p) const
{
	const Vector3f f = factor * (p + offset);

	const int x = (int)(f.x());
	const int y = (int)(f.y());
	const int z = (int)(f.z());

	if (x < 0 || x >= nx) return 0.0f;
	if (y < 0 || y >= ny) return 0.0f;
	if (z < 0 || z >= nz) return 0.0f;

	return values[(z * ny + y) * nx + x];
}

__m256 Volume::operator [] (const Vector3f256 &lab) const
{
	__declspec(align(32)) float x[8];
	__declspec(align(32)) float y[8];
	__declspec(align(32)) float z[8];
	_mm256_store_ps(x, lab.x);
	_mm256_store_ps(y, lab.y);
	_mm256_store_ps(z, lab.z);

	__declspec(align(32)) float result[8];
	for (size_t i = 0; i != 8; ++i)
		result[i] = (*this)[Vector3f(x[i], y[i], z[i])];

	return _mm256_load_ps(result);
}