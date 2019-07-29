#include "Animation.h"

#include "AlignedPtr.h"
#include "BezierValueMap.h"
#include "Projection.h"
#include "Volume.h"

#include <cmath>
#include <stdexcept>

#include <boost/math/constants/constants.hpp>

using namespace Eigen;
using namespace std;

namespace
{
	constexpr float animationSeconds = 8.0f;
}

Animation::Animation(float duration, ModelCache & modelCache)
	: duration   (duration)
	, valueMap   (make_aligned_unique<BezierValueMap>
		( Eigen::Vector2f{0.00023f, 0.0f} // mean = 0.13; 0.13 / 572 = 0.000227
		, Eigen::Vector2f{0.1f,     1.0f}
		, 0.0f, 572.0f, 0.00001f
		) )
	, modelCache (modelCache) {}

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

void Animation::SetCamera(float time)
{
	constexpr float tau = boost::math::constants::two_pi<float>();

	constexpr float rate    = 1.0f / animationSeconds;
	constexpr float d       = 450.0f;
	constexpr float vOffset = -5.0f;

	const float a =  tau * time * rate;

	const Vector3f eye(d * cos(a), d * sin(a), vOffset);
	const Vector3f at (50.0f,      0.0f,       vOffset);
	const Vector3f up ( 0.0f,      0.0f,       1.0f);

	camera = LookAt(eye, at, up);
}

void Animation::SetModel(float time)
{
	startModel = std::move(modelCache.Load("C:\\Users\\Alexey\\Projects\\Colours visualization\\madoka\\gymnopedie\\fgt\\0.fgt"));
	endModel   = std::move(modelCache.Load("C:\\Users\\Alexey\\Projects\\Colours visualization\\madoka\\gymnopedie\\fgt\\1.fgt"));

	blendedModel = make_aligned_unique<BlendedModel>(*startModel, *endModel, time / duration);

	mappedModel = make_aligned_unique<MappedModel>(*blendedModel, *valueMap);
}