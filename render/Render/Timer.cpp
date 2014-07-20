#include "Timer.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>

#include <boost/timer.hpp>

using namespace std;

//-------------
// Timer::Stats
//-------------

Timer::Stats::Stats()
	: n(0), mean(0.0), m2(0.0), sum(0.0)
{
}

void Timer::Stats::Add(double x)
{
	// Knuth's online variance computation algorithm
	double delta = x - mean;
	n    += 1;
	mean += delta / n;
	m2   += delta * (x - mean);
	sum  += x;
}

double Timer::Stats::Var() const
{
	return m2 / (n - 1);
}

double Timer::Stats::StDev() const
{
	return sqrt(Var());
}

//------
// Timer
//------

Timer::Timer(const char * name, bool isEnabled)
	: isEnabled (isEnabled)
	, name      (name)
	, parent    (0)
{
}

Timer::Timer(Timer & parent, const char * name)
	: isEnabled (parent.isEnabled)
	, name      (name)
	, parent    (&parent)
{
}

Timer::~Timer()
{
	if (!isEnabled)
		return;
	try
	{
		if (parent)
		{
			parent->children[name].Add(elapsed());
		}
		else
		{
			typedef map<string, Stats>::const_iterator Iterator;

			double total(elapsed());

			size_t nameLength = 0u;
			for (Iterator i(children.begin()), end(children.end()); i != end; ++i)
				nameLength = max(nameLength, i->first.size());

			cout << name << " timings\n";
			for (Iterator i(children.begin()), end(children.end()); i != end; ++i)
			{
				string pad(nameLength + 1 - i->first.size(), ' ');
				cout << "  " << i->first << ':' << pad;
				Print(cout, i->second, total);
				cout << '\n';
			}

			cout << "  total: " << total << "\n" << flush;
		}
	}
	catch (...)
	{
		// eat any exceptions
	}
}


void Timer::Print(ostream & out, const Timer::Stats & stats, double total)
{
	// from boost/progress.hpp
	try
	{
		// use istream instead of ios_base to workaround GNU problem (Greg Chicares)
		istream::fmtflags oldFlags = out.setf
			( ios::fixed
			, ios::floatfield
			);
		streamsize oldPrec(out.precision(6));

		out << stats.mean;
		if (stats.n > 1)
			out << "(" << stats.StDev() << ")";
		out << "s";
		if (stats.n > 1)
			out << " x" << stats.n;
		out.precision(2);
		out << " " << (100.0 * stats.sum / total) << '%';

		out.flags     (oldFlags);
		out.precision (oldPrec);
	}
	catch (...)
	{
		// eat any exceptions
	} 
}