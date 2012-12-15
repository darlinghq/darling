#include <iostream>
#include "libsshcxx/SSH.h"
#include "libsshcxx/SFTP.h"
#include "libsshcxx/SSHChannel.h"
#include "termcolor.h"
#include "pstream.h"
#include "exceptions.h"
#include "boostxml.h"
#include "timer.h"
#include <memory>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <fstream>
#include <unistd.h>
#include <sys/stat.h>

const char* DYLD_COMMAND = "dyld";
const char* OSX_HOST = "osx";
std::unique_ptr<SSH> g_ssh;
std::unique_ptr<SFTP> g_sftp;
std::unique_ptr<SSHChannel> g_shell;

void bits(const char* progname);
void runTest(const char* path);
std::string uniqueName(const std::string& path);
std::string cflags(const char* path);
const char* compiler(const char* path);
std::string stripext(std::string file);

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Specify the tests to run\n";
		return 1;
	}
	
	bits(argv[0]);
	if (const char* cmd = getenv("DYLD"))
		DYLD_COMMAND = cmd;

	try
	{
		std::cout << "Opening SSH connection...\n";
		g_ssh.reset(new SSH);
		g_ssh->setHost(OSX_HOST);
		g_ssh->connect();
		g_ssh->login();
		g_shell.reset(g_ssh->newChannel());
		g_shell->openNonInteractive();
		g_shell->startShell();
		g_sftp.reset(g_ssh->sftpChannel());

		int failures = 0;
		int offset = 0;
		timer tm;
		std::ofstream xml;
		boostxml bxml;

		if (argc >= 2 && strncmp(argv[1], "--xml=", 6) == 0)
		{
			xml.open(argv[1] + 6, std::ofstream::out);
			offset++;
		}
		
		for (int i = offset+1; i < argc; i++)
		{
			time_t timeStart, timeEnd;
			try
			{
				termcolor::set(termcolor::WHITE, termcolor::BLACK, termcolor::BRIGHT);
				std::cout << "=======\n";
				std::cout << "Running test (" << i-offset << '/' << argc-1 << "): " << argv[i] << std::endl;
				std::cout << "=======\n";
				termcolor::reset();
				
				tm.start();

				runTest(argv[i]);
				bxml.addOK(argv[i], tm.stop());
				
				termcolor::set(termcolor::GREEN, termcolor::BLACK, termcolor::BRIGHT);
				std::cout << "*** Test OK!\n";
				termcolor::reset();
			}
			catch (const std::runtime_error& e)
			{
				termcolor::setBright(termcolor::RED);
				std::cerr << "*** Test failure!\n";
				termcolor::reset();
				
				std::cerr << e.what() << std::endl;
				bxml.addFailure(argv[i], tm.stop(), e.what());
				
				failures++;
			}
			catch (const different_output_error& e)
			{
				termcolor::setBright(termcolor::RED);
				std::cerr << "*** Test failure!\n";
				termcolor::reset();
				
				termcolor::setBright(termcolor::WHITE);
				std::cerr << "Remote output:\n";
				termcolor::reset();
				std::cerr << e.remote();
				
				termcolor::setBright(termcolor::WHITE);
				std::cerr << "Local output:\n";
				termcolor::reset();
				std::cerr << e.local();

				std::stringstream ss;
				ss << "Test outputs differ!\n\n";
				ss << "Remote output:\n";
				ss << e.remote() << "\n\n";
				ss << "Local output:\n";
				ss << e.local();
				bxml.addFailure(argv[i], tm.stop(), ss.str());
				
				failures++;
			}
			catch (const nonzero_exit_error& e)
			{
				termcolor::setBright(termcolor::RED);
				std::cerr << "*** Non-zero exit status!\n";
				termcolor::reset();

				termcolor::setBright(termcolor::WHITE);
				std::cerr << ((e.remote()) ? "remote" : "local" );
				std::cerr << " test output leading to the error:\n";
				termcolor::reset();
				std::cerr << e.output();

				std::stringstream ss;
				ss << "Non-zero exit status (" << ((e.remote() ? "remote" : "local")) << ")\n";
				ss << e.output();

				bxml.addFailure(argv[i], tm.stop(), ss.str());

				failures++;
			}
		}
		
		if (!failures)
		{
			termcolor::setBright(termcolor::GREEN);
			std::cout << "ALL OK!\n";
		}
		else
		{
			termcolor::setBright(termcolor::YELLOW);
			std::cerr << failures << " test failures\n";
		}
		
		termcolor::reset();
		xml << bxml.str();
	}
	catch (const compile_error& e)
	{
		termcolor::setBright(termcolor::RED);
		std::cerr << "*** Error compiling:\n" << e.what();
		termcolor::reset();
		return 1;
	}
	catch (const std::exception& e)
	{
		termcolor::setBright(termcolor::RED);
		std::cerr << e.what() << std::endl;
		termcolor::reset();
		return 1;
	}

	return 0;
}

