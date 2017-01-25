#pragma once

#include <string>

class Path
{
private:

	std::string root;

public:

	explicit Path(const char * root) : root(root) {}

	operator const char * () const
	{
		return root.c_str();
	}

	Path operator / (const std::string & relative) const
	{
		return Path((root + relative).c_str());
	}

	Path operator / (const char * relative) const
	{
		return Path((root + relative).c_str());
	}

};