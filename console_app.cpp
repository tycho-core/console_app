//////////////////////////////////////////////////////////////////////////////
// Copyright 2006  Martin Slater
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "console_app.h"
#include "core/pc/safe_windows.h"
#include "core/platform.h"
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <functional>

//////////////////////////////////////////////////////////////////////////////
// CLASS
//////////////////////////////////////////////////////////////////////////////

static std::function<void()> exit_function;

#if TYCHO_PC
static BOOL WINAPI console_app_ctrl_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
		case CTRL_C_EVENT:
		case CTRL_BREAK_EVENT:
		case CTRL_CLOSE_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			exit_function();
			return TRUE;
		default:
			return FALSE;
	}
}
#endif // TYCHO_PC

namespace tycho
{
namespace core
{

	console_app::console_app(const std::string &app_name, 
				const std::string &app_desciption,
				console_app_interface *i) :
		m_interface(i),
		m_app_name(app_name),
		m_app_description(app_desciption)
	{
		tycho::core::name_thread(-1, "MainThread");
        exit_function = std::bind(&console_app::ctrl_c_handler, this);
#if TYCHO_PC
		SetConsoleCtrlHandler(console_app_ctrl_handler, TRUE);		
#endif
		i->set_app(*this);
	}
		
	console_app::~console_app()
	{
#if TYCHO_PC
		SetConsoleCtrlHandler(0, FALSE);		
#endif
	}	

	int console_app::run(int argc, wchar_t *argv[])
	{
		using namespace core::program_options;
		using namespace std;
		namespace po = boost::program_options;
		
		m_interface->register_options(m_options);
								
		try
		{	
			m_options.internal.add_options()
				("help", "display usage information")
	            ("response-file", po::value<string>(), "can also be specified with '@name'")
				("genresponse", po::value<std::string>(), "generate a blank response file");

			boost::program_options::options_description all_options("Command line options");
			all_options.add(m_options.internal).add(m_options.global).add(m_options.visible).add(m_options.run);
			
			po::variables_map vm;
			
			try 
			{
				po::store(po::wcommand_line_parser(argc, argv).
					options(all_options).
					positional(m_options.positional).
					extra_parser(detail::at_option_parser).run(), vm);
				po::notify(vm);
			}
			catch(const po::error &e)
			{
				std::cerr << "Invalid command line : " << e.what() << std::endl;
				write_usage(m_options, false);
				return EXIT_FAILURE;	
			}			

			if(vm.count("response-file"))
			{
				string filename(vm["response-file"].as<string>());
				ifstream ifs(filename.c_str());				
				if(!ifs)
				{
					std::cerr << "Unable to open response file " << filename << std::endl;
					return EXIT_FAILURE;				
				}

				// parse arguments from response file				
				try
				{
					arg_list args = make_param_list(ifs);
					po::store(po::command_line_parser(args).options(all_options).run(), vm);        
					po::notify(vm);
				}
				catch(const po::error &e)
				{
					std::cerr << "Invalid option in response file : " << e.what() << std::endl;
					write_usage(m_options, false);
					return EXIT_FAILURE;	
				}			
			}
			else if(vm.count("help"))
			{
				write_usage(m_options, true);
				return EXIT_SUCCESS;
			}
			else if(vm.count("genresponse"))
			{
				const std::string &filename = vm["genresponse"].as<std::string>();			
				std::ofstream ostr(filename.c_str());
				
				if(!ostr)
				{
					std::cerr << "Unable to create response file " << filename << std::endl;
					return EXIT_FAILURE;
				}
				
				po::options_description conf_options;
				conf_options.add(m_options.global).add(m_options.visible).add(m_options.run);
				write_config_file_options(m_app_name, m_app_description, conf_options, ostr);
				return EXIT_SUCCESS;
			}

			// parse arguments from the registry last
#if TYCHO_PC
			try
			{
				arg_list args = make_registry_param_list(m_app_name.c_str());
				po::store(po::command_line_parser(args).options(all_options).run(), vm);        
				po::notify(vm);
			}
			catch(const po::error &e)
			{
				std::cerr << "Invalid option in registry : " << e.what() << std::endl;
				write_usage(m_options, false);
				return EXIT_FAILURE;	
			}			
#endif

			return m_interface->run(vm);
		}
		catch(const std::exception &ex)
		{
			std::cerr << "Fatal error, terminating : "	<< ex.what() << std::endl;
			return EXIT_FAILURE;
		}
		catch(...)
		{
			std::cerr << "Fatal error, terminating : unknown" << std::endl;
			return EXIT_FAILURE;
		}
		
		return EXIT_SUCCESS;		
	}
	
	void console_app::ctrl_c_handler()
	{
		return m_interface->handle_exit();
	}

	void console_app::write_usage(std::wostream& str, bool long_version)
	{
		write_usage(str, m_options, long_version);
	}

	void console_app::write_usage(const console_app_options &options, bool long_version)
	{
		write_usage(std::wcout, m_options, long_version);
	}

	void console_app::write_usage(std::wostream& str, const console_app_options &options, bool long_version)
	{
		core::program_options::write_usage(m_app_name, m_app_description, options.global, options.run, options.internal, std::cout);

		if(long_version)
			m_interface->print_extra_help();
		std::cout << m_app_name << "  Copyright (c) Martin Slater 2014\n";
	}

} // end core namespace
} // end tycho namespace
