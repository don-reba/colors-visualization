#include "Animation.h"
#include "ProjectMesh.h"
#include "RenderMesh.h"
#include "Volume.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Eigen;

Matrix4f LookAt
	( const Vector3f & eye
	, const Vector3f & at
	, const Vector3f & up
	)
{
	Vector3f zaxis = (at - eye).normalized();
	Vector3f xaxis = up.cross(zaxis).normalized();
	Vector3f yaxis = zaxis.cross(xaxis);

	Matrix4f m;

	m.block<1, 3>(0, 0) = -xaxis;
	m.block<1, 3>(1, 0) =  yaxis;
	m.block<1, 3>(2, 0) =  zaxis;
	m.block<1, 3>(3, 0) =  Vector3f::Zero();

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

int main()
{
	const string projectRoot("C:\\Users\\Alexey\\Programming\\Colours visualization\\");

	Volume v(LoadVolume((projectRoot + "voxelize\\volume.dat").c_str()));

	const Mesh mesh(LoadPly((projectRoot + "shell\\hull.ply").c_str()));

	const size_t w(1280), h(720);
	vector<Vector4f> buffer(w * h);

	const float focalDistance(1.0f);

	const Matrix3f rayCast(RayCast(static_cast<float>(w), static_cast<float>(h), focalDistance));

	const Matrix4f projection(Perspective(focalDistance));

	RotationAnimation animation(Vector3f(460.0f, 0.0f, 0.0f));

	size_t frameCount(1);

	for (size_t i(0); i != frameCount; ++i)
	{
		fill(buffer.begin(), buffer.end(), Vector4f::Zero());

		// set up the camera
		const Vector3f at(0.0f, 0.0f, 0.0f);
		const Vector3f up(0.0f, 0.0f, 1.0f);
		const Matrix4f world = LookAt(animation.Eye(i, frameCount), at, up);

		// render
		ProjectMesh(world, projection, w, h, buffer.data(), mesh);
		RenderMesh(world, rayCast, w, h, buffer.data(), mesh);

		// save
		//ostringstream pathName;
		//pathName << projectRoot << "render\\animation\\" << i << ".png";
		//SaveBuffer(pathName.str().c_str(), w, h, buffer.data());
		string pathName(projectRoot + "test.png");

		SaveBuffer((projectRoot + "test.png").c_str(), w, h, buffer.data());
	}

	return 0;
}
