#include "ProjectMesh.h"

#include "Timer.h"

#include <stdexcept>

using namespace Eigen;
using namespace std;

struct Triangle2i
{
	Vector2i v0;
	Vector2i v1;
	Vector2i v2;
};

struct Triangle2f
{
	Vector2f v0;
	Vector2f v1;
	Vector2f v2;
};

void SortTriangle
	( const Triangle2f & tri
	, const Vector2f * & v0
	, const Vector2f * & v1
	, const Vector2f * & v2
	)
{
	float y0 = tri.v0.y();
	float y1 = tri.v1.y();
	float y2 = tri.v2.y();
	if (y0 <= y1)
	{
		if (y1 <= y2)
		{
			v0 = &tri.v0;
			v1 = &tri.v1;
			v2 = &tri.v2;
		}
		else
		{
			if (y0 <= y2)
			{
				v0 = &tri.v0;
				v1 = &tri.v2;
				v2 = &tri.v1;
			}
			else
			{
				v0 = &tri.v2;
				v1 = &tri.v0;
				v2 = &tri.v1;
			}
		}
	}
	else
	{
		if (y0 <= y2)
		{
			v0 = &tri.v1;
			v1 = &tri.v0;
			v2 = &tri.v2;
		}
		else
		{
			if (y1 <= y2)
			{
				v0 = &tri.v1;
				v1 = &tri.v2;
				v2 = &tri.v0;
			}
			else
			{
				v0 = &tri.v2;
				v1 = &tri.v1;
				v2 = &tri.v0;
			}
		}
	}
}

void DrawLine(float sx, float sy, float ex, int w, int h, float id, Pixel * buffer)
{
	int y = static_cast<size_t>(sy);
	if (y < 0 || y >= h)
		return;
	while (sx < ex + 1)
	{
		int x = static_cast<size_t>(sx);
		if (x >= 0 && x < w)
		{
			Pixel & pxl(buffer[y * w + x]);
			// we store pixel id in A and B components of the pixel
			(pxl.A == 0.0f ? pxl.A : pxl.B) = id;
		}
		sx += 1.0f;
	}
}

void Rasterize(const Triangle2f & tri, float id, size_t w, size_t h, Pixel * buffer)
{
	const Vector2f * v1;
	const Vector2f * v2;
	const Vector2f * v3;
	SortTriangle(tri, v1, v2, v3);
	if (v1->y() > v2->y() || v2->y() > v3->y())
		throw runtime_error("SortTriangle failed");

	float x1 = v1->x();
	float y1 = v1->y();
	float x2 = v2->x();
	float y2 = v2->y();
	float x3 = v3->x();
	float y3 = v3->y();

	float dx1(y2 > y1 ? (x2 - x1) / (y2 - y1) : x2 - x1);
	float dx2(y3 > y1 ? (x3 - x1) / (y3 - y1) : 0.0f);
	float dx3(y3 > y2 ? (x3 - x2) / (y3 - y2) : 0.0f);

	float sx = x1;
	float sy = y1;
	float ex = x1;
	float ey = y1;
	if(dx1 > dx2)
	{
		while (sy <= y2)
		{
			DrawLine(sx, sy, ex, w, h, id, buffer);
			sx += dx2; sy += 1.0f;
			ex += dx1; ey += 1.0f;
		}
		ex = x2; ey = y2;
		while (sy <= y3)
		{
			DrawLine(sx, sy, ex, w, h, id, buffer);
			sx += dx2; sy += 1.0f;
			ex += dx3; ey += 1.0f;
		}
	}
	else
	{
		while (sy <= y2)
		{
			DrawLine(sx, sy, ex, w, h, id, buffer);
			sx += dx1; sy += 1.0f;
			ex += dx2; ey += 1.0f;
		}
		sx = x2; sy = y2;
		while (sy <= y3)
		{
			DrawLine(sx, sy, ex, w, h, id, buffer);
			sx += dx3; sy += 1.0f;
			ex += dx2; ey += 1.0f;
		}
	}
}

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

void Rasterize2(const Triangle2i & tri, float id, size_t w, size_t h, Pixel * buffer)
{
	// the triangle bounding box
	int minX = min3(tri.v0.x(), tri.v1.x(), tri.v2.x());
	int maxX = max3(tri.v0.x(), tri.v1.x(), tri.v2.x());
	int minY = min3(tri.v0.y(), tri.v1.y(), tri.v2.y());
	int maxY = max3(tri.v0.y(), tri.v1.y(), tri.v2.y());

	// intersect with the screen
	minX = max(minX, 0);
	maxX = min(maxX, static_cast<int>(w - 1));
	minY = max(minY, 0);
	maxY = min(maxY, static_cast<int>(h - 1));

	// rasterize
	Vector2i p;
	for (p.y() = minY; p.y() <= maxY; ++p.y())
	for (p.x() = minX; p.x() <= maxX; ++p.x())
	{
		// bias implements the top-left fill rule
		int bias0(IsTopLeft(tri.v1, tri.v2) ? 0 : -1);
		int bias1(IsTopLeft(tri.v2, tri.v0) ? 0 : -1);
		int bias2(IsTopLeft(tri.v0, tri.v1) ? 0 : -1);

		int w0(Orient2d(tri.v1, tri.v2, p) + bias0);
		int w1(Orient2d(tri.v2, tri.v0, p) + bias1);
		int w2(Orient2d(tri.v0, tri.v1, p) + bias2);

		if ((w0 | w1 | w2) >= 0)
		{
			Pixel & pxl(buffer[p.y() * w + p.x()]);
			// we store pixel id in A and B components of the pixel
			(pxl.A == 0.0f ? pxl.A : pxl.B) = id;
		}
	}
}

Vector2i CameraToScreen(Vector2f v, size_t width, size_t height)
{
	const float w(static_cast<float>(width));
	const float h(static_cast<float>(height));
	return Vector2i
		( static_cast<int>(v.x() * w + 0.5f * w)
		, static_cast<int>(v.y() * w + 0.5f * h)
		);

}

Vector2f Transform(const Matrix4f & m, const Vector3f & v)
{
	const float v0(v(0, 0)), v1(v(1, 0)), v2(v(2, 0));
	const float f = 1.0f / (m(3, 0) * v0 + m(3, 1) * v1 + m(3, 2) * v2 + m(3, 3));
	return Vector2f
		( f * (m(0, 0) * v0 + m(0, 1) * v1 + m(0, 2) * v2 + m(0, 3))
		, f * (m(1, 0) * v0 + m(1, 1) * v1 + m(1, 2) * v2 + m(1, 3))
		);
}

void ProjectMesh
	( const Matrix4f & world
	, const Matrix4f & projection
	,       size_t     w
	,       size_t     h
	,       Pixel    * buffer
	, const Mesh     & mesh
	)
{
	Timer timer("ProjectMesh", true);

	vector<Triangle2i> faces(mesh.faces.size());

	Matrix4f m = projection * world;

	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v0]), w, h);
		faces[i].v1 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v1]), w, h);
		faces[i].v2 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v2]), w, h);
		MakeCounterclockwise(faces[i]);
	}

	for (size_t i(0), size(faces.size()); i != size; ++i)
	{
		const float id(static_cast<float>(i + 1));
		Rasterize2(faces[i], id, w, h, buffer);
	}
}