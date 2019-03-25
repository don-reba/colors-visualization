#pragma once

#include <string>

class Path
{
private:

	std::string root;

public:

	Path(std::string root) : root(std::move(root)) {}

	operator const char * () const
	{
		return root.c_str();
	}

	Path operator + (const std::string & str) const
	{
		return root + str;
	}

	Path operator / (const std::string & relative) const
	{
		return root + "\\" + relative;
	}
};