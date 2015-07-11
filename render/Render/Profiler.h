#pragma once

#include <map>
#include <string>
#include <chrono>
#include <memory>
#include <vector>

// A helper class for profiling functions.
class Profiler
{
public:

	class Timer
	{
		using clock = std::chrono::steady_clock;

		Profiler          & profiler;
		clock::time_point   start;
	public:
		Timer(Profiler & profiler, const char * name);
		~Timer();

		Timer & operator = (const Timer &) = delete;
	};

	class Stats
	{
		int    n;
		double mean;
		double m2;
		double sum;
	public:
		Stats();

		void Add(double x);

		int    Count() const;
		double Mean()  const;
		double Total() const;
		double StDev() const;
		double Var()   const;
	};

	struct Node
	{
		Node  * parent;
		Stats   stats;

		std::map<std::string, std::unique_ptr<Node>> children;

		Node(Node * parent) : parent(parent) {}
	};

public:

	std::unique_ptr<Node> root;
	Node * current;

	Profiler();
};