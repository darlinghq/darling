#include <iostream>
#include "boost/program_options.hpp"
#include "boost/filesystem.hpp"
#include "boost/filesystem/fstream.hpp"

#include "RezParser.generated.hh"
#include "RezLexer.h"
#include "RezWorld.h"

#include "ResourceFork.h"
#include "BinaryIO.h"
#include "ResourceFile.h"
#include "Diagnostic.h"

namespace po = boost::program_options;
namespace fs = boost::filesystem;

static po::options_description desc;

static void usage()
{
    std::cerr << "Usage: " << "Rez [options] input-file\n";
    std::cerr << desc << std::endl;
}

static void CopyBinaryResources(RezWorld& world, const std::string& fn)
{
    ResourceFile copyRsrc;
    if(!copyRsrc.read(fn))
    {
        world.problem(Diagnostic(Diagnostic::error, "Could not read binary resource file " + fn, yy::location()));
    }
    else if(world.verboseFlag)
    {
        std::cerr << "Read " << copyRsrc.resources.countResources() << " resources from " << fn << "\n";
    }

    world.getResources().addResources(copyRsrc.resources);
}

int main(int argc, const char *argv[])
{
    desc.add_options()
        ("help,h", "show this help message")
        ("output,o", po::value<std::string>()->default_value("rez.output.rsrc"), "output file")
        ("append,a", "append to existing output file")
        ("type,t", po::value<std::string>()->default_value("rsrc"), "output file finder type code")
        ("creator,c", po::value<std::string>()->default_value("RSED"), "output file finder creator code")
        ("define,D", po::value<std::vector<std::string>>(), "predefine preprocessor symbol")
        ("include,I", po::value<std::vector<std::string>>(), "add include file path")
        ("copy", po::value<std::vector<std::string>>(), "copy resources from other resource file")
        ("cc", po::value<std::vector<std::string>>(), "also write output to another file")
        ("debug,d", "debug logging")
        ("data", po::value<std::string>(), "copy data fork from another file")
    ;
    po::options_description hidden, alldesc;
    hidden.add_options()
        ("input", po::value<std::vector<std::string>>(), "input file" )
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

    if(options.count("help")
        || (!options.count("input") && !options.count("copy") 
            && !options.count("output")))
    {
        usage();
        return 0;
    }

    RezWorld world;

    if(options.count("debug"))
        world.verboseFlag = true;

    std::string outfile = options["output"].as<std::string>();
    ResourceFile rsrcFile;

    if(options.count("append"))
    {
        rsrcFile.read(outfile);

        world.getResources().addResources(rsrcFile.resources);
    }

    if(options.count("data"))
    {
        std::string fn = options["data"].as<std::string>();
        ResourceFile dataFile;
        if(!dataFile.read(fn))
            world.problem(Diagnostic(Diagnostic::error, "Could not read dataresource file " + fn, yy::location()));
        rsrcFile.data = dataFile.data;
    }

    if(options.count("copy"))
        for(std::string fn : options["copy"].as<std::vector<std::string>>())
            CopyBinaryResources(world, fn);

    if(options.count("input"))
        for(std::string fn : options["input"].as<std::vector<std::string>>())
        {
            fs::path path(fn);
            if(path.extension() == ".rsrc" || path.extension() == ".bin")
            {
                CopyBinaryResources(world, fn);
            }
            else
            {
                try
                {
                    RezLexer lexer(world, fn);

                    if(options.count("define"))
                        for(std::string define : options["define"].as<std::vector<std::string>>())
                            lexer.addDefine(define);
                    if(options.count("include"))
                        for(std::string path : options["include"].as<std::vector<std::string>>())
                            lexer.addIncludePath(path);

                    if(world.verboseFlag)
                    {
                        std::cerr << "Compiling " << fn << "...\n";
                    }

                    RezParser parser(lexer, world);
                    parser.parse();
                }
                catch(...)
                {
                    world.problem(Diagnostic(Diagnostic::fatalError,"unknown error",yy::location(&fn)));
                }
            }
        }

    if(world.hadErrors)
        return 1;

    rsrcFile.resources = world.getResources();
    rsrcFile.creator = options["creator"].as<std::string>();
    rsrcFile.type = options["type"].as<std::string>();

    if(world.verboseFlag)
    {
        std::cerr << "Writing " << rsrcFile.resources.countResources() << " resources.\n";
    }
    rsrcFile.write(outfile);

    if(options.count("cc"))
        for(std::string ccFile : options["cc"].as<std::vector<std::string>>())
        {
            rsrcFile.write(ccFile);
        }

    return 0;
}
