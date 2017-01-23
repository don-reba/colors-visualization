#include "Animation.h"
#include "BezierValueMap.h"
#include "FgtVolume.h"
#include "MappedModel.h"
#include "Profiler.h"
#include "Projection.h"
#include "ProjectMesh.h"
#include "RateIndicator.h"
#include "RenderMesh.h"
#include "Volume.h"
#include "BandValueMap.h"

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

		const float  duration   = 6.0f; // seconds
		const size_t frameCount = static_cast<size_t>(duration * fps + 0.5f);
		//vector<size_t> frames = GetFrames(frameCount);
		vector<size_t> frames = { 150 };

		mutex frameMutex;

		RateIndicator rateIndicator(60.0);

		auto ProcessFrame = [&]()
		{
			vector<Vector4f> buffer(res.w * res.h);

			const Vector3f bgColor(90.0f, 0.005f, -0.01f);

			Animation animation(duration, projectRoot.c_str());

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

					animation.SetTime(duration * frame / frameCount);

					fill(buffer.begin(), buffer.end(), Vector4f::Zero());

					// render
					ProjectMesh(animation.GetCamera(), projection, res, buffer.data(), mesh);
					RenderMesh
						( animation.GetCamera(), rayCast, res, buffer.data(), mesh
						, animation.GetModel(), aamask, profiler, rateIndicator
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
				PrintFrameInfo(frame, frameCount, profiler);
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

	Run(projectRoot, res720p, aa1x, 60.0f);

	return 0;
}
