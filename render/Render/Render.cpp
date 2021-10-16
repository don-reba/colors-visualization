#include "Animation.h"
#include "ModelCache.h"
#include "Path.h"
#include "Profiler.h"
#include "ProgramOptions.h"
#include "Projection.h"
#include "ProjectMesh.h"
#include "RateIndicator.h"
#include "RenderMesh.h"
#include "Script.h"

#include <Eigen/Dense>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <locale>
#include <mutex>
#include <numeric>
#include <boost/program_options.hpp>
#include <sstream>
#include <string>
#include <vector>

using namespace std;
using namespace Eigen;

namespace
{
	void PrintProfilerNode(ostream & stream, size_t level, Profiler::Node * node)
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
			const string     & name = timerStat.first;
			const Profiler::Stats & stat = timerStat.second->stats;

			for (size_t i(0); i != level; ++i)
				stream << "  ";

			string pad(nameLength + 1 - name.size(), ' ');
			stream << name << ':' << pad;

			stream << setprecision(6) << stat.Mean();
			if (stat.Count() > 1)
				stream << "(" << setprecision(6) << stat.StDev() << ")";
			stream << "s";
			if (stat.Count() > 1)
				stream << " x" << stat.Count();
			if (node->children.size() > 1)
				stream << " " << setprecision(2) << (100.0 * stat.Total() / total) << '%';

			stream << '\n';

			PrintProfilerNode(stream, level + 1, timerStat.second.get());
		}
	}

	void PrintFrameInfo(size_t frameIndex, size_t frameCount, const Profiler & profiler)
	{
		ostringstream msg;
		msg.setf(ios::fixed, ios::floatfield);

		msg << "frame " << frameIndex << " out of " << frameCount << '\n';

		PrintProfilerNode(msg, 1, profiler.current);

		cout << msg.str() << flush;
	}

	struct FrameSetPrinter : boost::static_visitor<ostream &>
	{
		ostream & stream;

		explicit FrameSetPrinter(ostream & stream) : stream(stream) {}

		ostream & operator() (FramesAll)    { return stream << "all frames"; }
		ostream & operator() (FrameRange r) { return stream << "frames " << get<0>(r) << " to " << get<1>(r); }
		ostream & operator() (FrameIndex i) { return stream << "frame " << i; }
	};

	ostream & operator << (ostream & stream, const FrameSet & frameSet)
	{
		FrameSetPrinter printer(stream);
		return boost::apply_visitor(printer, frameSet);
	}

	void PrintScript(const Script & script)
	{
		cout
			<< "Rendering '" << script.model << "' to '" << script.outputPath << "': "
			<< script.duration << "s at "
			<< script.fps << " fps, "
			<< to_string(script.res.w) << "x" << to_string(script.res.h) << ", "
			<< script.aamask.size() << "x AA, "
			<< script.frames << "\n"
			<< '\n';
	}

	struct FrameExpander : boost::static_visitor<vector<size_t>>
	{
		size_t frameCount;

		explicit FrameExpander(size_t frameCount) : frameCount(frameCount) {}

		vector<size_t> operator() (FramesAll) const
		{
			return MakeRange(0, frameCount);
		}

		vector<size_t> operator() (FrameRange r) const
		{
			size_t min = get<0>(r);
			size_t max = get<1>(r);
			return (min < frameCount)
				? MakeRange(min, std::min(max + 1, frameCount))
				: vector<size_t>();
		}

		vector<size_t> operator() (FrameIndex i) const
		{
			return { i };
		}

		vector<size_t> MakeRange(size_t min, size_t max) const
		{
			vector<size_t> frames(max - min);
			frames.reserve(frameCount);
			iota(frames.rbegin(), frames.rend(), min);
			return frames;
		}
	};

	void Run(const Path & projectRoot, const Script & script, int thread_count, Profiler & globalProfiler)
	{
		Profiler::Timer timer(globalProfiler, "total time");

		const Resolution & res(script.res);

		const Mesh mesh(LoadPly(projectRoot / script.meshPath));

		// set up the camera
		const float    focalDistance = 1.0f;
		const Matrix3f rayCast       = RayCast(res, focalDistance);
		const Matrix4f projection    = Perspective(focalDistance);

		// create an array of frame indices
		const size_t   frameCount = static_cast<size_t>(script.duration * script.fps + 0.5f);
		vector<size_t> frames     = boost::apply_visitor(FrameExpander(frameCount), script.frames);

		// about 0.02 for 1080p
		const float stepLength = 50.0f / (float)sqrt(res.w * res.w + res.h * res.h);

		// set up the model
		ModelCache modelCache;

		mutex frameMutex;

		RateIndicator rateIndicator(60.0);

		auto ProcessFrame = [&]
		{
			Animation animation(script.duration, script.rotationDuration, modelCache, projectRoot / script.model.c_str());

			vector<Vector4f> buffer(res.w * res.h);

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
					Profiler::Timer timer(profiler, "total");

					animation.SetTime(script.duration * frame / frameCount);

					fill(buffer.begin(), buffer.end(), Vector4f::Zero());

					// render
					ProjectMesh(animation.GetCamera(), projection, res, buffer.data(), mesh);
					//SaveProjectionBuffer(projectRoot / "projection.png", res, buffer.data());
					RenderMesh
						( animation.GetCamera(), rayCast, res, buffer.data(), mesh
						, animation.GetModel(), script.aamask, stepLength, profiler, rateIndicator
						);

					// post-process
					AddNoise(res, buffer.data(), script.noise);

					// save
					SaveBuffer
						( projectRoot / script.outputPath / to_string(frame) + ".png"
						, res
						, buffer.data()
						, script.background
						);
				}

				rateIndicator.Reset();
				if (script.printFrameInfo)
					PrintFrameInfo(frame, frameCount, profiler);
			}
		};

		vector<future<void>> threads(thread_count);
		for (auto & t : threads)
			t = async(launch::async, ProcessFrame);
		for (auto & t : threads)
			t.get();
	}
}

int main(int argc, char ** argv)
{
	std::ignore = cout.imbue(locale(""));

	Eigen::initParallel();

	try
	{
		ProgramOptions options(argc, argv);

		Profiler profiler;

		Script script = LoadScript(options.ProjectRoot() / "script.txt");
		PrintScript(script);

		constexpr int thread_count = 10;
		Run(options.ProjectRoot(), script, thread_count, profiler);

		std::cout << "\n";
		PrintProfilerNode(std::cout, 0, profiler.current);
	}
	catch (const exception & e)
	{
		cout << "error: " << e.what() << "\n";
	}

	return 0;
}
