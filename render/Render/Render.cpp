#include "Color.h"
#include "Image.h"
#include "Mesh.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <algorithm>
#include <iostream>
#include <cmath>

using namespace std;
using namespace Eigen;

const float pi = 3.14159265359f;

Matrix4f LookAtLH
	( const Vector3f & eye
	, const Vector3f & at
	, const Vector3f & up
	)
{
	Vector3f zaxis = (at - eye).normalized();
	Vector3f xaxis = up.cross(zaxis).normalized();
	Vector3f yaxis = zaxis.cross(xaxis);

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

Matrix3f RayCast(float focalDistance, float pixelSpacing, float width, float height)
{
	const float f = focalDistance;
	const float s = pixelSpacing;
	const float w = width;
	const float h = height;

	Matrix3f m = Matrix3f::Zero();

	m(0, 0) = s;
	m(1, 1) = s;
	m(0, 2) = -0.5f * (w - s);
	m(1, 2) = -0.5f * (h - s);
	m(2, 2) = f;

	return m;
}

void Print(const Vector4f & v)
{
	cout << "  " << v.hnormalized().transpose() << '\n';
}

int main(int argc, wchar_t * argv[])
{
	//----------------
	// set up the view
	//----------------

	Matrix4f m;

	m = LookAtLH
		( Vector3f(10.0f, 0.0f, 0.0f) // eye
		, Vector3f( 0.0f, 0.0f, 0.0f) // at
		, Vector3f(0.0f, 0.0f, 1.0f)  // up
		);

	//-----------------
	// set up the scene
	//-----------------

	Mesh mesh = LoadPly("C:\\Users\\Alexey\\Programming\\Colours visualization\\shell\\hull.ply");

	//-------
	// render
	//-------

	size_t w(256), h(256);
	vector<Pixel> buffer(w * h);
	for (size_t i(0), size(buffer.size()); i != size; ++i)
		buffer[i].A = 0.0f;

	const char * path("C:\\Users\\Alexey\\Programming\\Colours visualization\\render\\test.png");
	SaveBuffer(path, w, h, buffer.data());

	// cout << LabToRgb(Vector3f(47.1740f, 13.5540f, -23.8950f)).transpose() << endl;
	// cout << RgbToLab(Vector3f(0.44706f, 0.41569f,  0.59608f)).transpose() << endl;

	/*
	Vector4f v1(2.63286f, -2.57869f,  2.49635f, 1.0f);
	Vector4f v2(1.59699f,  2.40328f,  2.01660f, 1.0f);
	Vector4f v3(0.96908f,  2.37590f, -3.23591f, 1.0f);

	Matrix4f m = Perspective(35.0f, 0.1f, 100.0f) * LookAt
		( Vector3f(10.0f, 0.0f, 0.0f) // eye
		, Vector3f( 0.0f, 0.0f, 0.0f) // at
		, Vector3f(0.0f, 0.0f, 1.0f)  // up
		);

	cout << (m * v1).hnormalized() << "\n\n";
	cout << (m * v2).hnormalized() << "\n\n";
	cout << (m * v3).hnormalized() << "\n\n";
	// translate: 10, 0, 0
	// 2.63286, -2.57869, 2.49635
	// 1.59699, 2.40328, 2.0166
	// 0.96908, 2.3759, -3.23591
	// f: 35
	// clip: 0.1, 100
	*/

	return 0;
}
