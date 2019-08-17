#include "Animation.h"

#include "AlignedPtr.h"
#include "BezierValueMap.h"
#include "Path.h"
#include "Projection.h"
#include "Volume.h"

#include <algorithm>
#include <filesystem>
#include <cmath>
#include <stdexcept>

#include <boost/math/constants/constants.hpp>

using namespace Eigen;
using namespace std;

namespace
{
	constexpr float rotationSeconds = 9.0f;
}

Animation::Animation(float duration, ModelCache & modelCache, const char * modelPath)
	: duration   (duration)
	, valueMap   (make_aligned_unique<BezierValueMap>
		( Eigen::Vector2f{0.00023f, 0.0f} // mean = 0.13; 0.13 / 572 = 0.000227
		, Eigen::Vector2f{0.1f,     1.0f}
		, 0.0f, 572.0f, 0.00001f
		) )
	, modelCache (modelCache)
	, keyframes  (ReadKeyframes(modelPath)) {}

const Matrix4f & Animation::GetCamera() const
{
	return camera;
}

const IModel & Animation::GetModel() const
{
	return *mappedModel;
}

void Animation::SetTime(float time)
{
	SetCamera(time);
	SetModel(time);
}

std::vector<Animation::Keyframe> Animation::ReadKeyframes(const char * path)
{
	std::vector<Keyframe> keyframes;
	for (const auto & file : filesystem::directory_iterator(path))
		keyframes.push_back({std::stof(file.path().stem()), file.path().string()});
	if (keyframes.empty())
		throw std::runtime_error("No models found.");
	sort(keyframes.begin(), keyframes.end());
	return keyframes;
}

void Animation::SetCamera(float time)
{
	constexpr float tau = boost::math::constants::two_pi<float>();

	constexpr float rate    = 1.0f / rotationSeconds;
	constexpr float d       = 250.0f;
	constexpr float vOffset = 40.0f;

	const float a =  tau * time * rate;

	const Vector3f eye(d * cos(a), d * sin(a), vOffset);
	const Vector3f at (50.0f,      0.0f,       vOffset);
	const Vector3f up ( 0.0f,      0.0f,       1.0f);

	camera = LookAt(eye, at, up);
}

void Animation::SetModel(float time)
{
	const auto endKf   = min(keyframes.end() - 1, lower_bound(keyframes.begin(), keyframes.end(), time));
	const auto startKf = max(keyframes.begin(), endKf - 1);

	startModel = modelCache.Swap(startModel, startKf->path.c_str());
	endModel   = modelCache.Swap(endModel,   endKf->path.c_str());

	const float t = (startKf == endKf) ? 0.0f : (time - startKf->time) / (endKf->time - startKf->time);

	blendedModel = make_aligned_unique<BlendedModel>(*startModel, *endModel, t);

	mappedModel = make_aligned_unique<MappedModel>(*blendedModel, *valueMap);
}