#pragma once

struct Pixel
{
	float Alpha, L, A, B;
	Pixel();
};

void SaveBuffer(const char * path, size_t width, size_t height, const Pixel * buffer);