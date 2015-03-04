//////////////////////////////////////////////////////////////////////////////
// Copyright 2006  Martin Slater
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "program_options.h"

#if TYCHO_PC
#include "core/pc/safe_windows.h"
#endif


#include "boost/regex.hpp"
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// CLASS
//////////////////////////////////////////////////////////////////////////////

namespace po = boost::program_options;

namespace tycho
{
namespace core
{
namespace program_options
{
		
	namespace detail
	{
		std::pair<std::string, std::string> at_option_parser(std::string const&s)
		{
			using namespace std;
				
			if ('@' == s[0])
				return make_pair(string("response-file"), s.substr(1));
			else
				return pair<string, string>();
		}	
	}
		
	/// make a program options list from response file.
	TYCHO_CONSOLEAPP_ABI arg_list make_param_list(std::istream &istr)
	{
		using namespace std;
		arg_list result;
			
		const boost::regex comment_regex("\\s*(#.*)?$");      
		const boost::regex group_regex("\\[(.*)\\]");
		const boost::regex key_val_regex("(.*?)\\s*=\\s*(.*?)");
		std::string line;
		std::string group;
			
		while(istr.good()) 
		{
			std::getline(istr, line);

			// cleanup all comment and whitespace
			line = boost::regex_replace(line, comment_regex, "");

			// extract the key/value pair
			if(!line.empty())
			{
				boost::smatch what;
					
				if(boost::regex_match(line, what, group_regex))
					group = what[1];
				else if(boost::regex_match(line, what, key_val_regex))
				{
					string key = what[1];
					const string &val = what[2];
						
					if(!val.empty())
					{						
						size_t dot = key.find_first_of('.');
							
						if(dot == string::npos)
						{
							string newkey(group);
							if(!group.empty())								
								newkey.append(".");
							newkey.append(key);								
							key = newkey;
						}
						string arg("--");						
						arg.append(key).append("=").append(val);
						result.push_back(arg);
					}
				}
				else
				{
					// assume its a bool switch
					string arg("--");		
					if(!group.empty())				
						arg.append(group).append(".");
					arg.append(line);
					result.push_back(arg);						
				}
			}
		}			
			
		return result;
	}
		
#if TYCHO_PC
	/// searches the registry for settings and formats them so they are
	/// compatible with program options list
	TYCHO_CONSOLEAPP_ABI arg_list make_registry_param_list(const char *app_name)
	{	
		arg_list result;
		std::string key("Software\\mslater\\");
		key.append(app_name);
			
		::HKEY root_key;
		if(::RegOpenKeyA(HKEY_LOCAL_MACHINE, key.c_str(), &root_key) != ERROR_SUCCESS)
		{
			// create the key and return
			if(::RegCreateKeyA(HKEY_LOCAL_MACHINE, key.c_str(), &root_key) == ERROR_SUCCESS)
				::RegCloseKey(root_key);
			return arg_list();
		}

		DWORD index = 0;
		char name_buf[MAX_PATH];
		char data_buf[MAX_PATH];
		DWORD name_len = _countof(name_buf);
 		DWORD data_len = sizeof(data_buf);
		while(::RegEnumValueA(root_key, index, name_buf, &name_len, NULL, NULL, reinterpret_cast<LPBYTE>(data_buf), &data_len) == ERROR_SUCCESS)
		{
			std::string arg("--");						
			arg.append(name_buf).append("=").append(data_buf);
			result.push_back(arg);
			name_len = _countof(name_buf);
	 		data_len = sizeof(data_buf);
			++index;
		}
		::RegCloseKey(root_key);
		return result;
	}
#else
    arg_list make_registry_param_list(const char *)
    {
        // no-op
    }
#endif // TYCHO_PC

	TYCHO_CONSOLEAPP_ABI
    std::ostream& 
	write_usage(const std::string &appname, 
		  		const std::string &description,
				const po::options_description &global, 
				const po::options_description &run, 
				const po::options_description &internal, 
				std::ostream &ostr)
	{
		ostr << description << "\n";
		ostr << run;
		ostr << global << "\n";
		ostr << internal << "\n";
		return ostr;
	}
		
    TYCHO_CONSOLEAPP_ABI 
	std::ostream& 
	write_config_file_options(const std::string &appname, 
						const std::string &description,
						const po::options_description &desc, 
						std::ostream &ostr)
	{	
		using namespace std;
						
		ostr << "# " << appname << " configuration file\n\n";
			
		// gather options by group
		typedef vector<boost::shared_ptr<po::option_description> > option_list;
		typedef map<string, option_list> group_map;
		group_map groups;
			
		const option_list &options = desc.options();
			
		for(option_list::const_iterator it(options.begin()); it != options.end(); ++it)
		{
			const boost::shared_ptr<po::option_description> &option = *it;
			
			string name(option->long_name());				
			size_t dot = name.find_first_of('.');
				
			string group;
			if(dot != string::npos)
				group = name.substr(0, dot);				
					
			groups[group].push_back(option);
		}

		for(group_map::const_iterator it(groups.begin()); it != groups.end(); ++it)
		{
			ostr << "["	<< it->first << "]\n";
				
			for(option_list::const_iterator nit(it->second.begin()); nit != it->second.end(); ++nit)
			{
				auto option = *nit;
 				auto value = option->semantic();

				string name(option->long_name());				
				size_t dot = name.find_first_of('.');																			
				ostr << "# " << option->description();

				// hack to extract the default value for the option, only way we can get it 
				// without modifying the library is to parse it back out of the value string
				// the exception is if the option is a bool_switch() as then you cannot declare
				// it as name = value but by its presence or not
				const po::typed_value<bool> *bool_switch = dynamic_cast<const po::typed_value<bool>*>(value.get());
				if(bool_switch)
				{
					ostr << ", uncomment option to enable\n";
					bool visible = false;
					boost::any value_store;
					if(value->apply_default(value_store))					
					{
						visible = boost::any_cast<bool>(value_store);
					}
					if(!visible)
						ostr << "#";
					ostr << name.substr(dot+1) << "\n\n";
				}
				else
				{
					string msg = value->name();
					const boost::regex default_regex("arg \\(=(.*)\\)");      
					boost::smatch what;					
					std::string default_string;
					if(boost::regex_match(msg, what, default_regex))
						default_string = what[1];					
				
					ostr << "\n" << name.substr(dot+1) << "=" << default_string << "\n\n";
				}
			}
		}
			
		return ostr;
	}

	
} // end namespace
} // end namespace
} // end namespace
