#pragma once

#include <map>
#include <string>
#include <chrono>

// A helper class for profiling functions.
class Profiler
{
public:

	struct Timer
	{
		using clock = std::chrono::steady_clock;

		std::string   name;
		Profiler    & profiler;

		clock::time_point start;

		Timer(Profiler & profiler, const char * name);
		~Timer();

		Timer & operator = (const Timer &) = delete;
	};

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

public:

	std::map<std::string, Stats> timerStats;
};