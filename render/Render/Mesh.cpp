#include "Mesh.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace Eigen;
using namespace std;

Mesh::Triangle::Triangle(int v0, int v1, int v2) noexcept
	: v0(v0), v1(v1), v2(v2)
{
}

struct PlyHeader
{
	int VertexCount;
	int FaceCount;
};

PlyHeader ParseHeader(ifstream & ply)
{
	PlyHeader header = { 0 };

	string line;
	while (ply && line != "end_header")
	{
		getline(ply, line);
		istringstream tokens(line);

		string type; tokens >> type;
		if (type == "element")
		{
			string element; tokens >> element;
			/**/ if (element == "vertex") tokens >> header.VertexCount;
			else if (element == "face")   tokens >> header.FaceCount;
		}
	}

	return header;
}

Mesh LoadPly(const char * path)
{
	ifstream ply(path);
	if (!ply)
		throw runtime_error("Mesh file could not be opened.");

	PlyHeader header = ParseHeader(ply);

	Mesh mesh;

	mesh.vertices.reserve(header.VertexCount);
	for (int i = 0; i != header.VertexCount; ++i)
	{
		float x, y, z;
		ply >> x >> y >> z;
		mesh.vertices.push_back(Vector3f(x, y, z));
	}

	mesh.faces.reserve(header.FaceCount);
	for (int i = 0; i != header.FaceCount; ++i)
	{
		int n, v0, v1, v2;
		ply >> n >> v0 >> v1 >> v2;
		if (n != 3)
			throw runtime_error("encountered non-triangular face");
		mesh.faces.push_back(Mesh::Triangle(v0, v1, v2));
	}

	return mesh;
}