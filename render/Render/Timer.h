#pragma once

#include <map>
#include <string>

#include <boost/timer.hpp>

// A helper class for profiling functions.
class Timer : boost::timer
{
private:

	struct Stats
	{
		int    n;
		double mean;
		double m2;
		double sum;

		Stats();

		void Add(double x);

		double Var() const;
		double StDev() const;
	};

private:

	bool         isEnabled;
	const char * name;
	Timer      * parent;

	std::map<std::string, Stats> children;

public:

	// Parent constructor to be called at the beginning of a function.
	explicit Timer(const char * name, bool isEnabled);

	// Child constructor to be called in nested scopes with a function.
	explicit Timer(Timer & parent, const char * name);

	~Timer();

private:

	void Print(std::ostream & out, const Timer::Stats & stats, double total);
};