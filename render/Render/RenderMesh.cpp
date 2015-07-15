#include "RenderMesh.h"

#include "Color.h"
#include "Pow.h"

#include <cmath>
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

	const Vector3f n(u.cross(v));
	if (n.isZero())
		return -1.0f;

	const float r0(n.dot(ray));
	if (r0 == 0.0f)
		return -1.0f;
	const float r1(n.dot(tri.v0) / r0);
	if (r1 < 0.0f)
		return -1.0f;
	return r1;
}

Vector3f Transform(const Matrix4f & m, const Vector3f & v)
{
	const float v0(v(0, 0)), v1(v(1, 0)), v2(v(2, 0));
	const float f(1.0f / (m(3, 0) * v0 + m(3, 1) * v1 + m(3, 2) * v2 + m(3, 3)));
	return Vector3f
		( f * (m(0, 0) * v0 + m(0, 1) * v1 + m(0, 2) * v2 + m(0, 3))
		, f * (m(1, 0) * v0 + m(1, 1) * v1 + m(1, 2) * v2 + m(1, 3))
		, f * (m(2, 0) * v0 + m(2, 1) * v1 + m(2, 2) * v2 + m(2, 3))
		);
}

inline float Wrap(float n, float mod)
{
	if (n >= 0)
		return n - floor(n / mod) * mod;
	return n + ceil(-n / mod) * mod;
}

void Integrate
	( const Volume   & volume
	, const Vector3f & offset
	, const Vector3f & ray
	, float            step
	, float            min
	, float            max
	, Vector4f       & pxl
	)
{
	// integrate by stepping through [min, max - step]
	// stop if the colour becomes sufficintly opaque

	const float minAmount(1.0f / 256.0f);
	const float attenuation(0.1f);

	float    amount(1.0f);
	Vector3f color(Vector3f::Zero());

	float x(min);

	while (x <= max - 4 * step && amount >= minAmount)
	{
		Vector3f nextColors[4];
		for (size_t i(0); i != 4; ++i)
		{
			nextColors[i] = ray;
			nextColors[i] *= x;
			nextColors[i] += offset;
			x += step;
		}

		float samples[4];
		for (size_t i(0); i != 4; ++i)
			samples[i] = 1.0f - volume[nextColors[i]];

		float transparencies[4];
		_mm_store_ps(transparencies, powf4(_mm_load_ps(samples), _mm_set1_ps(step / attenuation)));

		for (size_t i(0); i != 4; ++i)
		{
			nextColors[i] *= amount * (1.0f - transparencies[i]);
			color +=  nextColors[i];
			amount *= transparencies[i];
		}
	}
	while (x <= max - step && amount >= minAmount)
	{
		Vector3f nextColor(offset + x * ray); x += step;

		float sample(volume[nextColor]);

		float transparency(pow(1.0f - sample, step / attenuation));

		color += amount * (1.0f - transparency) * nextColor;

		amount *= transparency;
	}

	// add the remaining half-step
	// the formula works even for zero remainder

	Vector3f nextColor(offset + x * ray);

	float transparency(pow(1.0f - volume[nextColor], (max - x) / attenuation));

	color += amount * (1.0f - transparency) * nextColor;

	amount *= transparency;

	// rescale colour

	if (amount != 1.0f)
		color /= 1.0f - amount;

	// set pixel value

	pxl.x() = color.x();
	pxl.y() = color.y();
	pxl.z() = color.z();
	pxl.w() = 1.0f - amount;
}

void RenderMesh
	( const Matrix4f & camera
	, const Matrix3f & rayCast
	,       size_t     w
	,       size_t     h
	,       Vector4f * buffer
	, const Mesh     & mesh
	, const Volume   & volume
	,       Profiler & profiler
	)
{
	Profiler::Timer timer(profiler, "RenderMesh");

	if (w == 0 && h == 0)
		return;

	const Matrix4f world(camera.inverse());

	// move the mesh into camera space
	vector<Triangle3f> faces(mesh.faces.size());
	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = ::Transform(camera, mesh.vertices[mesh.faces[i].v0]);
		faces[i].v1 = ::Transform(camera, mesh.vertices[mesh.faces[i].v1]);
		faces[i].v2 = ::Transform(camera, mesh.vertices[mesh.faces[i].v2]);
	}

	// integrate inside the mesh
	for (size_t y(0); y != h; ++y)
	for (size_t x(0); x != w; ++x)
	{
		Vector4f & pxl(buffer[y * w + x]);

		if (pxl.x() == 0.0f || pxl.y() == 0.0f)
			continue;

		const size_t triIndex0(static_cast<size_t>(pxl.x()) - 1);
		const size_t triIndex1(static_cast<size_t>(pxl.y()) - 1);

		Vector3f cameraRay(rayCast * Vector3f(static_cast<float>(x), static_cast<float>(y), 1.0f));
		cameraRay.normalize();

		float min(DistanceToTri(faces[triIndex0], cameraRay)); if (min < 0.0f) continue;
		float max(DistanceToTri(faces[triIndex1], cameraRay)); if (max < 0.0f) continue;
		if (min > max)
			swap(min, max);

		const float stepLength (0.2f);
		const Vector3f offset (::Transform(world, Vector3f::Zero()));
		const Vector3f ray    (::Transform(world, cameraRay) - offset);

		Integrate(volume, offset, ray, stepLength, min, max, pxl);
	}
}