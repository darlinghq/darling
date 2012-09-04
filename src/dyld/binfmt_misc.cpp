#include "binfmt_misc.h"
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <sys/types.h>

static void checkRoot()
{
	if (getuid())
		throw std::runtime_error("You must be root for this operation");
}

void Darling::binfmtRegister(const char* loader)
{
	checkRoot();

	std::ofstream reg("/proc/sys/fs/binfmt_misc/register");
	if (!reg.is_open())
		throw std::runtime_error("Cannot register the binfmt_misc handler, is binfmt_misc kernel support loaded and mounted?");

	// TODO: universal file handling will need to be reworked for multilib environments
	reg << ":Mach-O Universal:M::\\xca\\xfe\\xba\\xbe::" << loader << ":\n";
	reg.close();

	reg.open("/proc/sys/fs/binfmt_misc/register");
#ifdef __x86_64__
	reg << ":Mach-O 64bit:M::\\xcf\\xfa\\xed\\xfe::" << loader << ":\n";
#else
	reg << ":Mach-O 32bit:M::\\xce\\xfa\\xed\\xfe::" << loader << ":\n";
#endif
	reg.close();
}

static void deregister(const char* fmt)
{
	std::ofstream dereg(std::string("/proc/sys/fs/binfmt_misc/") + fmt);
	if (dereg.is_open())
		dereg << "-1\n";
}

void Darling::binfmtDeregister()
{
	checkRoot();

	deregister("Mach-O Universal");
#ifdef __x86_64__
	deregister("Mach-O 64bit");
#else
	deregister("Mach-O 32bit");
#endif
}

