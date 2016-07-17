#include "ProjectMesh.h"

#include <iostream>
#include <stdexcept>

using namespace Eigen;
using namespace std;

struct Triangle2i
{
	Vector2i v0;
	Vector2i v1;
	Vector2i v2;
};

int min3(int v0, int v1, int v2)
{
	return min(min(v0, v1), v2);
}

int max3(int v0, int v1, int v2)
{
	return max(max(v0, v1), v2);
}

int Orient2d(const Vector2i & a, const Vector2i & b, const Vector2i & c)
{
	return (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
}

void MakeCounterclockwise(Triangle2i & tri)
{
	if (Orient2d(tri.v0, tri.v1, tri.v2) < 0)
		swap(tri.v1, tri.v2);
}

bool IsTopLeft(const Vector2i & a, const Vector2i & b)
{
	// left
	if (a.y() > b.y())
		return true;
	// top
	if (a.y() == b.y() && a.x() > b.x())
		return true;
	return false;
}

void Rasterize(const Triangle2i & tri, float id, Resolution res, Vector4f * buffer)
{
	// the triangle bounding box
	int minX(min3(tri.v0.x(), tri.v1.x(), tri.v2.x()));
	int maxX(max3(tri.v0.x(), tri.v1.x(), tri.v2.x()));
	int minY(min3(tri.v0.y(), tri.v1.y(), tri.v2.y()));
	int maxY(max3(tri.v0.y(), tri.v1.y(), tri.v2.y()));

	// intersect with the screen
	minX = max(minX, 0);
	maxX = min(maxX, static_cast<int>(res.w - 1));
	minY = max(minY, 0);
	maxY = min(maxY, static_cast<int>(res.h - 1));

	// rasterize
	Vector2i p;
	for (p.y() = minY; p.y() <= maxY; ++p.y())
	for (p.x() = minX; p.x() <= maxX; ++p.x())
	{
		// bias implements the top-left fill rule
		const int bias0(IsTopLeft(tri.v1, tri.v2) ? 0 : -1);
		const int bias1(IsTopLeft(tri.v2, tri.v0) ? 0 : -1);
		const int bias2(IsTopLeft(tri.v0, tri.v1) ? 0 : -1);

		const int w0(Orient2d(tri.v1, tri.v2, p) + bias0);
		const int w1(Orient2d(tri.v2, tri.v0, p) + bias1);
		const int w2(Orient2d(tri.v0, tri.v1, p) + bias2);

		if ((w0 | w1 | w2) >= 0)
		{
			Vector4f & pxl(buffer[p.y() * res.w + p.x()]);
			// we store pixel id in A and B components of the pixel
			// this assumed the volume is convex
			(pxl.x() == 0.0f ? pxl.x() : pxl.y()) = id;
		}
	}
}

Vector2i CameraToScreen(Vector2f v, float w, float h)
{
	return Vector2i
		( static_cast<int>(v.x() * w + 0.5f * w)
		, static_cast<int>(v.y() * w + 0.5f * h)
		);

}

Vector2f Transform(const Matrix4f & m, const Vector3f & v)
{
	const float v0(v(0, 0)), v1(v(1, 0)), v2(v(2, 0));
	const float f(1.0f / (m(3, 0) * v0 + m(3, 1) * v1 + m(3, 2) * v2 + m(3, 3)));
	return Vector2f
		( f * (m(0, 0) * v0 + m(0, 1) * v1 + m(0, 2) * v2 + m(0, 3))
		, f * (m(1, 0) * v0 + m(1, 1) * v1 + m(1, 2) * v2 + m(1, 3))
		);
}

void ProjectMesh
	( const Matrix4f   & world
	, const Matrix4f   & projection
	,       Resolution   res
	,       Vector4f   * buffer
	, const Mesh       & mesh
	)
{
	const Matrix4f m(projection * world);

	const float w(static_cast<float>(res.w));
	const float h(static_cast<float>(res.h));

	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		Triangle2i face;

		face.v0 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v0]), w, h);
		face.v1 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v1]), w, h);
		face.v2 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v2]), w, h);
		MakeCounterclockwise(face);

		const float id(static_cast<float>(i + 1));
		Rasterize(face, id, res, buffer);
	}
}