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
		// stop if the colour becomes sufficiently opaque

		const float minAmount(1.0f / 256.0f);
		const float unitWidth(1.0f);

		float    amount(1.0f);
		Vector3f color = Vector3f::Zero();

		float t = min;

		__m256 power = _mm256_set1_ps(step / unitWidth);

		// t is incremented 8 steps at a time for SIMD to be effective
		while (t <= max && amount >= minAmount)
		{
			// get the current coordinate
			__declspec(align(32)) float tSteps[8] = {};
			for (size_t i = 0; i != 8 && t <= max; ++i)
			{
				tSteps[i] = t;
				t += step;
			}

			// p = tSteps * ray + offset
			const Vector3f256 p =
			{ _mm256_fmadd_ps(_mm256_load_ps(tSteps), _mm256_set1_ps(ray.x()), _mm256_set1_ps(offset.x()))
			, _mm256_fmadd_ps(_mm256_load_ps(tSteps), _mm256_set1_ps(ray.y()), _mm256_set1_ps(offset.y()))
			, _mm256_fmadd_ps(_mm256_load_ps(tSteps), _mm256_set1_ps(ray.z()), _mm256_set1_ps(offset.z()))
			};

			// (x + 100 < 0.1 mod 10) ? 0 : x
			//const __m256 xGrid =
			//	_mm256_and_ps
			//		( p.x
			//		, _mm256_cmp_ps
			//			( _mm256_set1_ps(0.1f)
			//			, _mm256_fmod_ps
			//				( _mm256_add_ps(p.x, _mm256_set1_ps(100.0f))
			//				, _mm256_set1_ps(10.0f)
			//				)
			//			, _CMP_LT_OQ
			//			)
			//		);

			// sample the model
			__m256 samples        = _mm256_sub_ps(_mm256_set1_ps(1.0f), model[p]);
			__m256 transparencies = _mm256_min_ps(powf8(samples, power), _mm256_set1_ps(1.0f));

			// add the new value
			__declspec(align(32)) float a[8], x[8], y[8], z[8];
			_mm256_store_ps(a, transparencies);
			//_mm256_store_ps(x, xGrid);
			_mm256_store_ps(x, p.x);
			_mm256_store_ps(y, p.y);
			_mm256_store_ps(z, p.z);

			for (size_t i = 0; i != 8 && t <= max; ++i)
			{
				const float factor = amount * (1.0f - a[i]);
				color.x() += x[i] * factor;
				color.y() += y[i] * factor;
				color.z() += z[i] * factor;
				amount *= a[i];
			}
		}

		// set pixel value
		pxl.x() += color.x();
		pxl.y() += color.y();
		pxl.z() += color.z();
		pxl.w() += 1.0f - amount;
	}

	float DistanceToTri(const Triangle3f & tri, const Vector3f & ray, bool facing)
	{
		// intersect ray with the plane
		const Vector3f u(tri.v1 - tri.v0);
		const Vector3f v(tri.v2 - tri.v0);

		const Vector3f n(u.cross(v));
		if (n.isZero())
			return -1.0f;

		const float r0(n.dot(ray));
		if (r0 == 0.0f) // parallel
			return -1.0f;
		const float d(n.dot(tri.v0) / r0);
		if (d < 0.0f) // triangle behind the camera
			return -1.0f;

		// inside-outside test
		const float dir = facing ? 1.0f : -1.0f;
		if (dir * ray.dot(tri.v0.cross(tri.v1 - tri.v0)) < 0.0f)
			return -1.0f;
		if (dir * ray.dot(tri.v1.cross(tri.v2 - tri.v1)) < 0.0f)
			return -1.0f;
		if (dir * ray.dot(tri.v2.cross(tri.v0 - tri.v2)) < 0.0f)
			return -1.0f;

		return d;
	}

	float BoundByVertex(const Triangle3f & tri, const Vector3f & ray, float distance, float (*f)(float, float))
	{
		if (distance >= 0.0f)
			return distance;
		const float d0 = ray.dot(tri.v0);
		const float d1 = ray.dot(tri.v1);
		const float d2 = ray.dot(tri.v2);
		return f(d0, f(d1, d2));
	}
}

Vector3f Transform(const Matrix4f & m, const Vector3f & v)
{
	const Vector4f y = m * Vector4f(v(0, 0), v(1, 0), v(2, 0), 1.0f);
	return Vector3f
		( y(0, 0) / y(3, 0)
		, y(1, 0) / y(3, 0)
		, y(2, 0) / y(3, 0)
		);
}

void RenderMesh
	( const Matrix4f      & camera
	, const Matrix3f      & rayCast
	,       Resolution      res
	,       Vector4f      * buffer
	, const Mesh          & mesh
	, const IModel        & model
	, const AAMask        & aamask
	,       float           stepLength
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
		faces[i].v0 = ::Transform(camera, mesh.vertices[mesh.faces[i].v0]);
		faces[i].v1 = ::Transform(camera, mesh.vertices[mesh.faces[i].v1]);
		faces[i].v2 = ::Transform(camera, mesh.vertices[mesh.faces[i].v2]);
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
		float subsamplesIntegrated = 0.0;
		for (Subsample subsample : aamask)
		{
			const Vector3f subpixel  = Vector3f(x + subsample.dx, y + subsample.dy, 1.0f);
			const Vector3f cameraRay = (rayCast * subpixel).normalized();

			float min = BoundByVertex(faces[triIndex0], cameraRay, DistanceToTri(faces[triIndex0], cameraRay, true),  std::fminf);
			float max = BoundByVertex(faces[triIndex1], cameraRay, DistanceToTri(faces[triIndex1], cameraRay, false), std::fmaxf);

			const Vector3f offset (::Transform(world, Vector3f::Zero()));
			const Vector3f ray    (::Transform(world, cameraRay) - offset);

			RefineRange(offset, ray, stepLength, min, max);

			if (min >= max)
				continue;

			Integrate(model, offset, ray, stepLength, min, max, pxl);
			subsamplesIntegrated += 1.0;
			rateIndicator.Increment();
		}

		// rescale the colour
		if (pxl.w() > 0.0f)
		{
			pxl.x() /= pxl.w();
			pxl.y() /= pxl.w();
			pxl.z() /= pxl.w();
			pxl.w() /= subsamplesIntegrated;
		}
		else
		{
			pxl = Vector4f::Zero();
		}
	}
}