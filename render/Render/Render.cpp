#include "Color.h"
#include "Image.h"
#include "Mesh.h"
#include "Timer.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <algorithm>
#include <iostream>
#include <cmath>
#include <limits>

using namespace std;
using namespace Eigen;

const float pi = 3.14159265359f;

struct Triangle
{
	Vector3f v0;
	Vector3f v1;
	Vector3f v2;
};

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

Matrix3f RayCast(float width, float height, float focalDistance)
{
	// sx 0  -dx
	// 0  xy -dy
	// 0  0   f

	const float w = 1.0f;
	const float h = height / width;
	const float f = focalDistance;

	const float sx = 1.0f / width;

	Matrix3f m = Matrix3f::Zero();

	m(0, 0) =  sx;
	m(1, 1) =  sx;
	m(0, 2) = -0.5f * (w - sx);
	m(1, 2) = -0.5f * (h - sx);
	m(2, 2) =  f;

	return m;
}

float Intersect(const Triangle & tri, const Vector3f & ray)
{
	// intersect ray with the plane
	const Vector3f u = tri.v1 - tri.v0;
	const Vector3f v = tri.v2 - tri.v0;

	const Vector3f n = u.cross(v);
	if (n.isZero())
		return -1.0f;

	const float r0 = n.dot(ray);
	if (r0 == 0.0f)
		return -1.0f;
	const float r1 = n.dot(tri.v0) / r0;
	if (r1 < 0.0f)
		return -1.0f;
	const Vector3f w = r1 * ray - tri.v0;

	// compute barycentric coordinates
	const float uu = u.dot(u);
	const float uv = u.dot(v);
	const float vv = v.dot(v);
	const float wu = w.dot(u);
	const float wv = w.dot(v);

	float f = 1.0f / (uv * uv - uu * vv);

	float s = f * (uv * wv - vv * wu);
	if (s < 0.0f || s > 1.0f)
		return -1.0f;

	float t = f * (uv * wu - uu * wv);
	if (t < 0.0f || s + t > 1.0f)
		return -1.0f;

	return r1;
}

Vector3f Transform(const Matrix4f & m, const Vector3f & v)
{
	const float v0(v(0, 0)), v1(v(1, 0)), v2(v(2, 0));
	const float f = 1.0f / (m(3, 0) * v0 + m(3, 1) * v1 + m(3, 2) * v2 + m(3, 3));
	return Vector3f
		( f * (m(0, 0) * v0 + m(0, 1) * v1 + m(0, 2) * v2 + m(0, 3))
		, f * (m(1, 0) * v0 + m(1, 1) * v1 + m(1, 2) * v2 + m(1, 3))
		, f * (m(2, 0) * v0 + m(2, 1) * v1 + m(2, 2) * v2 + m(2, 3))
		);
}

void Render
	( const Matrix4f & world
	, const Matrix3f & rayCast
	,       size_t     w
	,       size_t     h
	,       Pixel    * buffer
	, const Mesh     & mesh
	)
{
	Timer timer("render", true);

	if (w == 0 && h == 0)
		return;

	vector<Triangle> faces(mesh.faces.size());
	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = ::Transform(world, mesh.vertices[mesh.faces[i].v0]);
		faces[i].v1 = ::Transform(world, mesh.vertices[mesh.faces[i].v1]);
		faces[i].v2 = ::Transform(world, mesh.vertices[mesh.faces[i].v2]);
	}

	const float noDepth(numeric_limits<float>::max());
	vector<float> depth(w * h, noDepth);

	for (size_t y(0); y != h; ++y)
	for (size_t x(0); x != w; ++x)
	{
		const int index(y * w + x);

		Vector3f ray(rayCast * Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0f));
		ray.normalize();

		for (size_t i = 0; i != faces.size(); ++i)
		{
			Timer t(timer, "intersect");

			const float z = Intersect(faces[i], ray);
			if (z >= 0.0f)
				depth[index] = min(depth[index], z);
		}
	}

	float minDepth = numeric_limits<float>::max();
	float maxDepth = numeric_limits<float>::min();
	for (size_t i(0), size(w * h); i != size; ++i)
	{
		if (depth[i] != noDepth)
		{
			minDepth = min(minDepth, depth[i]);
			maxDepth = max(maxDepth, depth[i]);
		}
	}
	float depthFactor = 100.0f / (maxDepth - minDepth);
	for (size_t i(0), size(w * h); i != size; ++i)
	{
		if (depth[i] != noDepth)
		{
			buffer[i].Alpha = 1.0f;
			buffer[i].L = 100.0f - depthFactor * (depth[i] - minDepth);
		}
	}
}

int main()
{
	const size_t w(256), h(256);

	Matrix4f world = LookAt
		( Vector3f(10.0f, 0.0f, 0.0f) // eye
		, Vector3f(00.0f, 0.0f, 0.0f) // at
		, Vector3f(00.0f, 0.0f, 1.0f) // up
		);

	Matrix3f rayCast = RayCast(static_cast<float>(w), static_cast<float>(h), 1.0);

	Mesh mesh = LoadPly("C:\\Users\\Alexey\\Programming\\Colours visualization\\shell\\hull.ply");

	vector<Pixel> buffer(w * h);

	Render(world, rayCast, w, h, buffer.data(), mesh);

	const char * path("C:\\Users\\Alexey\\Programming\\Colours visualization\\render\\test.png");
	SaveBuffer(path, w, h, buffer.data());

	return 0;
}
