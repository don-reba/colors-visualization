#include "RenderMesh.h"

#include "Color.h"
#include "Pow.h"

#include <cmath>

using namespace Eigen;
using namespace std;

namespace
{
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

	Vector3f TransformTo3D(const Matrix4f & m, const Vector3f & v)
	{
		const float v0(v(0, 0)), v1(v(1, 0)), v2(v(2, 0));
		const float f(1.0f / (m(3, 0) * v0 + m(3, 1) * v1 + m(3, 2) * v2 + m(3, 3)));
		return Vector3f
			( f * (m(0, 0) * v0 + m(0, 1) * v1 + m(0, 2) * v2 + m(0, 3))
			, f * (m(1, 0) * v0 + m(1, 1) * v1 + m(1, 2) * v2 + m(1, 3))
			, f * (m(2, 0) * v0 + m(2, 1) * v1 + m(2, 2) * v2 + m(2, 3))
			);
	}

	void RefineRange
		( const Vector3f & offset
		, const Vector3f & ray
		, float            step
		, float          & min
		, float          & max
		)
	{
		while (min < max && !::IsValidLab(offset + min * ray))
			min += step;
		if (min < max)
			max = min + step * ::floorf((max - min) / step);
		while (min < max && !::IsValidLab(offset + max * ray))
			max -= step;
	}

	void Integrate
		( const IModel       & model
		, const Vector3f     & offset
		, const Vector3f     & ray
		, float                step
		, float                min
		, float                max
		, Vector4f           & pxl
		)
	{
		// integrate by stepping through [min, max - step]
		// stop if the colour becomes sufficintly opaque

		const float minAmount(1.0f / 256.0f);
		const float unitWidth(1.0f);

		float    amount(1.0f);
		Vector3f color(Vector3f::Zero());

		float t = min;

		__m256 power = _mm256_set1_ps(step / unitWidth);

		// t is incremented 8 steps at a time for SIMD to be effective
		while (t <= max && amount >= minAmount)
		{
			// get the current coordinate
			__declspec(align(32)) float x[8];
			__declspec(align(32)) float y[8];
			__declspec(align(32)) float z[8];
			float t2 = t;
			for (size_t i = 0; i != 8; ++i)
			{
				x[i] = ray.x() * t2 + offset.x();
				y[i] = ray.y() * t2 + offset.y();
				z[i] = ray.z() * t2 + offset.z();
				t2 += step;
			}

			Vector3f256 p = {_mm256_load_ps(x), _mm256_load_ps(y), _mm256_load_ps(z)};

			// sample the model
			__m256 samples        = _mm256_sub_ps(_mm256_set1_ps(1.0f), model[p]);
			__m256 transparencies = _mm256_min_ps(powf8(samples, power), _mm256_set1_ps(1.0f));

			// add the new value
			__declspec(align(32)) float transparencyValues[8];
			_mm256_store_ps(transparencyValues, transparencies);

			for (size_t i = 0; i != 8 && t <= max; ++i)
			{
				float factor = amount * (1.0f - transparencyValues[i]);
				color.x() += x[i] * factor;
				color.y() += y[i] * factor;
				color.z() += z[i] * factor;
				amount *= transparencyValues[i];
				t += step;
			}
		}

		// set pixel value
		pxl.x() += color.x();
		pxl.y() += color.y();
		pxl.z() += color.z();
		pxl.w() += 1.0f - amount;
	}
}

void RenderMesh
	( const Matrix4f      & camera
	, const Matrix3f      & rayCast
	,       Resolution      res
	,       Vector4f      * buffer
	, const Mesh          & mesh
	, const IModel        & model
	, const AAMask        & aamask
	,       Profiler      & profiler
	,       RateIndicator & rateIndicator
	)
{
	Profiler::Timer timer(profiler, "RenderMesh");

	if (res.w == 0 && res.h == 0)
		return;

	const Matrix4f world(camera.inverse());

	// move the mesh into camera space
	vector<Triangle3f> faces(mesh.faces.size());
	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = ::TransformTo3D(camera, mesh.vertices[mesh.faces[i].v0]);
		faces[i].v1 = ::TransformTo3D(camera, mesh.vertices[mesh.faces[i].v1]);
		faces[i].v2 = ::TransformTo3D(camera, mesh.vertices[mesh.faces[i].v2]);
	}

	// integrate inside the mesh
	for (size_t y(0); y != res.h; ++y)
	for (size_t x(0); x != res.w; ++x)
	{
		// get bounding triangle coordinates from the pixel, then reset it
		Vector4f & pxl(buffer[y * res.w + x]);

		if (pxl.x() == 0.0f || pxl.y() == 0.0f)
		{
			pxl = Vector4f::Zero();
			continue;
		}

		const size_t triIndex0(static_cast<size_t>(pxl.x()) - 1);
		const size_t triIndex1(static_cast<size_t>(pxl.y()) - 1);

		pxl = Vector4f::Zero();

		// send a ray for every subsample, average the results
		for (auto & subsample : aamask)
		{
			Vector3f subpixel  = Vector3f(x + subsample.dx, y + subsample.dy, 1.0f);
			Vector3f cameraRay = (rayCast * subpixel).normalized();

			float min(DistanceToTri(faces[triIndex0], cameraRay)); if (min < 0.0f) continue;
			float max(DistanceToTri(faces[triIndex1], cameraRay)); if (max < 0.0f) continue;
			if (min > max)
				swap(min, max);

			const float stepLength (0.02);
			const Vector3f offset (::TransformTo3D(world, Vector3f::Zero()));
			const Vector3f ray    (::TransformTo3D(world, cameraRay) - offset);

			RefineRange(offset, ray, stepLength, min, max);

			if (min >= max)
				continue;

			Integrate(model, offset, ray, stepLength, min, max, pxl);
			rateIndicator.Increment();
		}

		// rescale the colour
		if (pxl.w() > 0.0f)
		{
			pxl.x() /= pxl.w();
			pxl.y() /= pxl.w();
			pxl.z() /= pxl.w();
			pxl.w() /= aamask.size();
		}
		else
		{
			pxl = Vector4f::Zero();
		}
	}
}