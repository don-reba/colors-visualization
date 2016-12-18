#pragma once

struct Resolution
{
	size_t w;
	size_t h;
};
static const Resolution res4k    { 3840, 2160 };
static const Resolution res1080p { 1920, 1080 };
static const Resolution res720p  { 1280, 720  };
static const Resolution res576p  { 1024, 576  };
static const Resolution res360p  { 640,  360  };