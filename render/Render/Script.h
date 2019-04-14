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

enum class ModelType { Fgt, Voxel };

std::string ToString(ModelType modelType);

struct ModelSource final
{
	ModelType   type = ModelType::Fgt;
	std::string path;
};

struct Script final
{
	std::string meshPath;
	std::string outputPath;
	ModelSource model;
	Resolution  res = res1080p;
	AAMask      aamask = aa1x;
	float       fps = 30.0f;
	float       duration = 8.0;
	FrameSet    frames = FramesAll();
	bool        printFrameInfo = true;
};

Script LoadScript(const char * path);