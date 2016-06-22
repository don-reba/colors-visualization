#include "Animation.h"
#include "Profiler.h"
#include "Projection.h"
#include "ProjectMesh.h"
#include "RenderMesh.h"
#include "Volume.h"

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <fstream>
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

		for (size_t i(0); i != level; ++i)
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

	msg << "frame " << frameIndex << " out of " << frameCount << '\n';

	PrintProfilerNode(msg, 0, profiler.current);

	cout << msg.str() << flush;
}

string MakeAnimationFilename(const string & root, size_t i)
{
	ostringstream s;
	s << root << "render\\animation\\" << i << ".png";
	return s.str();
}

struct Resolution
{
	size_t w, h;
	Resolution(size_t width, size_t height) : w(width), h(height) {}
};
const Resolution res1080p (1920, 1080);
const Resolution res720p  (1280, 720);

int main()
{
	Eigen::initParallel();

	const string projectRoot("C:\\Users\\Alexey\\Projects\\Colours visualization\\");

	const Volume volume(Volume::Load((projectRoot + "voxelize\\volume.dat").c_str(), Volume::PostprocessNone));
	//const Volume volume(Volume::MakeTest());

	const Mesh mesh(LoadPly((projectRoot + "shell\\hull.ply").c_str()));

	const float focalDistance(1.0f);

	const Resolution res(res720p);

	const Matrix3f rayCast(RayCast(static_cast<float>(res.w), static_cast<float>(res.h), focalDistance));

	const Matrix4f projection(Perspective(focalDistance));

	// set up the camera animation
	const Vector3f eye (500.0f, 0.0f, 0.0f);
	const Vector3f at  ( 50.0f, 0.0f, 0.0f);
	const Vector3f up  (  0.0f, 0.0f, 1.0f);
	const RotationAnimation animation(eye, at);

	const size_t frameCount(360);

	vector<size_t> frames(frameCount);
	iota(frames.rbegin(), frames.rend(), 0);
	//vector<size_t> frames;
	//frames.push_back(337);
	//frames.push_back(172);

	cout << volume.Nx << "x" << volume.Ny << "x" << volume.Nz << " volume" << endl;

	mutex frameMutex;

	auto ProcessFrame = [&]()
	{
		vector<Vector4f> buffer(res.w * res.h);

		const Vector3f white(100.0f, 0.005f, -0.01f);

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

			BezierLookup spline({0.1f, 0.0f}, {0.0f, 1.0f}, 1 << 20);

			{
				Profiler::Timer timer(profiler, "Total");

				fill(buffer.begin(), buffer.end(), Vector4f::Zero());

				// set up the camera
				const Matrix4f camera(LookAt(animation.Eye(frame, frameCount), at, up));

				// render
				ProjectMesh(camera, projection, res.w, res.h, buffer.data(), mesh);
				RenderMesh(camera, rayCast, res.w, res.h, buffer.data(), mesh, volume, spline, profiler);

				// save
				SaveBuffer(MakeAnimationFilename(projectRoot, frame).c_str(), res.w, res.h, buffer.data(), white);
			}

			PrintFrameInfo(frame, frameCount, profiler);
		}
	};

	array<thread, 4> threads;
	for (auto & t : threads)
		t = thread(ProcessFrame);
	for (auto & t : threads)
		t.join();

	return 0;
}
