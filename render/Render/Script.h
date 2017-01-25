#pragma once

#include "Antialias.h"
#include "Resolution.h"

#include <boost\variant.hpp>

#include <string>
#include <tuple>

struct FramesAll {};
using  FrameRange = std::tuple<size_t, size_t>;
using  FrameIndex = size_t;
using  FrameSet   = boost::variant<FramesAll, FrameRange, FrameIndex>;

struct Script
{
	std::string meshPath;
	Resolution  res;
	AAMask      aamask;
	float       fps;
	float       duration;
	FrameSet    frames;

};

Script LoadScript(const char * path);