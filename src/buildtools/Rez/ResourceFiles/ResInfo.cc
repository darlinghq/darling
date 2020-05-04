#include <iostream>
#include <map>
#include <string>
#include "ResourceFile.h"
#include "boost/program_options.hpp"
namespace po = boost::program_options;
using std::map;
using std::string;

static po::options_description desc;

map<string,ResourceFile::Format> formats {
#ifdef __APPLE__
    {"real", ResourceFile::Format::real},
#endif
    {"macbin", ResourceFile::Format::macbin},
    {"basilisk", ResourceFile::Format::basilisk},
    {"applesingle", ResourceFile::Format::applesingle},
    {"underscore_appledouble", ResourceFile::Format::underscore_appledouble}
};
map<ResourceFile::Format,string> reverseFormats;

static void usage()
{
    std::cerr << "Usage: " << "ResInfo [options] input-file\n";
    std::cerr << desc << std::endl;
    std::cerr << "Available Resource Fork formats are:\n";
    for(auto p : formats)
        std::cerr << "    " << p.first << std::endl;
} 


int main(int argc, char *argv[])
{
    desc.add_options()
        ("help,h", "show this help message")
        ("type,t", "print file type")
        ("creator,c", "print creator code")
        ("all,a", "print all info")
        ("format,f", "print format")
        ("count,n", "print number of resources")
        ("size,s", "show data fork size")
        ("filename,l", "echo input file name")
        ("set-format,F", po::value<string>(), "resource fork format)")
    ;
    po::options_description hidden, alldesc;
    hidden.add_options()
        ("input", po::value<std::vector<string>>(), "input file" )
    ;
    alldesc.add(desc).add(hidden);

    po::variables_map options;
    try
    {
        auto parsed = po::command_line_parser(argc, argv)
                .options(alldesc)
                .positional(po::positional_options_description().add("input", -1))
                .style(po::command_line_style::default_style)
                .run();

        po::store(parsed, options);
    }
    catch(po::error& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl << std::endl;
        usage();
        return 1;
    }

    po::notify(options);

    if(options.count("help") || !options.count("input"))
    {
        usage();
        return 0;
    }
    
    for(auto p : formats)
        reverseFormats[p.second] = p.first;

    bool showFilename = options.count("filename");
    bool showType = options.count("type") != 0;
    bool showCreator = options.count("creator") != 0;
    bool showFormat = options.count("format") != 0;
    bool showCount = options.count("count") != 0;
    bool showSize = options.count("size") != 0;

    ResourceFile::Format format = ResourceFile::Format::autodetect;
    
    if(options.count("all"))
        showType = showCreator = showFormat = showCount = showSize = true;
    
    if(options.count("set-format"))
    {
        string s = options["set-format"].as<string>();
        if(formats.find(s) != formats.end())
            format = formats[s];
        else
        {
            std::cerr << "Unknown format " << s << std::endl << std::endl;
            usage();
            exit(1);
        }
    }
    
    if(options.count("input"))
        for(std::string fn : options["input"].as<std::vector<std::string>>())
        {
            ResourceFile rsrcFile;
            
            if(!rsrcFile.read(fn, format))
            {
                std::cerr << "Can't read file.\n";
                return 1;
            }
            
            std::ostringstream out;
            if(showType)
                out << " " << rsrcFile.type;
            if(showCreator)
                out << " " << rsrcFile.creator;
            if(showFormat)
                out << " " << reverseFormats[rsrcFile.getFormat()];
            if(showCount)
                out << " " << rsrcFile.resources.resources.size();
            if(showSize)
                out << " " << rsrcFile.data.size();

            string str = out.str();
            if(str.size())
            {
                if(showFilename)
                    std::cout << fn << ": ";            
                std::cout << out.str().substr(1) << std::endl;
            }
            else
            {
                if(showFilename)
                    std::cout << fn << std::endl;
            }
        }
        
    return 0;
}
