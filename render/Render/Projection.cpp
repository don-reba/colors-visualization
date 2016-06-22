#include "Projection.h"

using namespace Eigen;

Matrix4f LookAt
	( const Vector3f & eye
	, const Vector3f & at
	, const Vector3f & up
	)
{
	const Vector3f zaxis((at - eye).normalized());
	const Vector3f xaxis(zaxis.cross(up).normalized());
	const Vector3f yaxis(xaxis.cross(zaxis));

	Matrix4f m;

	m.block<1, 3>(0, 0) = xaxis;
	m.block<1, 3>(1, 0) = yaxis;
	m.block<1, 3>(2, 0) = zaxis;
	m.block<1, 3>(3, 0) = Vector3f::Zero();

	m(0, 3) = -xaxis.dot(eye);
	m(1, 3) = -yaxis.dot(eye);
	m(2, 3) = -zaxis.dot(eye);
	m(3, 3) = 1.0f;

	return m;
}

Matrix4f Perspective(float focalDistance)
{
	Matrix4f m(Matrix4f::Zero());

	m(0, 0) = 1.0f;
	m(1, 1) = 1.0f;
	m(3, 2) = 1.0f / focalDistance;
	m(3, 3) = 1.0f;

	return m;
}

Matrix3f RayCast(float width, float height, float focalDistance)
{
	// sx 0  -dx
	// 0  xy -dy
	// 0  0   f

	const float w(1.0f);
	const float h(height / width);
	const float f(focalDistance);

	const float sx(1.0f / width);

	Matrix3f m(Matrix3f::Zero());

	m(0, 0) =  sx;
	m(1, 1) =  sx;
	m(0, 2) = -0.5f * (w - sx);
	m(1, 2) = -0.5f * (h - sx);
	m(2, 2) =  f;

	return m;
}
