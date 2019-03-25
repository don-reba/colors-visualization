#include "Animation.h"

#include "AlignedPtr.h"
#include "BezierValueMap.h"
#include "FgtVolume.h"
#include "MappedModel.h"
#include "Projection.h"
#include "Volume.h"
#include "BandValueMap.h"

#include <cmath>
#include <stdexcept>

#include <boost/align/aligned_alloc.hpp>

using namespace Eigen;
using namespace std;

Animation::Animation(float duration, const IModel & model)
	: duration  (duration)
	, baseModel (model)
{
}

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
	const float tau = 6.28318530718f;

	const float rate = 1.0f / 8.0f;

	const float a =  tau * time * rate;
	const float d = 350.0f;

	const Vector3f eye(d * cos(a), d * sin(a), 0.0f);
	const Vector3f at (50.0f,      0.0f,       0.0f);
	const Vector3f up ( 0.0f,      0.0f,       1.0f);

	camera = LookAt(eye, at, up);
}

void Animation::SetModel(float/* time*/)
{
	const float thickness = 0.1f;
	const float min       = 0.4f - thickness;
	const float max       = 13.0f;
	const float rate      = 1.0f / 16.0f;

	//float cycle;

	//const float x = min + (max - min) * modf(time * rate, &cycle);
	const float x = 1.075f;

	using Eigen::Vector2f;
	valueMap = make_aligned_unique<BezierValueMap>(Vector2f{ 1.0f, 0.0f }, Vector2f{ 1.0f, 0.0f }, 0.0f, 1.075f, 0.0001f);
	//valueMap.reset(new(valueMapPointer) BezierValueMap({ 0.8f, 0.0f }, { 1.0f, 1.0f }, 0.2f, 8.0f, 0.0001f));
	//void * valueMapPointer = aligned_alloc(32, sizeof(BandValueMap));
	//valueMap.reset(new(valueMapPointer) BandValueMap(x, x + thickness));

	model = make_aligned_unique<MappedModel>(baseModel, *valueMap);
}