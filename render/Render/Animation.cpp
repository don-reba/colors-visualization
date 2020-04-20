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

Animation::Animation(float animationSeconds, float rotationSeconds, ModelCache & modelCache, const char * modelPath)
	: animationSeconds (animationSeconds)
	, rotationSeconds  (rotationSeconds)
	, baseModel (modelCache.Load(modelPath))
	, bandMap
		( PiecewiseLinearValueMap::Points
			{ {0.00f, 37.320f}, {0.02f, 10.070f}, {0.04f, 7.753f}, {0.06f, 6.774f}, {0.08f, 6.121f}, {0.10f, 5.644f}
			, {0.12f,  5.259f}, {0.14f, 4.950f},  {0.16f, 4.683f}, {0.18f, 4.446f}, {0.20f, 4.237f}, {0.22f, 4.042f}
			, {0.24f,  3.859f}, {0.26f, 3.702f},  {0.28f, 3.556f}, {0.30f, 3.409f}, {0.32f, 3.270f}, {0.34f, 3.143f}
			, {0.36f,  3.019f}, {0.38f, 2.900f},  {0.40f, 2.791f}, {0.42f, 2.691f}, {0.44f, 2.600f}, {0.46f, 2.509f}
			, {0.48f,  2.423f}, {0.50f, 2.337f},  {0.52f, 2.256f}, {0.54f, 2.177f}, {0.56f, 2.099f}, {0.58f, 2.018f}
			, {0.60f,  1.939f}, {0.62f, 1.861f},  {0.64f, 1.785f}, {0.66f, 1.712f}, {0.68f, 1.642f}, {0.70f, 1.575f}
			, {0.72f,  1.515f}, {0.74f, 1.456f},  {0.76f, 1.396f}, {0.78f, 1.335f}, {0.80f, 1.277f}, {0.82f, 1.221f}
			, {0.84f,  1.166f}, {0.86f, 1.111f},  {0.88f, 1.056f}, {0.90f, 0.999f}, {0.92f, 0.936f}, {0.94f, 0.857f}
			, {0.96f,  0.776f}, {0.98f, 0.692f},  {1.00f, 0.426f}
			}
		)
	, valueMap  (make_aligned_unique<BandValueMap>(0.0f, 0.0f))
	, model     (make_aligned_unique<MappedModel>(*baseModel, *valueMap))
{}

const Matrix4f & Animation::GetCamera() const
{
	return camera;
}

const IModel & Animation::GetModel() const
{
	return *model;
}

void Animation::SetTime(float time)
{
	SetCamera(time);
	SetModel(time);
}

void Animation::SetCamera(float time)
{
	constexpr float tau = boost::math::constants::two_pi<float>();

	const float rate    = 1.0f / rotationSeconds;
	const float d       = 450.0f;
	const float vOffset = -5.0f;

	const float a =  tau * time * rate;

	const Vector3f eye(d * cos(a), d * sin(a), vOffset);
	const Vector3f at (50.0f,      0.0f,       vOffset);
	const Vector3f up ( 0.0f,      0.0f,       1.0f);

	camera = LookAt(eye, at, up);
}

void Animation::SetModel(float time)
{
	const float cycleCount = 2.0f;
	const float thickness = 0.2f;
	const float rate      = cycleCount / animationSeconds;

	float cycle;
	const float x = bandMap[modf(time * rate, &cycle)];

	*valueMap = BandValueMap(x, x + thickness);
}