void runTest(const char* path)
{
	/*
	std::string line;
	g_shell->out() << "echo Hello world\n" << std::flush;
	std::getline(g_shell->in(), line);

	termcolor::set(termcolor::RED);
	std::cout << line << std::endl;
	termcolor::reset();
	*/
	
	std::stringstream cmd;
	std::string binary;
	std::string out, err;
	std::string dirname, filename = "/tmp/darlingtest-";
	int rv;

	filename += getenv("USER");
	filename += '/';
	dirname = filename;
	filename += uniqueName(path);

	mkdir(dirname.c_str(), 0700);

	binary = stripext(filename);
	
	termcolor::set(termcolor::WHITE, termcolor::BLACK, termcolor::DIM);

	std::cout << "Uploading " << path << "...\n";
	g_sftp->mkdir(dirname.c_str(), 0700);
	// upload the source code
	g_sftp->upload(path, filename);

	try
	{
		std::cout << "Compiling...\n";
		// compile the code remotely
		cmd << compiler(path) << ' ' << cflags(path) << filename << " -o " << binary;
		rv = g_ssh->runCommand(cmd.str(), out, err);

		if (rv)
			throw compile_error(err);

		std::cout << "Running remotely...\n";
		// run the program remotely
		rv = g_ssh->runCommand(binary, out, err);
		
		if (rv)
			throw nonzero_exit_error(true, out);

		std::cout << "Downloading...\n";
		// download the Mach-O executable
		g_sftp->download(binary, binary);

		std::cout << "Running locally...\n";
		// run the executable via dyld
		std::stringstream locOut;
		pstream* loc = pstream::popen(std::string(DYLD_COMMAND) + " " + binary);

		locOut << loc;
		
		rv = loc->wait();
		
		if (rv)
			throw nonzero_exit_error(false, locOut.str());

		if (locOut.str() != out)
			throw different_output_error(out, locOut.str());

		// clean up locally
		unlink(binary.c_str());

		try
		{
			// clean up remotely
			g_sftp->unlink(binary);
			g_sftp->unlink(filename);
		}
		catch (...) {}

	}
	catch (...)
	{
		// clean up locally
		unlink(binary.c_str());

		try
		{
			// clean up remotely
			g_sftp->unlink(binary);
			g_sftp->unlink(filename);
		}
		catch (...) {}

		throw;
	}
}

std::string cflags(const char* path)
{
	std::string cflags;
	const char* suffix = strrchr(path, '.');
	if (!suffix)
		throw std::runtime_error("Unsupported file name");

	if (!strcmp(DYLD_COMMAND, "dyld32") || sizeof(void*) == 4)
		cflags += "-m32 ";

	if (!strcmp(suffix, ".m") || !strcmp(suffix, ".mm"))
		cflags += "-lobjc ";

	std::ifstream f(path);
	std::string first;

	std::getline(f, first);
	if (first.compare(0, 11, "// CFLAGS: ") == 0)
	{
		cflags += first.substr(11);
		cflags += ' ';
	}

	return cflags;
}

const char* compiler(const char* path)
{
	const char* suffix = strrchr(path, '.');
	if (!suffix)
		throw std::runtime_error("Unsupported file name");

	if (!strcmp(suffix, ".c") || !strcmp(suffix, ".m"))
		return "gcc";
	else if (!strcmp(suffix, ".cpp") || !strcmp(suffix, ".mm"))
		return "g++";
	else
		throw std::runtime_error("Unsupported file name");
}


std::string uniqueName(const std::string& path)
{
	char* name = new char[path.size()+1];
	strcpy(name, path.c_str());

	std::stringstream out;
	out << getpid() << '.';
	out << basename(name);

	delete [] name;
	return out.str();
}

void bits(const char* progname)
{
	char* arg0 = strdup(progname);
	char* cmd = basename(arg0);

	termcolor::set(termcolor::WHITE, termcolor::BLACK, termcolor::DIM);
	if (!strcmp(cmd, "runtest64"))
	{
		std::cout << "Running in 64-bit mode.\n";
		DYLD_COMMAND = "dyld64";
	}
	else if (!strcmp(cmd, "runtest32"))
	{
		std::cout << "Running in 32-bit mode.\n";
		DYLD_COMMAND = "dyld32";
	}
	termcolor::reset();

	free(arg0);
}

std::string stripext(std::string file)
{
	size_t pos = file.rfind('.');
	if (pos == std::string::npos)
		return file;
	return file.substr(0, pos);
}

