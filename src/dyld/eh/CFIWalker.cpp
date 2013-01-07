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

#include "CFIWalker.h"
#include <sstream>
#include <stdexcept>
#include "log.h"

CFIWalker::CFIWalker(uint8_t* instructions, uintptr_t length, uint8_t dwarfPtrEncoding)
: m_buffer(instructions, length), m_dwarfPtrEncoding(dwarfPtrEncoding)
{
}

void CFIWalker::processNextReg(void* opaque, int (*mapRegisterNumber)(void*, int))
{
	uint64_t reg, newReg;
	
	reg = m_buffer.readULEB128();
	newReg = mapRegisterNumber(opaque, reg);
	
	if (newReg != reg)
	{
		LOG << "Updating register number: " << reg << " -> " << newReg << std::endl;
		m_buffer.updateLastULEB128(newReg);
	}
}

uint8_t CFIWalker::processMixedInstr(void* opaque, int (*mapRegisterNumber)(void*, int), uint8_t instr)
{
	int origRegNo = int(instr) & 0x3f;
	int newRegNo = mapRegisterNumber(opaque, origRegNo);
	
	if (origRegNo != newRegNo)
	{
		LOG << "Updating mixed instr. register number: " << origRegNo << " -> " << newRegNo << std::endl;
		return (uint8_t) (instr & 0xC0) | (newRegNo & 0x3f);
	}
	else
		return instr;
}

void CFIWalker::walk(void* opaque, int (*mapRegisterNumber)(void*,int))
{
	while (!m_buffer.atEnd())
	{
		const uint8_t instr = m_buffer.read();
		uint64_t len;
		
		switch (instr)
		{
			case DW_CFA_nop:
				break;
			case DW_CFA_advance_loc1:
				m_buffer.read();
				break;
			case DW_CFA_advance_loc2:
				m_buffer.read16();
				break;
			case DW_CFA_advance_loc4:
				m_buffer.read32();
				break;
			case DW_CFA_set_loc:
				// read a DWARF pointer with ptrEncoding
				m_buffer.readDwarfPointer(m_dwarfPtrEncoding);
				break;
			case DW_CFA_offset_extended:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset (*data align factor)
				m_buffer.readULEB128();
				break;
			case DW_CFA_restore_extended:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// loads up initial value
				break;
			case DW_CFA_undefined:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// marks register as unused
				break;
			case DW_CFA_same_value:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				break;
			case DW_CFA_register:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 reg2
				processNextReg(opaque, mapRegisterNumber);
				break;
			case DW_CFA_remember_state:
				break;
			case DW_CFA_restore_state:
				break;
			case DW_CFA_def_cfa:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_def_cfa_register:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				break;
			case DW_CFA_def_cfa_offset:
				// uleb128 - drop
				m_buffer.readULEB128();
				break;
			case DW_CFA_def_cfa_expression:
				// uleb128 length
				len = m_buffer.readULEB128();
				// add read value to m_pPos
				m_buffer.readBlock(size_t(len));
				break;
			case DW_CFA_expression:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 length
				len = m_buffer.readULEB128();
				// add read value to m_pPos
				m_buffer.readBlock(size_t(len));
				break;
			case DW_CFA_offset_extended_sf:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_def_cfa_sf:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_def_cfa_offset_sf:
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_val_offset:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_val_offset_sf:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_val_expression:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 length
				len = m_buffer.readULEB128();
				// add read value to m_pPos
				m_buffer.readBlock(size_t(len));
				break;
			//case DW_CFA_low_user: // not in Apple's libunwind
			//case DW_CFA_GNU_window_save: // not in Apple's libunwind
			case DW_CFA_GNU_args_size:
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			case DW_CFA_GNU_negative_offset_extended:
				// uleb128 reg
				processNextReg(opaque, mapRegisterNumber);
				// uleb128 offset
				m_buffer.readULEB128();
				break;
			//case DW_CFA_high_user: // not in Apple's libunwind
			default:
			{
				uint8_t op = instr & 0x3f;
				switch (instr & 0xC0)
				{
					case DW_CFA_offset:
					{
						// op is reg!
						uint8_t newInstr = processMixedInstr(opaque, mapRegisterNumber, instr);
						m_buffer.updateLast(newInstr);
						// uleb128 offset
						m_buffer.readULEB128();
						break;
					}
					case DW_CFA_advance_loc:
						break;
					case DW_CFA_restore:
					{
						// op is reg!
						uint8_t newInstr = processMixedInstr(opaque, mapRegisterNumber, instr);
						m_buffer.updateLast(newInstr);
						break;
					}
					default:
					{
						std::stringstream ss;
						ss << "Unhandled DWARF instruction " << std::hex << instr << std::dec;
						throw std::runtime_error(ss.str());
					}
				}
			}
		}
	}
}
