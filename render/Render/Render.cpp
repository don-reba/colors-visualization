#define ANIMATE

#include "Animation.h"
#include "Profiler.h"
#include "ProjectMesh.h"
#include "RenderMesh.h"
#include "Volume.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace Eigen;

Matrix4f LookAt
	( const Vector3f & eye
	, const Vector3f & at
	, const Vector3f & up
	)
{
	Vector3f zaxis = (at - eye).normalized();
	Vector3f xaxis = zaxis.cross(up).normalized();
	Vector3f yaxis = xaxis.cross(zaxis);

	Matrix4f m;

	m.block<1, 3>(0, 0) = xaxis;
	m.block<1, 3>(1, 0) = yaxis;
	m.block<1, 3>(2, 0) = zaxis;
	m.block<1, 3>(3, 0) = Vector3f::Zero();

	m(0, 3) = -xaxis.dot(eye);
	m(1, 3) = -yaxis.dot(eye);
	m(2, 3) = -zaxis.dot(eye);
	m(3, 3) = 1.0f;

	return m;
}

Matrix4f Perspective(float focalDistance)
{
	Matrix4f m(Matrix4f::Zero());

	m(0, 0) = 1.0f;
	m(1, 1) = 1.0f;
	m(3, 2) = 1.0f / focalDistance;
	m(3, 3) = 1.0f;

	return m;
}

Matrix3f RayCast(float width, float height, float focalDistance)
{
	// sx 0  -dx
	// 0  xy -dy
	// 0  0   f

	const float w(1.0f);
	const float h(height / width);
	const float f(focalDistance);

	const float sx(1.0f / width);

	Matrix3f m(Matrix3f::Zero());

	m(0, 0) =  sx;
	m(1, 1) =  sx;
	m(0, 2) = -0.5f * (w - sx);
	m(1, 2) = -0.5f * (h - sx);
	m(2, 2) =  f;

	return m;
}

void PrintProfilerNode(ostream & msg, size_t level, Profiler::Node * node)
{
	size_t nameLength (0u);
	double total      (0.0);
	for (const auto & timerStat : node->children)
	{
		nameLength = max(nameLength, timerStat.first.size());
		total += timerStat.second->stats.Total();
	}

	for (const auto & timerStat : node->children)
	{
		const std::string     & name = timerStat.first;
		const Profiler::Stats & stat = timerStat.second->stats;

		for (size_t i = 0; i != level; ++i)
			msg << "  ";

		string pad(nameLength + 1 - name.size(), ' ');
		msg << "  " << name << ':' << pad;

		msg << setprecision(6) << stat.Mean();
		if (stat.Count() > 1)
			msg << "(" << setprecision(6) << stat.StDev() << ")";
		msg << "s";
		if (stat.Count() > 1)
			msg << " x" << stat.Count();
		if (node->children.size() > 1)
			msg << " " << setprecision(2) << (100.0 * stat.Total() / total) << '%';

		msg << '\n';

		PrintProfilerNode(msg, level + 1, timerStat.second.get());
	}
}

void PrintFrameInfo(size_t frameIndex, size_t frameCount, const Profiler & profiler)
{
	ostringstream msg;
	msg.setf(ios::fixed, ios::floatfield);

	msg << "frame " << (frameIndex + 1) << " out of " << frameCount << '\n';

	PrintProfilerNode(msg, 0, profiler.current);

	cout << msg.str() << flush;
}

#ifdef ANIMATE

string MakeAnimationFilename(const string & root, size_t i)
{
	ostringstream s;
	s << root << "render\\animation\\" << i << ".png";
	return s.str();
}

#endif

int main()
{
	Eigen::initParallel();

	const string projectRoot("D:\\Programming\\Colours visualization\\");

	const Volume volume(LoadVolume((projectRoot + "voxelize\\volume.dat").c_str()));

	const Mesh mesh(LoadPly((projectRoot + "shell\\hull.ply").c_str()));

	const float focalDistance(1.0f);

	const size_t w(1280), h(720);

	const Matrix3f rayCast(RayCast(static_cast<float>(w), static_cast<float>(h), focalDistance));

	const Matrix4f projection(Perspective(focalDistance));

	// set up the camera animation
	const Vector3f eye (500.0f, 0.0f, 0.0f);
	const Vector3f at  ( 50.0f, 0.0f, 0.0f);
	const Vector3f up  (  0.0f, 0.0f, 1.0f);
	const RotationAnimation animation(eye, at);

#ifdef ANIMATE
	const size_t frameCount(360);
#else
	const size_t frameCount(1);
#endif

	vector<size_t> frames(frameCount);
	iota(frames.rbegin(), frames.rend(), 0);

	mutex frameMutex;

	auto ProcessFrame = [&]()
	{
		vector<Vector4f> buffer(w * h);

		for (;;)
		{
			size_t frame;
			{
				lock_guard<mutex> guard(frameMutex);
				if (frames.empty())
					break;
				frame = frames.back();
				frames.pop_back();
			}

			Profiler profiler;

			{
				Profiler::Timer timer(profiler, "Total");

				fill(buffer.begin(), buffer.end(), Vector4f::Zero());

				// set up the camera
				const Matrix4f camera = LookAt(animation.Eye(frame, frameCount), at, up);

				// render
				ProjectMesh(camera, projection, w, h, buffer.data(), mesh);
				RenderMesh(camera, rayCast, w, h, buffer.data(), mesh, volume, profiler);

				// save
				#ifdef ANIMATE
					SaveBuffer(MakeAnimationFilename(projectRoot, frame).c_str(), w, h, buffer.data());
				#else
					SaveBuffer((projectRoot + "render\\test.png").c_str(), w, h, buffer.data());
				#endif
			}

			PrintFrameInfo(frame, frameCount, profiler);
		}
	};

	vector<thread> threads(4);
	for (auto & t : threads)
		t = thread(ProcessFrame);
	for (auto & t : threads)
		t.join();

	return 0;
}
