#include "Animation.h"
#include "BezierDirect.h"
#include "FgtVolume.h"
#include "MappedModel.h"
#include "Profiler.h"
#include "Projection.h"
#include "ProjectMesh.h"
#include "RateIndicator.h"
#include "RenderMesh.h"
#include "Volume.h"
#include "WallValueMap.h"

#include <Eigen/Dense>

#include <algorithm>
#include <array>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <mutex>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace std;
using namespace Eigen;

namespace
{
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

		msg << "frame " << (frameIndex + 1) << " out of " << frameCount << '\n';

		PrintProfilerNode(msg, 0, profiler.current);

		cout << msg.str() << flush;
	}

	string MakeAnimationFilename(const string & root, size_t i)
	{
		ostringstream s;
		s << root << "render\\animation\\" << i << ".png";
		return s.str();
	}

	vector<size_t> GetFrames(size_t frameCount)
	{
		vector<size_t> frames(frameCount);
		frames.reserve(frameCount);
		iota(frames.rbegin(), frames.rend(), 0);
		return frames;
	}

	void Run
		( const string     & projectRoot
		, const IModel     & volume
		, const Resolution & res
		, const AAMask     & aamask
		, float              fps
		)
	{

		const Mesh mesh(LoadPly((projectRoot + "shell\\hull.ply").c_str()));

		// set up the camera
		const float    focalDistance = 1.0f;
		const Matrix3f rayCast       = RayCast(res, focalDistance);
		const Matrix4f projection    = Perspective(focalDistance);

		// set up the camera animation
		const Vector3f eye (400.0f, 0.0f, 0.0f);
		const Vector3f at  ( 50.0f, 0.0f, 0.0f);
		const Vector3f up  (  0.0f, 0.0f, 1.0f);
		const RotationAnimation animation(eye, at);

		const size_t frameCount = static_cast<size_t>(6.0f * fps + 0.5f);
		//vector<size_t> frames = GetFrames(frameCount);
		vector<size_t> frames = { 150 };

		mutex frameMutex;

		RateIndicator rateIndicator(fps);

		auto ProcessFrame = [&]()
		{
			vector<Vector4f> buffer(res.w * res.h);

			const Vector3f bgColor(90.0f, 0.005f, -0.01f);

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
					const Matrix4f camera(LookAt(animation.Eye(frame, frameCount), at, up));

					// render
					ProjectMesh(camera, projection, res, buffer.data(), mesh);
					RenderMesh
						(camera, rayCast, res, buffer.data(), mesh
						, volume, aamask, profiler, rateIndicator
						);

					// save
					SaveBuffer
						( MakeAnimationFilename(projectRoot, frame).c_str()
						, static_cast<unsigned int>(res.w)
						, static_cast<unsigned int>(res.h)
						, buffer.data()
						, bgColor
						);
				}

				rateIndicator.Reset();
				PrintFrameInfo(frame, frames.size(), profiler);
			}
		};

		array<thread, 3> threads;
		for (auto & t : threads)
			t = thread(ProcessFrame);
		for (auto & t : threads)
			t.join();
	}
}

int main()
{
	cout.imbue(locale(""));

	Eigen::initParallel();

	const string projectRoot("C:\\Users\\Alexey\\Projects\\Colours visualization\\");

	const bool hifi = true;

	if (hifi)
		Run
			( projectRoot
			, MappedModel
				( FgtVolume((projectRoot + "fgt\\coef s3 a6 2.0.dat").c_str())
				//, BezierDirect({ 0.8f, 0.0f }, { 1.0f, 1.0f }, 0.2f, 8.0f, 0.0001f)
				, WallValueMap(6.0f, 7.0f)
				)
			, res1080p
			, aa4x
			, 60.0f
			);
	else
		Run
			( projectRoot
			, MappedModel
				( Volume((projectRoot + "voxelize\\volume s3.dat").c_str())
				, BezierDirect({ 0.8f, 0.0f }, { 1.0f, 1.0f }, 0.2f, 8.0f, 0.0001f)
				)
			, res4k
			, aa4x
			, 30.0f
			);

	return 0;
}
