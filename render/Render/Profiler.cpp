#include "Profiler.h"

#include <cmath>

using namespace std;
using namespace std::chrono;

//---------
// Profiler
//---------

Profiler::Profiler()
	: root    (new Node(nullptr))
	, current (root.get())
{
}

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

int Profiler::Stats::Count() const
{
	return n;
}

double Profiler::Stats::Mean() const
{
	return mean;
}

double Profiler::Stats::StDev() const
{
	return sqrt(Var());
}

double Profiler::Stats::Total() const
{
	return sum;
}

double Profiler::Stats::Var() const
{
	return m2 / (n - 1);
}

//----------------
// Profiler::Timer
//----------------

Profiler::Timer::Timer(Profiler & profiler, const char * name)
	: profiler (profiler)
	, start    (clock::now())
{
	auto i = profiler.current->children.find(name);
	if (i == profiler.current->children.end())
	{
		unique_ptr<Node> node(new Profiler::Node(profiler.current));
		i = profiler.current->children.insert(make_pair(name, move(node))).first;
	}
	profiler.current = i->second.get();
}

Profiler::Timer::~Timer()
{
	double secondsElapsed(duration_cast<duration<double>>(clock::now() - start).count());
	profiler.current->stats.Add(secondsElapsed);
	profiler.current = profiler.current->parent;
}
