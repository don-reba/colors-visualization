#pragma once
#include <cstdint>

struct Volume
{
	size_t Nx;
	size_t Ny;
	size_t Nz;
	std::int32_t * Values;

	Volume(size_t nx, size_t ny, size_t nz);
	Volume(Volume && other);
	~Volume();
};

Volume LoadVolume(const char * path);