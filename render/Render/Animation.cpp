#include "Animation.h"

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
using namespace boost::alignment;

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
	const float tau = 6.28318530718f;

	const float rate = 1.0f / 8.0f;

	const float a =  tau * time * rate;
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
		void * modelPointer = aligned_alloc(32, sizeof(FgtVolume));
		baseModel.reset(new(modelPointer) FgtVolume((projectRoot + "fgt\\coef s3 a6 2.0.dat").c_str()));
		//baseModel.reset(new Volume((projectRoot + "voxelize\\volume s3.dat").c_str()));
	}

	const float thickness = 1.0f;
	const float min       = 0.4f - thickness;
	const float max       = 9.0f;
	const float rate      = 1.0f / 4.0f;

	float cycle;

	const float x = min + (max - min) * modf(time * rate, &cycle);

	//void * valueMapPointer = aligned_alloc(32, sizeof(BezierValueMap));
	//valueMap.reset(new(valueMapPointer) BezierValueMap({ 0.8f, 0.0f }, { 1.0f, 1.0f }, 0.2f, 8.0f, 0.0001f));
	void * valueMapPointer = aligned_alloc(32, sizeof(BandValueMap));
	valueMap.reset(new(valueMapPointer) BandValueMap(x, x + thickness));

	void * modelPointer = aligned_alloc(32, sizeof(MappedModel));
	model.reset(new(modelPointer) MappedModel(*baseModel, *valueMap));
}