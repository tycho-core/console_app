//////////////////////////////////////////////////////////////////////////////
// Copyright 2006  Martin Slater
//////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif  // _MSC_VER

#ifndef CONSOLE_APP_H_MS_2006_8_16_23_45_36_
#define CONSOLE_APP_H_MS_2006_8_16_23_45_36_


//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "consoleapp_abi.h"
#include "program_options.h"
#include "core/console.h"

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
	class console_app;

	struct console_app_options
	{
		 boost::program_options::options_description global;
		 boost::program_options::options_description visible;
		 boost::program_options::options_description run;
		 boost::program_options::options_description internal;
		 boost::program_options::positional_options_description positional;
	};

	struct TYCHO_CONSOLEAPP_ABI console_app_interface
	{	
		/// constructor
		console_app_interface() :
			m_app(0)
		{}

		/// register the applications command line parameters
		virtual void register_options(console_app_options&) {}

		/// user has requested application shutdown
		virtual void handle_exit() { exit(1) ;}

		/// main application run function
		virtual int run(boost::program_options::variables_map&) = 0;

		/// hook to allow application to display extra help information
		virtual void print_extra_help() {}

		console_app& app() { return *m_app; }
		const console_app& app() const { return *m_app; }

	private:
		friend console_app;

		console_app* m_app;

		void set_app(core::console_app& app ) { m_app = &app; }
	};
	
	/// console application object. all tools and utilities use this to
	/// provide a consistent look and feel to all command line tools
	class TYCHO_CONSOLEAPP_ABI console_app
	{
	public:
		/// constructor
		console_app(const std::string &app_name, 
					const std::string &app_desciption,
					console_app_interface *);
					
		/// destructor
		~console_app();
		
		/// main run function, should be called early in the main function
		int run(int argc, wchar_t *argv[]);

		/// creates the console application, runs and returns its exit code
		template<class Interface>
		static int main(const std::string &app_name, 
					const std::string &app_desciption,
					int argc, wchar_t *argv[])
		{
			tycho::core::console::initialise(app_name.c_str());
			Interface i;
			int ret_code = console_app(app_name, app_desciption, &i).run(argc, argv);
			tycho::core::console::shutdown();
			return ret_code;
		}

		/// write usage info to the passed stream
		void write_usage(std::wostream& str, bool long_version);
		
	private:
		void ctrl_c_handler();
		void write_usage(const console_app_options &options, bool long_version);
		void write_usage(std::wostream& str, const console_app_options &options, bool long_version);

	private:
		console_app_interface *m_interface;
		std::string m_app_name;
		std::string m_app_description;
		console_app_options m_options;		
	};	
	
} // end core namespace
} // end tycho namespace


#endif  // CONSOLE_APP_H_MS_2006_8_16_23_45_36_

