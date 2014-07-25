#include "Animation.h"

#include <cmath>

using namespace Eigen;
using namespace std;

RotationAnimation::RotationAnimation(Vector3f eye)
	: eye(eye)
{
}

Vector3f RotationAnimation::Eye(size_t step, size_t count) const
{
	float a(6.28318530718f * static_cast<float>(step) / static_cast<float>(count));

	return Vector3f
		(eye.x() * cos(a) - eye.y() * sin(a)
		, eye.x() * sin(a) + eye.y() * cos(a)
		, eye.z()
		);

}