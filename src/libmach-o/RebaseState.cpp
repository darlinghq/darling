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

#include "RebaseState.h"
#include "leb.h"
#include "log.h"
#include "MachOImpl.h"
#include <cstdio>

typedef unsigned long long ull;

RebaseState::RebaseState(MachOImpl* mach0)
	: mach(mach0), type(0), seg_index(0), seg_offset(0)
{
}

bool RebaseState::readRebaseOp(const uint8_t*& p)
{
	uint8_t op = *p & REBASE_OPCODE_MASK;
	uint8_t imm = *p & REBASE_IMMEDIATE_MASK;
	p++;
	
	switch (op)
	{
	case REBASE_OPCODE_DONE:
		return false;

	case REBASE_OPCODE_SET_TYPE_IMM:
		type = imm;
		break;

	case REBASE_OPCODE_SET_SEGMENT_AND_OFFSET_ULEB:
		seg_index = imm;
		seg_offset = uleb128(p);
		break;

	case REBASE_OPCODE_ADD_ADDR_ULEB:
		seg_offset += uleb128(p);
		break;

	case REBASE_OPCODE_ADD_ADDR_IMM_SCALED:
		seg_offset += imm * mach->m_ptrsize;
		break;

	case REBASE_OPCODE_DO_REBASE_IMM_TIMES:
		for (int i = 0; i < imm; i++)
			addRebase();
		break;

	case REBASE_OPCODE_DO_REBASE_ULEB_TIMES:
	{
		int count = uleb128(p);
		for (int i = 0; i < count; i++)
			addRebase();
		break;
	}

	case REBASE_OPCODE_DO_REBASE_ADD_ADDR_ULEB:
		addRebase();
		seg_offset += uleb128(p);
		break;

	case REBASE_OPCODE_DO_REBASE_ULEB_TIMES_SKIPPING_ULEB:
	{
		int count = uleb128(p);
		uint64_t skip = uleb128(p);
		for (int i = 0; i < count; i++)
		{
			addRebase();
			seg_offset += skip;
		}
		break;
	}

	default:
		fprintf(stderr, "unknown op: %x\n", op);
	}

	return true;
}

void RebaseState::addRebase()
{
	MachO::Rebase* rebase = new MachO::Rebase();
	uint64_t vmaddr;
	if (mach->m_is64)
		vmaddr = mach->m_segments64[seg_index]->vmaddr;
	else
		vmaddr = mach->m_segments[seg_index]->vmaddr;

	LOGF("add rebase! seg_index=%d seg_offset=%llu type=%d vmaddr=%p\n",
			seg_index, (ull)seg_offset, type, (void*)vmaddr);
	rebase->vmaddr = vmaddr + seg_offset;
	rebase->type = type;
	mach->m_rebases.push_back(rebase);

	seg_offset += mach->m_ptrsize;
}

