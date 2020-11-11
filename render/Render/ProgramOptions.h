#pragma once

#include "Path.h"

#include <string>

class ProgramOptions
{
public:

	ProgramOptions(int argc, char ** argv);

	Path ProjectRoot() const;

public:

	std::string root;
};
