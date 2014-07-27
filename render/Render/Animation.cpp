#include "Animation.h"

#include <cmath>

using namespace Eigen;
using namespace std;

RotationAnimation::RotationAnimation(Vector3f eye, Vector3f at)
	: eye(eye), at(at)
{
}

Vector3f RotationAnimation::Eye(size_t step, size_t count) const
{
	float a(6.28318530718f * static_cast<float>(step) / static_cast<float>(count));

	Vector3f d(eye - at);

	return Vector3f
		( d.x() * cos(a) - d.y() * sin(a) + at.x()
		, d.x() * sin(a) + d.y() * cos(a) + at.y()
		, eye.z()
		);

}