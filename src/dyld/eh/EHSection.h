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

#ifndef EHSECTION_H
#define EHSECTION_H
#include <stdint.h>
#include <vector>
#include <map>
#include "BufReader.h"
#include "BufWriter.h"

class EHSection
{
public:
	EHSection();
	~EHSection();
	
	// Loads an __eh_frame
	void load(const void* start, uintptr_t length);
	
	// Serializes a previously loaded __eh_frame, appends a terminating entry
	// The returned memory must not be moved
	void store(void** mem, uintptr_t* length);
	
	// Frees all the internal structures
	void clear();
	
	void swapRegisterNumbers(const std::map<int, int>& swapList);
private:
	struct CIE;
	struct FDE;
	
	// Loads a CIE from the given reader
	// endPos: absolute address of the first byte after this CIE, used to get the instruction block length
	// location: absolute address of the length field preceding this CIE
	void loadCIE(BufReader& reader, uint64_t endPos, uintptr_t location);
	
	// Loads an FDE from the given reader
	// endPos: absolute address of the first byte after this FDE, used to get the instruction block length
	// cie: a CIE this FDE is part of, determined by the location passed in loadCIE() matched by the 'id'
	void loadFDE(BufReader& reader, uint64_t endPos, CIE* cie);
	
	void storeCIE(BufWriter& writer, CIE* cie);
	void storeFDE(BufWriter& writer, FDE* fde, CIE* cie, uintptr_t cieStart);
	
	static void swapRegisterNumbers(std::vector<uint8_t>& where, const std::map<int, int>& swapList, uint8_t ptrEncoding);
private:
	// Needed for relative pointer adjustments
	uintptr_t m_originalStart, m_originalEnd;
	
	struct FDE
	{
		DwarfPointer startAddress, length;
		int64_t augmentationDataLength;
		DwarfPointer lsdaPointer;
		
		std::vector<uint8_t> instructions;
	};
	
	struct CIE
	{
		uintptr_t start;
		uint8_t version;
		const char* augmentationString;
		int64_t codeAlign, dataAlign;
		int64_t returnRegister;
		bool augmentationDataPresent;
		uint8_t lsdaEncoding, ptrEncoding;
		DwarfPointer personality;
		
		std::vector<uint8_t> instructions;
		std::vector<FDE*> fdes;
		uint8_t* lsdaEncodingPtr; // only used when writing
	};
	
	std::vector<CIE*> m_cies;
	
	// Used to look up the CIE an FDE belongs into
	std::map<uintptr_t, CIE*> m_ciePositions;
};

#endif
