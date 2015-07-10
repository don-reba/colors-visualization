#include "Profiler.h"

#include <cmath>

using namespace std;
using namespace std::chrono;

//----------------
// Profiler::Stats
//----------------

Profiler::Stats::Stats()
	: n(0), mean(0.0), m2(0.0), sum(0.0)
{
}

void Profiler::Stats::Add(double x)
{
	// Knuth's online variance computation algorithm
	double delta = x - mean;
	n    += 1;
	mean += delta / n;
	m2   += delta * (x - mean);
	sum  += x;
}

double Profiler::Stats::Var() const
{
	return m2 / (n - 1);
}

double Profiler::Stats::StDev() const
{
	return sqrt(Var());
}

//----------------
// Profiler::Timer
//----------------

Profiler::Timer::Timer(Profiler & profiler, const char * name)
	: name     (name)
	, profiler (profiler)
	, start    (clock::now())
{
}

Profiler::Timer::~Timer()
{
	double secondsElapsed = duration_cast<duration<double>>(clock::now() - start).count();
	profiler.timerStats[name].Add(secondsElapsed);
}