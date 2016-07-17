#pragma once

struct Resolution
{
	size_t w, h;
	Resolution(size_t width, size_t height) : w(width), h(height) {}
};
const Resolution res1080p (1920, 1080);
const Resolution res720p  (1280, 720);
