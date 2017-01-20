#include "Animation.h"

#include "FgtVolume.h"
#include "MappedModel.h"
#include "Projection.h"
#include "Volume.h"
#include "WallValueMap.h"

#include <cmath>
#include <stdexcept>

using namespace Eigen;
using namespace std;

Animation::Animation(float duration, const char * projectRoot)
	: duration    (duration)
	, projectRoot (projectRoot)
{
}

const Matrix4f & Animation::GetCamera() const
{
	return camera;
}

const IModel & Animation::GetModel() const
{
	if (!model)
		throw runtime_error("The model has not been loaded.");
	return *model;
}

void Animation::SetTime(float time)
{
	SetCamera(time);

	SetModel(time);
}

void Animation::SetCamera(float time)
{
	const float a = 6.28318530718f * time / duration;
	const float d = 350.0f;

	const Vector3f eye(d * cos(a), d * sin(a), 0.0f);
	const Vector3f at (50.0f,      0.0f,       0.0f);
	const Vector3f up ( 0.0f,      0.0f,       1.0f);

	camera = LookAt(eye, at, up);
}

void Animation::SetModel(float time)
{
	if (!baseModel)
	{
		baseModel.reset(new FgtVolume((projectRoot + "fgt\\coef s3 a6 2.0.dat").c_str()));
		//baseModel.reset(new Volume((projectRoot + "voxelize\\volume s3.dat").c_str()));
	}

	const float thickness = 1.0f;
	const float min       = -thickness;
	const float max       = 8.0f;
	const float rate      = 1.0f;

	float cycle;

	const float x = min + (max - min) * modf(time * rate, &cycle);

	valueMap.reset(new WallValueMap(x, x + thickness));

	model.reset(new MappedModel(*baseModel, *valueMap));
}