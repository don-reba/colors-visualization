#include "Volume.h"

#include <fstream>
#include <stdexcept>

using namespace std;

Volume::Volume(size_t nx, size_t ny, size_t nz)
	: Nx(nx), Ny(ny), Nz(nz), Values(nullptr)
{
	Values = new int32_t[nx * ny * nz];
}

Volume::Volume(Volume && other)
	: Nx(other.Nx)
	, Ny(other.Ny)
	, Nz(other.Nz)
	, Values(other.Values)
{
	other.Values = nullptr;
}

Volume::~Volume()
{
	if (Values)
		delete [] Values;
}

template <typename T>
void Read(ifstream & stream, T * data)
{
	stream.read(reinterpret_cast<char*>(data), sizeof(T));
}

Volume LoadVolume(const char * path)
{
	ifstream f(path);
	if (!f)
		throw runtime_error("File could not be opened.");

	int32_t nx, ny, nz;
	Read(f, &nx);
	Read(f, &ny);
	Read(f, &nz);

	Volume v(nx, ny, nz);
	f.read(reinterpret_cast<char*>(v.Values), nx * ny * nz * sizeof(int32_t));
	return v;
}