#include "RenderMesh.h"

#include "Color.h"
#include "Timer.h"

#include <iostream>
#include <limits>

using namespace Eigen;
using namespace std;

struct Triangle3f
{
	Vector3f v0;
	Vector3f v1;
	Vector3f v2;
};

float DistanceToTri(const Triangle3f & tri, const Vector3f & ray)
{
	// intersect ray with the plane
	const Vector3f u(tri.v1 - tri.v0);
	const Vector3f v(tri.v2 - tri.v0);

	const Vector3f n = u.cross(v);
	if (n.isZero())
		return -1.0f;

	const float r0 = n.dot(ray);
	if (r0 == 0.0f)
		return -1.0f;
	const float r1 = n.dot(tri.v0) / r0;
	if (r1 < 0.0f)
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

LabToRgbLookup labToRgbLookup(65536);

void Integrate
	( const Vector3f & offset
	, const Vector3f & ray
	, float            step
	, float            min
	, float            max
	, Pixel          & pxl)
{
	for (float x(min); x <= max; x += step)
	{
		Vector3f lab = offset + x * ray;
		Vector3f rgb = LabToRgb(lab, labToRgbLookup);
		if (rgb.x() < 0.0f || rgb.x() > 1.0f) continue;
		if (rgb.y() < 0.0f || rgb.y() > 1.0f) continue;
		if (rgb.z() < 0.0f || rgb.z() > 1.0f) continue;
		pxl.Alpha = 1.0f;
		pxl.L     = lab.x();
		pxl.A     = lab.y();
		pxl.B     = lab.z();
		return;
	}
	pxl.Alpha = 0.0f;
	pxl.L     = 0.0f;
	pxl.A     = 0.0f;
	pxl.B     = 0.0f;
}

void RenderMesh
	( const Matrix4f & world
	, const Matrix3f & rayCast
	,       size_t     w
	,       size_t     h
	,       Pixel    * buffer
	, const Mesh     & mesh
	)
{
	Timer timer("RenderMesh", true);

	if (w == 0 && h == 0)
		return;

	const Matrix4f worldInverse(world.inverse());

	// move the mesh into camera space
	vector<Triangle3f> faces(mesh.faces.size());
	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = ::Transform(world, mesh.vertices[mesh.faces[i].v0]);
		faces[i].v1 = ::Transform(world, mesh.vertices[mesh.faces[i].v1]);
		faces[i].v2 = ::Transform(world, mesh.vertices[mesh.faces[i].v2]);
	}

	// integrate inside the mesh
	size_t i(0);
	for (size_t y(0); y != h; ++y)
	for (size_t x(0); x != w; ++x)
	{
		Pixel & pxl(buffer[i++]);

		if (pxl.A == 0.0f || pxl.B == 0.0f)
			continue;

		const size_t triIndex0(static_cast<size_t>(pxl.A) - 1);
		const size_t triIndex1(static_cast<size_t>(pxl.B) - 1);

		Vector3f ray(rayCast * Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0f));
		ray.normalize();

		float min, max;

		{
			Timer t(timer, "Intersect");
			min = DistanceToTri(faces[triIndex0], ray); if (min < 0.0f) continue;
			max = DistanceToTri(faces[triIndex1], ray); if (max < 0.0f) continue;
			if (min > max)
				swap(min, max);
		}

		const float stepLength(0.1f);
		const Vector3f offset   = ::Transform(worldInverse, Vector3f::Zero());
		const Vector3f worldRay = ::Transform(worldInverse, ray) - offset;

		{
			Timer t(timer, "Integrate");
			Integrate(offset, worldRay, stepLength, min, max, pxl);
		}
	}
}