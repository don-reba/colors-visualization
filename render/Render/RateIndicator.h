#pragma once

#include <chrono>
#include <mutex>

// Reports
class RateIndicator
{
private:

	std::chrono::steady_clock::time_point startTime;

	double frequencySeconds;

	long count;

	std::mutex lock;

public:

	RateIndicator(double frequencySeconds);
	void Increment();
	void Reset();

private:

	void ReportRate();
};
