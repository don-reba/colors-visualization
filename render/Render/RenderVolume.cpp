#include "RenderVolume.h"

#include "Timer.h"

#include <stdexcept>

using namespace Eigen;
using namespace std;

struct Triangle
{
	Vector2f v0;
	Vector2f v1;
	Vector2f v2;
};

void SortTriangle
	( const Triangle & tri
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

void DrawLine(float sx, float sy, float ex, int w, int h, Pixel * buffer, Pixel value)
{
	while (sx < ex + 1)
	{
		int x = static_cast<size_t>(sx);
		int y = static_cast<size_t>(sy);
		if (x >= 0 && x < w && y >= 0 && y < h)
			buffer[y * w + x] = value;
		sx += 1.0f;
	}
}

void Rasterize(const Triangle & tri, size_t w, size_t h, Pixel * buffer)
{
	Pixel pxl;
	pxl.Alpha =   1.0f;
	pxl.L     =  50.0f;
	pxl.A     = -80.0f;
	pxl.B     = -70.0f;

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
			DrawLine(sx, sy, ex, w, h, buffer, pxl);
			sx += dx2; sy += 1.0f;
			ex += dx1; ey += 1.0f;
		}
		ex = x2; ey = y2;
		while (sy <= y3)
		{
			DrawLine(sx, sy, ex, w, h, buffer, pxl);
			sx += dx2; sy += 1.0f;
			ex += dx3; ey += 1.0f;
		}
	}
	else
	{
		while (sy <= y2)
		{
			DrawLine(sx, sy, ex, w, h, buffer, pxl);
			sx += dx1; sy += 1.0f;
			ex += dx2; ey += 1.0f;
		}
		sx = x2; sy = y2;
		while (sy <= y3)
		{
			DrawLine(sx, sy, ex, w, h, buffer, pxl);
			sx += dx3; sy += 1.0f;
			ex += dx2; ey += 1.0f;
		}
	}
}

Vector2f CameraToScreen(Vector2f v, size_t width, size_t height)
{
	const float w(static_cast<float>(width));
	const float h(static_cast<float>(height));
	return Vector2f
		( v.x() * w + 0.5f * w
		, v.y() * w + 0.5f * h
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

void RenderVolume
	( const Matrix4f & world
	, const Matrix4f & projection
	,       size_t     w
	,       size_t     h
	,       Pixel    * buffer
	, const Mesh     & mesh
	)
{
	Timer timer("render", true);

	vector<Triangle> faces(mesh.faces.size());

	Matrix4f m = projection * world;

	for (size_t i(0), size(mesh.faces.size()); i != size; ++i)
	{
		faces[i].v0 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v0]), w, h);
		faces[i].v1 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v1]), w, h);
		faces[i].v2 = CameraToScreen(::Transform(m, mesh.vertices[mesh.faces[i].v2]), w, h);
	}

	for (size_t i(0), size(faces.size()); i != size; ++i)
		Rasterize(faces[i], w, h, buffer);
}