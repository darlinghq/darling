/*
This file is part of Darling.

Copyright (C) 2013 Lubos Dolezel

Darling is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Darling is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Darling.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "EHSection.h"
#include <string>
#include <stdexcept>
#include <sstream>
#include <limits>
#include <memory>
#include <cstring>
#include "../../util/log.h"

EHSection::EHSection()
{
}

EHSection::~EHSection()
{
	clear();
}

void EHSection::clear()
{
	for (CIE* cie : m_cies)
	{
		for (FDE* fde : cie->fdes)
			delete fde;
		delete cie;
	}
	m_cies.clear();
	m_ciePositions.clear();
}

	
void EHSection::load(const void* start, uintptr_t length)
{
	BufReader reader(start, length);
	
	clear();
	
	m_originalStart = reinterpret_cast<uintptr_t>(start);
	m_originalEnd = m_originalStart + length;
	
	while (!reader.atEnd())
	{
		uint64_t length;
		int32_t id;
		uintptr_t location, endPos;
		
		location = reader.pos();
		
		length = uint64_t(reader.read32()) & 0xffffffff;
		
		if (length == 0xffffffff)
			length = reader.read64();
		else if (!length)
			break; // terminating entry
		
		endPos = reader.pos() + length;
		id = reader.read32S();
		
		if (!id)
			loadCIE(reader, endPos, location);
		else
		{
			uintptr_t pos = reader.pos() - sizeof(id); // offset is from the id field to the length field of CIE
			pos -= id;
			
			auto it = m_ciePositions.find(pos);
			if (it == m_ciePositions.end())
			{
				std::stringstream ss;
				ss << "CIE not found for position " << std::hex << pos << std::dec << std::endl;
				throw std::runtime_error(ss.str());
			}
			
			loadFDE(reader, endPos, it->second);
		}
	}
}

void EHSection::loadCIE(BufReader& reader, uint64_t endPos, uintptr_t location)
{
	CIE* cie = new CIE;
	
	cie->start = reader.pos();
	cie->version = reader.read();
	cie->augmentationString = reader.readString();
	cie->codeAlign = reader.readLEB128();
	cie->dataAlign = reader.readLEB128();
	
	if (cie->codeAlign != 1)
	{
		std::stringstream ss;
		ss << "Invalid code alignment: " << int(cie->codeAlign) << std::endl;
		delete cie;
		throw std::runtime_error(ss.str());
	}
	
	if (cie->version == 1)
		cie->returnRegister = reader.read();
	else if (cie->version == 3)
		cie->returnRegister = reader.readLEB128();
	else
	{
		std::stringstream ss;
		ss << "Invalid CIE version: " << int(cie->version) << std::endl;
		delete cie;
		throw std::runtime_error(ss.str());
	}
	
	cie->augmentationDataPresent = false;
	
	uint64_t augmentationDataLength, augmentationDataEnd;
	
	cie->lsdaEncoding = cie->ptrEncoding = 0xff;
	
	for (const char* augP = cie->augmentationString; *augP; augP++)
	{
		switch (*augP)
		{
			case 'z':
				cie->augmentationDataPresent = true;
				augmentationDataLength = reader.readULEB128();
				augmentationDataEnd = reader.pos() + augmentationDataLength;
				break;
			case 'L':
				cie->lsdaEncoding = reader.read();
				break;
			case 'R':
				cie->ptrEncoding = reader.read();
				break;
			case 'P':
			{
				uint8_t persEncoding = reader.read();
				cie->personality = reader.readDwarfPointer(persEncoding);
				break;
			}
			case 'S':
				break;
			default:
			{
				std::stringstream ss;
				ss << "Unsupported character in CIE augmentation string: " << *augP;
				delete cie;
				
				throw std::runtime_error(ss.str());
			}
		}
	}
	
	reader.moveTo(augmentationDataEnd);
	
	uint64_t instructionsLength = endPos - reader.pos();
	const uint8_t* data = reinterpret_cast<const uint8_t*>(reader.readBlock(instructionsLength));
	
	cie->instructions.assign(data, data+instructionsLength);
	
	m_cies.push_back(cie);
	m_ciePositions[location] = cie;
	
	LOG << "Loaded a CIE at " << std::hex << location << std::dec << std::endl;
}

void EHSection::loadFDE(BufReader& reader, uint64_t endPos, CIE* cie)
{
	FDE* fde = new FDE;
	
	fde->startAddress = reader.readDwarfPointer(cie->ptrEncoding);
	fde->length = reader.readDwarfPointer(cie->ptrEncoding); // if ptrEncoding is relative, then in context of length, it is still just a plain number
	
	if (*cie->augmentationString == 'z')
		fde->augmentationDataLength = reader.readLEB128();
	else
		fde->augmentationDataLength = 0;
	
	if (strchr(cie->augmentationString, 'L') && cie->lsdaEncoding != 0xff) // DW_EH_PE_omit
		fde->lsdaPointer = reader.readDwarfPointer(cie->lsdaEncoding);
	
	uint64_t instructionsLength = endPos - reader.pos();
	const uint8_t* data = reinterpret_cast<const uint8_t*>(reader.readBlock(instructionsLength));
	
	fde->instructions.assign(data, data+instructionsLength);
	
	cie->fdes.push_back(fde);
}

void EHSection::store(void** mem, uintptr_t* length)
{
	uintptr_t originalLength = m_originalEnd - m_originalStart;
	uintptr_t newLength;
	
	// We're being pessimistic here, but we cannot use a resizable buffer,
	// since a possible move doing realloc() would invalidate the relative pointers
	newLength = originalLength * 2;
	
	*mem = new char[newLength];
	
	BufWriter writer (*mem, newLength);
	
	for (CIE* cie : m_cies)
	{
		uintptr_t cieStart = writer.pos();
		// Write the CIE
		storeCIE(writer, cie);
		
		// Write all the FDEs contained
		for (FDE* fde : cie->fdes)
			storeFDE(writer, fde, cie, cieStart);
	}
	
	// terminating entry
	writer.write32(0);
	
	if (length != nullptr)
		*length = writer.pos() - reinterpret_cast<uintptr_t>(*mem);
}

void EHSection::storeCIE(BufWriter& writer, CIE* cie)
{
	uint32_t* totalLength32 = nullptr;
	uint64_t* totalLength64 = nullptr;
	
	if (cie->instructions.size() < std::numeric_limits<uint32_t>::max() - 100) // approximation
	{
		totalLength32 = reinterpret_cast<uint32_t*>(writer.posPtr());
		writer.write32(0); // we fill this up later
	}
	else
	{
		writer.write32(0xffffffff);
		totalLength64 = reinterpret_cast<uint64_t*>(writer.posPtr());
		writer.write64(0); // we fill this up later
	}
	
	uintptr_t startPos = writer.pos();
	
	writer.write32(0); // id is always zero for a CIE
	
	writer.write(cie->version);
	writer.writeString(cie->augmentationString);
	writer.writeLEB128(cie->codeAlign);
	writer.writeLEB128(cie->dataAlign);
	
	if (cie->version == 1)
		writer.write(cie->returnRegister & 0xff);
	else if (cie->version == 3)
		writer.writeLEB128(cie->returnRegister);
	else
		throw std::runtime_error("Invalid CIE version");
	
	uint64_t augLength = 0;
	if (strchr(cie->augmentationString, 'L'))
		augLength++;
	if (strchr(cie->augmentationString, 'R'))
		augLength++;
	if (strchr(cie->augmentationString, 'P'))
		augLength += sizeof(void*) + 1; // sizeof(void*), because relocateToAddr() forces this
		
	writer.writeULEB128(augLength);
	
	for (const char* augP = cie->augmentationString; *augP; augP++)
	{
		switch (*augP)
		{
			case 'z':
				break;
			case 'L':
				
				writer.write(cie->lsdaEncoding);
				break;
			case 'R':
				writer.write(cie->ptrEncoding);
				break;
			case 'P':
			{
				cie->personality.relocateToAddr(writer.pos()+1, m_originalStart, m_originalEnd);
				writer.write(cie->personality.encoding);
				writer.writeDwarfPointer(cie->personality);
				break;
			}
			case 'S':
				break;
			default:
			{
				std::stringstream ss;
				ss << "Unsupported character in CIE augmentation string: " << *augP;
				delete cie;
				
				throw std::runtime_error(ss.str());
			}
		}
	}
	
	writer.writeBlock(&cie->instructions[0], cie->instructions.size());
	
	if (totalLength32)
		*totalLength32 = uint32_t( writer.pos() - startPos );
	else
		*totalLength64 = writer.pos() - startPos;
}

void EHSection::storeFDE(BufWriter& writer, FDE* fde, CIE* cie, uintptr_t cieStart)
{
	uint32_t* totalLength32 = nullptr;
	uint64_t* totalLength64 = nullptr;
	
	if (fde->instructions.size() < std::numeric_limits<uint32_t>::max() - 100) // approximation
	{
		totalLength32 = reinterpret_cast<uint32_t*>(writer.posPtr());
		writer.write32(0); // we fill this up later
	}
	else
	{
		writer.write32(0xffffffff);
		totalLength64 = reinterpret_cast<uint64_t*>(writer.posPtr());
		writer.write64(0); // we fill this up later
	}
	
	uintptr_t startPos = writer.pos();
	int32_t id = int32_t(writer.pos() - cieStart);
	writer.write32S(id);
	
	fde->startAddress.relocateToAddr(writer.pos(), m_originalStart, m_originalEnd);
	writer.writeDwarfPointer(fde->startAddress);
	writer.writeDwarfPointer(fde->length);
	
	if (*cie->augmentationString == 'z')
	{
		bool hasLSDAPtr = (strchr(cie->augmentationString, 'L') && cie->lsdaEncoding != 0xff); // DW_EH_PE_omit
		
		writer.writeLEB128(hasLSDAPtr ? DwarfPointer::getSize(cie->lsdaEncoding) : 0);
		
		if (hasLSDAPtr)
		{
			fde->lsdaPointer.relocateToAddr(writer.pos(), m_originalStart, m_originalEnd);
			writer.writeDwarfPointer(fde->lsdaPointer);
		}
	}
	
	writer.writeBlock(&fde->instructions[0], fde->instructions.size());
	
	if (totalLength32)
		*totalLength32 = uint32_t( writer.pos() - startPos );
	else
		*totalLength64 = writer.pos() - startPos;
}

void EHSection::swapRegisterNumbers(const std::vector<std::pair<int, int>>& swapList)
{
	for (CIE* cie : m_cies)
	{
		swapRegisterNumbers(cie->instructions, swapList);
		
		for (FDE* fde : cie->fdes)
			swapRegisterNumbers(fde->instructions, swapList);
	}
}

void EHSection::swapRegisterNumbers(std::vector<uint8_t>& where, const std::vector<std::pair<int, int>>& swapList)
{
	// TODO:
}
