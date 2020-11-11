#include "ProgramOptions.h"

#include <boost/program_options.hpp>

#include <iostream>

using namespace boost::program_options;

ProgramOptions::ProgramOptions(int argc, char ** argv)
{
	options_description options("Create a raytraced animation:");
	options.add_options()
		("root", value<std::string>(&root)->required(), "Project root path.");

	variables_map vm;
	store(parse_command_line(argc, argv, options), vm);
	notify(vm);
}

Path ProgramOptions::ProjectRoot() const {
	return Path(root);
}