#include "RateIndicator.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <stdexcept>

using namespace std;
using namespace std::chrono;

RateIndicator::RateIndicator(double frequencySeconds)
	: startTime        (steady_clock::now())
	, frequencySeconds (frequencySeconds)
	, count            (0)
{
	if (frequencySeconds <= 0.0)
		throw runtime_error("Invalid frequency.");
}

void RateIndicator::Increment()
{
	lock_guard<mutex> guard(lock);

	using fseconds = duration<double>;

	double time = duration_cast<fseconds>(steady_clock::now() - startTime).count();

	++count;

	if (count > 1 && time > frequencySeconds)
	{
		ReportRate();

		startTime = steady_clock::now();
		count     = 0;
	}
}

void RateIndicator::Reset()
{
	lock_guard<mutex> guard(lock);

	startTime = steady_clock::now();
	count     = 0;
}

void RateIndicator::ReportRate()
{
	cout << round(count / frequencySeconds) << " rays per second\n" << flush;
}