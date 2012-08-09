// Copyright 2011 Shinichiro Hamaji. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//   1. Redistributions of source code must retain the above copyright
//      notice, this list of  conditions and the following disclaimer.
//
//   2. Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials
//      provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY Shinichiro Hamaji ``AS IS'' AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL Shinichiro Hamaji OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
// USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.


#include "BindState.h"
#include "MachO.h"
#include "MachOImpl.h"
#include "log.h"
#include "leb.h"
#include <cstdio>
#include <cstring>

typedef long long ll;
typedef unsigned long long ull;

BindState::BindState(MachOImpl* mach0, bool is_weak0)
	: mach(mach0), ordinal(0), sym_name(NULL), type(BIND_TYPE_POINTER),
	addend(0), seg_index(0), seg_offset(0), is_weak(is_weak0)
{
}

void BindState::readBindOp(const uint8_t*& p)
{
	uint8_t op = *p & BIND_OPCODE_MASK;
	uint8_t imm = *p & BIND_IMMEDIATE_MASK;
	p++;
	
	LOGF("bind: op=%x imm=%d\n", op, imm);
	
	switch (op)
	{
	case BIND_OPCODE_DONE:
		break;

	case BIND_OPCODE_SET_DYLIB_ORDINAL_IMM:
		ordinal = imm;
		break;

	case BIND_OPCODE_SET_DYLIB_ORDINAL_ULEB:
		ordinal = uleb128(p);
		break;

	case BIND_OPCODE_SET_DYLIB_SPECIAL_IMM:
		if (imm == 0)
			ordinal = 0;
		else
			ordinal = BIND_OPCODE_MASK | imm;
		
		break;

	case BIND_OPCODE_SET_SYMBOL_TRAILING_FLAGS_IMM:
		sym_name = reinterpret_cast<const char*>(p);
		p += strlen(sym_name) + 1;
		LOGF("sym_name=%s\n", sym_name);
		break;

	case BIND_OPCODE_SET_TYPE_IMM:
		type = imm;
		break;

	case BIND_OPCODE_SET_ADDEND_SLEB:
		addend = sleb128(p);
		break;

	case BIND_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		seg_index = imm;
		seg_offset = uleb128(p);
		break;

	case BIND_OPCODE_ADD_ADDR_ULEB:
		seg_offset += uleb128(p);
		break;

	case BIND_OPCODE_DO_BIND:
		addBind();
		break;

	case BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB:
		LOGF("BIND_OPCODE_DO_BIND_ADD_ADDR_ULEB\n");
		addBind();
		seg_offset += uleb128(p);
		break;

	case BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED:
		LOGF("BIND_OPCODE_DO_BIND_ADD_ADDR_IMM_SCALED %d\n", (int)imm);
		addBind();
		seg_offset += imm * mach->m_ptrsize;
		break;

	case BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB: {
		uint64_t count = uleb128(p);
		uint64_t skip = uleb128(p);
		LOGF("BIND_OPCODE_DO_BIND_ULEB_TIMES_SKIPPING_ULEB %u %u\n",
			(unsigned)count, (unsigned)skip);
		for (uint64_t i = 0; i < count; i++)
		{
			addBind();
			seg_offset += skip;
		}
		break;
	}

	default:
		fprintf(stderr, "unknown op: %x\n", op);
	}
 }

void BindState::addBind()
{
	MachO::Bind* bind = new MachO::Bind();
	uint64_t vmaddr;
	if (mach->m_is64)
		vmaddr = mach->m_segments64[seg_index]->vmaddr;
	else
		vmaddr = mach->m_segments[seg_index]->vmaddr;

	LOGF("add bind! %s seg_index=%d seg_offset=%llu "
			"type=%d ordinal=%d addend=%lld vmaddr=%p is_weak=%d\n",
			sym_name, seg_index, (ull)seg_offset,
			type, ordinal, (ll)addend, (void*)(vmaddr + seg_offset), is_weak);
	bind->name = sym_name;
	bind->vmaddr = vmaddr + seg_offset;
	bind->addend = addend;
	bind->type = type;
	bind->ordinal = ordinal;
	bind->is_weak = is_weak;
	mach->m_binds.push_back(bind);

	seg_offset += mach->m_ptrsize;
}
