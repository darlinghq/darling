#define FOR_MACBINARY
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include "macbinary.h"

int main(int argc, char** argv)
{
	if (argc != 3 && argc != 4)
		printHelp();

	Command cmd = getCommand(argv[1]);
	std::ifstream file;

	try
	{
		MacBinaryHeader hdr;

		file.exceptions(std::ifstream::failbit | std::ifstream::badbit | std::ifstream::eofbit);
		file.open(argv[2], std::ios_base::binary | std::ios_base::in);

		file.read((char*) &hdr, sizeof hdr);
		checkHeader(hdr);

		switch (cmd)
		{
		case PrintName:
			printName(file, hdr);
			break;
		case ExtractDF:
			extractDF(file, hdr, (argc == 4)? argv[3] : 0);
			break;
		case ExtractRF:
			extractRF(file, hdr, (argc == 4)? argv[3] : 0);
			break;
		}

		return 0;
	}
	catch (const std::ifstream::failure& e)
	{
		std::cerr << "Error reading input file: " << argv[2] << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}
}

void printHelp()
{
	std::cerr << "Usage: macbinary <command> <file> [outfile]\n\n"
		"Commands: \ndf - extract the data fork\n"
		"rf - extract the resource fork (will append '.rf' to the original file name)\n"
		"fn - print the original file name\n";
	exit(1);
}

Command getCommand(const char* cmd)
{
	if (!strcmp(cmd, "df"))
		return ExtractDF;
	else if (!strcmp(cmd, "rf"))
		return ExtractRF;
	else if (!strcmp(cmd, "pn"))
		return PrintName;
	else
	{
		printHelp();
		return PrintName; // never reached
	}
}

void printName(std::ifstream& file, const MacBinaryHeader& hdr)
{
	std::cout << std::string(hdr.filename, hdr.filename_len) << std::endl;
}

void extractRF(std::ifstream& file, const MacBinaryHeader& hdr, const char* outName)
{
	if (!hdr.resource_len)
		std::cerr << "Warning: the resource fork is empty\n";

	uint32_t extraLen = 0;
	std::string name;
	
	if (uint32_t(hdr.signature) == 'mBIN')
		extraLen = hdr.sec_header_len;
	
	extraLen += hdr.data_len;
	extraLen = (extraLen+127) / 128 * 128;
	
	if (outName)
		name = outName;
	else
		name = std::string(hdr.filename, hdr.filename_len) + ".rf";

	extract(file, 128 + extraLen, hdr.resource_len, name);
}

void extractDF(std::ifstream& file, const MacBinaryHeader& hdr, const char* outName)
{
	if (!hdr.data_len)
		std::cerr << "Warning: the data fork is empty\n";
		
	uint32_t extraLen = 0;
	std::string name;
	
	if (hdr.signature == 'mBIN')
		extraLen = hdr.sec_header_len;
	
	if (outName)
		name = outName;
	else
		name = std::string(hdr.filename, hdr.filename_len);
	
	extract(file, 128 + extraLen, hdr.data_len, name);
}

void extract(std::ifstream& file, uint32_t offset, uint32_t len, std::string outName)
{
	std::ofstream fout;
	std::ostream* out;
	uint32_t pos = 0;
	
	std::cerr << "Extracting into " << outName << std::endl;
	
	try
	{
		if (outName != "-")
		{
			fout.exceptions(std::ofstream::failbit | std::ofstream::badbit);
			fout.open(outName, std::ios_base::out | std::ios_base::binary);
			out = &fout;
		}
		else
			out = &std::cout;

		file.seekg(offset);

		while (pos < len)
		{
			char buffer[4096];
			uint32_t toRead = std::min<uint32_t>(len-pos, sizeof buffer);

			file.read(buffer, toRead);
			out->write(buffer, toRead);

			pos += toRead;
		}
	}
	catch (const std::exception& e)
	{
		std::stringstream ss;
		ss << "Error writing output file: ";
		ss << outName;
		throw ss.str();
	}
}

void checkHeader(const MacBinaryHeader& hdr)
{
	if (hdr.signature == 'mBIN')
		return; // MacBinary III
	// TODO: crc // may be MacBinary II
	if (hdr.zero2 == 0)
		return; // may be MacBinary I
}

