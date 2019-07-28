#include "FgtVolume.h"

#include <fstream>
#include <iostream>

#include <boost/test/unit_test.hpp>

using namespace boost;
using namespace Eigen;
using namespace std;

namespace
{
	struct NeighborResult
	{
		Vector3f Delta;
		int      Index;
	};

	struct NeighborInfo
	{
		float Sigma;
		int   Alpha;
		int   PD;
		float Side;

		int N;

		Eigen::Vector3f DomainMin, DomainMax;
		Eigen::Vector3i Count;

		vector<NeighborResult> Expected;
	};

	NeighborInfo LoadNeighborInfo(const char * path)
	{
		NeighborInfo info;

		ifstream f(path);

		f >> info.Sigma >> info.Alpha >> info.PD >> info.Side >> info.N;

		f >> info.DomainMin.x() >> info.DomainMin.y() >> info.DomainMin.z();
		f >> info.DomainMax.x() >> info.DomainMax.y() >> info.DomainMax.z();
		f >> info.Count.x()     >> info.Count.y()     >> info.Count.z();

		while (f)
		{
			float x(0), y(0), z(0);
			int i;
			f >> x >> y >> z >> i;
			if (!f) break;
			info.Expected.push_back({ Vector3f(x, y, z), i });
		}

		return info;
	}
}