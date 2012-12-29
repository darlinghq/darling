#include <iostream>
#include <algorithm>
#include <vector>
#include <fstream>

std::vector<std::string> split(std::string str, char c);

int main(int argc, char** argv)
{
	std::string line;
	std::ifstream ifs(argv[1]);
	std::ofstream ofs(argv[2]);

	ofs << "section .note.GNU-stack noalloc noexec nowrite progbits\n";
	ofs << "%ifidn __OUTPUT_FORMAT__, elf\n"
		"extern  _GLOBAL_OFFSET_TABLE_\n";
	ofs << "global ebxsave\n"
		"section .tbss\nalign 4\n"
		"ebxsave: dd 0\n"

		"section .text\n"

		"%macro EbxGet 0\n"
		"mov eax, ebx\n"
		"call    .get_GOT\n"
		".get_GOT:\n" 
		"pop     ebx\n" 
		"add     ebx, _GLOBAL_OFFSET_TABLE_+$$-.get_GOT wrt ..gotpc\n"
		"mov     ecx, [ebxsave wrt ..tlsie]\n"
		"mov     [gs:ecx], eax\n"
		"%endmacro\n"
		
		"%macro EbxRestore 0\n"
		"mov     ecx, [ebxsave wrt ..tlsie]\n"
		"mov     ebx, [gs:ecx]\n"
		"%endmacro\n"
		
		"%else\n"
		
		"%macro EbxGet 0\n%endmacro\n"
		"%macro EbxRestore 0\n%endmacro\n"

		"%endif\n";

	while (std::getline(ifs, line))
	{
		if (line.empty())
			continue;
		if (line[0] == '#')
			continue;
		
		std::vector<std::string> tok = split(line, ';');
		bool o32 = false, o64 = false;
		
		if (tok.size() != 2)
		{
			std::cerr << "Invalid line: " << line << std::endl;
			continue;
		}

		if (tok[0].compare(0, 3, "32!") == 0)
			o32 = true;
		else if (tok[0].compare(0, 3, "64!") == 0)
			o64 = true;
		if (o32 || o64)
			tok[0] = tok[0].substr(3);

		if (o32)
			ofs << "%ifidn __OUTPUT_FORMAT__, elf\n";
		else if (o64)
			ofs << "%ifidn __OUTPUT_FORMAT__, elf64\n";

		ofs << "global " << tok[0] << std::endl
			<< "extern " << tok[1] << std::endl
			<< tok[0] << ":\n"
			<< "\tEbxGet\n"
			<< "\tjmp " << tok[1] << " WRT ..plt\n"
			<< "\tEbxRestore\n";
		
		if (o32 || o64)
			ofs << "%endif\n";
	}

	return 0;
}

std::vector<std::string> split(std::string str, char c)
{
	std::vector<std::string> rv;
	size_t pos = 0;

	while (true)
	{
		size_t next = str.find(c, pos);
		if (next == std::string::npos)
		{
			rv.push_back(str.substr(pos));
			break;
		}
		else
		{
			rv.push_back(str.substr(pos, next-pos));
			pos = next + 1;
		}
	}

	return rv;
}

