#pragma once

#include <cstdint>

struct Resolution
{
	using Type = std::uint32_t;
	Type w;
	Type h;
};
static const Resolution res2160p { 3840, 2160 };
static const Resolution res1080p { 1920, 1080 };
static const Resolution res720p  { 1280, 720  };
static const Resolution res576p  { 1024, 576  };
static const Resolution res360p  { 640,  360  };