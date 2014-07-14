#include "RenderMesh.h"

#include "Timer.h"

#include <limits>

using namespace Eigen;
using namespace std;

struct Triangle
{
	Vector3f v0;
	Vector3f v1;
	Vector3f v2;
};

float Intersect(const Triangle & tri, const Vector3f & ray)
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

void RenderMesh
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