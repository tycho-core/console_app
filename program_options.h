//////////////////////////////////////////////////////////////////////////////
// Copyright 2006  Martin Slater
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif  // _MSC_VER

#ifndef PROGRAM_OPTIONS_H_MS_2006_8_9_17_55_0_
#define PROGRAM_OPTIONS_H_MS_2006_8_9_17_55_0_


//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "consoleapp_abi.h"
#include "boost/program_options.hpp"
#include <list>
#include <string>
#include <iosfwd>

//////////////////////////////////////////////////////////////////////////////
// FORWARD DECLARATIONS
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// CLASS
//////////////////////////////////////////////////////////////////////////////

namespace tycho
{
namespace core
{

namespace program_options
{
		
	typedef std::vector<std::string> arg_list;

	/// make a program options list from response file.
	TYCHO_CONSOLEAPP_ABI arg_list make_param_list(std::istream &istr);		

	/// searches the registry for settings and formats them so they are
	/// compatible with program options list
	TYCHO_CONSOLEAPP_ABI arg_list make_registry_param_list(const char *app_name);

	/// print program usage information to passed stream
	TYCHO_CONSOLEAPP_ABI std::ostream& write_usage(const std::string &appname, 
						const std::string &description,
						const boost::program_options::options_description &, 
						const boost::program_options::options_description &, 
						const boost::program_options::options_description &, 
						std::ostream &ostr);


	TYCHO_CONSOLEAPP_ABI std::ostream& write_config_file_options(const std::string &appname, 
						const std::string &description,
						const boost::program_options::options_description &desc, std::ostream &ostr);
	
	namespace detail
	{
		std::pair<std::string, std::string> at_option_parser(std::string const&s);
	}

} // end namespace
} // end namespace
} // end namespace

#endif  // PROGRAM_OPTIONS_H_MS_2006_8_9_17_55_0_

