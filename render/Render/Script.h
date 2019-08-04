#pragma once

#include "Antialias.h"
#include "Resolution.h"

#include <boost\variant.hpp>
#include <Eigen/Dense>

#include <string>
#include <tuple>

struct FramesAll {};
using  FrameRange = std::tuple<size_t, size_t>;
using  FrameIndex = size_t;
using  FrameSet   = boost::variant<FramesAll, FrameRange, FrameIndex>;

enum class ModelType { Fgt, Voxel };

std::string ToString(ModelType modelType);

struct Script final
{
	using LabColor = Eigen::Vector3f;

	std::string meshPath;
	std::string outputPath;
	std::string model;
	Resolution  res = res1080p;
	AAMask      aamask = aa1x;
	float       fps = 30.0f;
	float       duration = 8.0;
	FrameSet    frames = FramesAll();
	bool        printFrameInfo = true;
	LabColor    background;
};

Script LoadScript(const char * path);