/* -*- mode: C++; c-basic-offset: 4; tab-width: 4 -*-
 *
 * Copyright (c) 2008-2010 Apple Inc. All rights reserved.
 *
 * @APPLE_LICENSE_HEADER_START@
 *
 * This file contains Original Code and/or Modifications of Original Code
 * as defined in and that are subject to the Apple Public Source License
 * Version 2.0 (the 'License'). You may not use this file except in
 * compliance with the License. Please obtain a copy of the License at
 * http://www.opensource.apple.com/apsl/ and read it before using this
 * file.
 *
 * The Original Code and all software distributed under the License are
 * distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
 * EXPRESS OR IMPLIED, AND APPLE HEREBY DISCLAIMS ALL SUCH WARRANTIES,
 * INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR NON-INFRINGEMENT.
 * Please see the License for the specific language governing rights and
 * limitations under the License.
 *
 * @APPLE_LICENSE_HEADER_END@
 */

/*
 //This is the exposed iterface for the Trie template
 //TODO: add erase methods
 //TODO: re-enable iterators

 template <typename V>
 struct Trie {
	struct Entry
	{
    std::string		name;
    V				info;

    Entry(void) {}
    Entry(const std::string& N, V I) : name(N), info(I) {}
  };

 struct const_iterator : std::iterator<std::bidirectional_iterator_tag, const Entry>;

 const_iterator begin() const;
 const_iterator end() const;

 Trie(void);
 Trie(const uint8_t* start, const uint8_t* end);
 Trie(const std::vector<Entry>& entries);

 void emit(std::vector<uint8_t>& output);
 */


#ifndef __TRIE__
#define __TRIE__
#define TRIE_DEBUG (0)

#include <algorithm>
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <iterator>

#include <mach-o/loader.h>

#if __cplusplus <= 201103L
namespace std {
	template<typename T, typename... Args>
	std::unique_ptr<T> make_unique(Args&&... args)
	{
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
}
#endif

namespace TrieUtils {

static void append_uleb128(uint64_t value, std::vector<uint8_t>& out) {
	uint8_t byte;
	do {
		byte = value & 0x7F;
		value &= ~0x7F;
		if ( value != 0 )
			byte |= 0x80;
		out.push_back(byte);
		value = value >> 7;
	} while( byte >= 0x80 );
}

static void append_string(std::string str, std::vector<uint8_t>& out) {
	for(char& c : str)
		out.push_back(c);
	out.push_back('\0');
}

static unsigned int	uleb128_size(uint64_t value) {
	uint32_t result = 0;
	do {
		value = value >> 7;
		++result;
	} while ( value != 0 );
	return result;
}

static
inline bool parse_uleb128(const uint8_t*& p, const uint8_t* end, uint64_t& result) {
	result = 0;
	int		 bit = 0;
	do {
		if (p == end)
			return false; // malformed uleb128 extends beyond input
		uint64_t slice = *p & 0x7f;

		if (bit >= 64 || slice << bit >> bit != slice)
			return false; // malformed
		else {
			result |= (slice << bit);
			bit += 7;
		}
	}
	while (*p++ & 0x80);
	return true;
}
};

template <typename V>
struct Trie {
	uint32_t count;
	uint32_t nodeCount;

	struct Entry
	{
		std::string		name;
		V				info;

		Entry(void) {}
		Entry(const std::string& N, V I) : name(N), info(I) {}
	};

	Trie(const std::vector<Entry>& entries) : count(0), nodeCount(1) {
		// make nodes for all exported symbols
		for (auto& entry : entries) {
			addEntry(entry);
		}
	}

	void emit(std::vector<uint8_t>& output) {
		// create vector of nodes
		std::vector<Node*> orderedNodes;
		orderedNodes.reserve(nodeCount);
		orderTrie(&root, orderedNodes);

		// assign each node in the vector an offset in the trie stream, iterating until all uleb128 sizes have stabilized
		bool more;
		do {
			uint32_t offset = 0;
			more = false;
			for (auto& node : orderedNodes) {
				if (node->updateOffset(offset)) {
					more = true;
				}
			}
		} while ( more );

		// create trie stream
		for (auto& node : orderedNodes) {
			node->appendToStream(output);
		}
	}

	static
	inline bool parseTrie(const uint8_t* start, const uint8_t* end, std::vector<Entry>& output)
	{
		// empty trie has no entries
		if ( start == end )
			return false;
		char cummulativeString[32768];
		std::vector<EntryWithOffset> entries;
		if ( !processExportNode(start, start, end, cummulativeString, 0, entries) )
			return false;
		// to preserve tie layout order, sort by node offset
		std::sort(entries.begin(), entries.end());
		// copy to output
		output.reserve(entries.size());
		for (auto& entryWithOffset : entries) {
			output.push_back(entryWithOffset.entry);
		}
		return true;
	}

private:
	struct Node
	{
		//This needs to be a map to unsure deterministic ordering of tries.
		std::map<std::string,std::unique_ptr<Node> > fChildren;
		bool				fIsTerminal;
		uint32_t			fTrieOffset;
		V					fInfo;

		Node(void) : fIsTerminal(false), fTrieOffset(0) {}
		Node(V v) :fInfo(v), fIsTerminal(true), fTrieOffset(0) {}
		Node(Node&&) = default;

		// byte for terminal node size in bytes, or 0x00 if not terminal node
		// teminal node (uleb128 flags, uleb128 addr [uleb128 other])
		// byte for child node count
		//  each child: zero terminated substring, uleb128 node offset
		bool updateOffset(uint32_t& offset) {
			uint32_t nodeSize = 1; // length of export info when no export info
			if ( fIsTerminal ) {
				nodeSize = fInfo.encodedSize();
				// do have export info, overall node size so far is uleb128 of export info + export info
				nodeSize += TrieUtils::uleb128_size(nodeSize);
			}
			// add children
			++nodeSize; // byte for count of chidren

			for (auto &edge : fChildren) {
				nodeSize += edge.first.length() + 1 + TrieUtils::uleb128_size(edge.second->fTrieOffset);
			}

			bool result = (fTrieOffset != offset);
			fTrieOffset = offset;
			//fprintf(stderr, "updateOffset %p %05d %s\n", this, fTrieOffset, fCummulativeString);
			offset += nodeSize;
			// return true if fTrieOffset was changed
			return result;
		}

		void appendToStream(std::vector<uint8_t>& out) {
			if ( fIsTerminal ) {
				fInfo.appendToStream(out);
			}
			else {
				// no export info uleb128 of zero is one byte of zero
				out.push_back(0);
			}
			// write number of children
			out.push_back(fChildren.size());
			// write each child
			for (auto &edge : fChildren) {
				TrieUtils::append_string(edge.first, out);
				TrieUtils::append_uleb128(edge.second->fTrieOffset, out);
			}
		}
	};

	Node root;

	struct EntryWithOffset
	{
		uintptr_t		nodeOffset;
		Entry			entry;

		bool operator<(const EntryWithOffset& other) const { return ( nodeOffset < other.nodeOffset ); }
	};

	void addEntry(const std::string& fullStr, std::string::const_iterator start, V v) {
		Node *currentNode = &root;
		bool done = false;

		while (!done && !currentNode->fChildren.empty() ) {
			done = true;

			for (auto &entry : currentNode->fChildren) {
				auto res = std::mismatch(entry.first.begin(), entry.first.end(), start);

				if (res.first ==  entry.first.end()) {
					//Matched a full edge, go down it
					done = false;
					currentNode = entry.second.get();
					start = res.second;
					break;
				} else if (res.first != entry.first.begin()) {
					// found a common substring, splice in new node
					//  was A -> C,  now A -> B -> C

					//Build the new strings
					std::string abEdgeStr(entry.first.begin(), res.first);
					std::string bcEdgeStr(res.first, entry.first.end());

					//Copy out the exist node and delete it from the currentNode
					std::unique_ptr<Node> nodeC;
					std::swap(nodeC, entry.second);
					currentNode->fChildren.erase(entry.first);

					//Build the new node and insert it
					std::unique_ptr<Node> nodeB = std::make_unique<Node>();
					Node *newNode = nodeB.get();

					nodeB->fChildren.insert(std::make_pair(bcEdgeStr, std::move(nodeC)));
					currentNode->fChildren.insert(std::make_pair(abEdgeStr, std::move(nodeB)));

					currentNode = newNode;
					start = res.second;
					++nodeCount;
					break;
				}
			}
		}

		// no commonality with any existing child, make a new edge that is this whole string
		std::string edgeStr(start, fullStr.end());
		v.willInsertAs(fullStr);

		if (edgeStr.empty()) {
			currentNode->fIsTerminal = true;
			currentNode->fInfo = v;
		} else {
			currentNode->fChildren.emplace(edgeStr, std::make_unique<Node>(v));
			++nodeCount;
		}
		++count;
	}

	void addEntry(Entry entry) {
		addEntry(entry.name, entry.name.begin(), entry.info);
	}

#if TRIE_DEBUG
	void printTrie(Node& node, std::string cummulativeString) {
		if (node.fTerminal) {
			printf("%s: \n", cummulativeString.c_str());
		}
		for (auto &edge : node.fChildren) {
			printTrie(*edge.second, cummulativeString+edge.first);
		}
	}

public:
	void printTrie(void) {
		printTrie(root, "");
	}
private:
#endif

	static inline bool processExportNode(const uint8_t* const start, const uint8_t* p, const uint8_t* const end,
										 char* cummulativeString, int curStrOffset,
										 std::vector<EntryWithOffset>& output)
	{
		if ( p >= end )
			return false;
		uint64_t terminalSize;
		if  ( !TrieUtils::parse_uleb128(p, end, terminalSize) )
			return false;
		const uint8_t* children = p + terminalSize;
		if ( children >= end )
			return false;
		if ( terminalSize != 0 ) {
			EntryWithOffset e;
			e.nodeOffset = p-start;
			e.entry.name = cummulativeString;
			e.entry.info.loadData(p,end);
			output.push_back(e);
		}
		const uint8_t childrenCount = *children++;
		const uint8_t* s = children;
		for (uint8_t i=0; i < childrenCount; ++i) {
			int edgeStrLen = 0;
			while (*s != '\0') {
				cummulativeString[curStrOffset+edgeStrLen] = *s++;
				++edgeStrLen;
			}
			cummulativeString[curStrOffset+edgeStrLen] = *s++;
			uint64_t childNodeOffet;
			if ( !TrieUtils::parse_uleb128(s, end, childNodeOffet) )
				return false;
			if ( !processExportNode(start, start+childNodeOffet, end, cummulativeString, curStrOffset+edgeStrLen, output) )
				return false;
		}
		return true;
	}

	void orderTrie(Node* node, std::vector<Node*>& orderedNodes) {
		orderedNodes.push_back(node);
		for (auto &edge : node->fChildren) {
			orderTrie(edge.second.get(), orderedNodes);
		}
	}
}; // struct Trie

struct ExportInfo {
	uint64_t		address;
	uint64_t		flags;
	uint64_t		other;
	std::string		importName;

	ExportInfo() : address(0), flags(0), other(0)  { }

	uint32_t encodedSize(void) {
		uint32_t size = 0;
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
			size = TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(other); // ordinal
			if ( !importName.empty() )
				size += importName.length();
			++size; // trailing zero in imported name
		}
		else {
			size = TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(address);
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				size += TrieUtils::uleb128_size(other);
		}
		return size;
	}

	void appendToStream(std::vector<uint8_t>& out) {
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
			if ( !importName.empty() ) {
				// nodes with re-export info: size, flags, ordinal, string
				uint32_t nodeSize = (uint32_t)(TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(other) + importName.length() + 1);
				out.push_back(nodeSize);
				TrieUtils::append_uleb128(flags, out);
				TrieUtils::append_uleb128(other, out);
				TrieUtils::append_string(importName, out);
			}
			else {
				// nodes with re-export info: size, flags, ordinal, empty-string
				uint32_t nodeSize = TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(other) + 1;
				out.push_back(nodeSize);
				TrieUtils::append_uleb128(flags, out);
				TrieUtils::append_uleb128(other, out);
				out.push_back(0);
			}
		}
		else if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER ) {
			// nodes with export info: size, flags, address, other
			uint32_t nodeSize = TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(address) + TrieUtils::uleb128_size(other);
			out.push_back(nodeSize);
			TrieUtils::append_uleb128(flags, out);
			TrieUtils::append_uleb128(address, out);
			TrieUtils::append_uleb128(other, out);
		}
		else {
			// nodes with export info: size, flags, address
			uint32_t nodeSize = TrieUtils::uleb128_size(flags) + TrieUtils::uleb128_size(address);
			out.push_back(nodeSize);
			TrieUtils::append_uleb128(flags, out);
			TrieUtils::append_uleb128(address, out);
		}
	}

	void loadData(const uint8_t* p, const uint8_t* const end) {
		TrieUtils::parse_uleb128(p, end, flags);
		if ( flags & EXPORT_SYMBOL_FLAGS_REEXPORT ) {
			TrieUtils::parse_uleb128(p, end, other); // dylib ordinal
			importName = (char*)p;
		}
		else {
			TrieUtils::parse_uleb128(p, end, address);
			if ( flags & EXPORT_SYMBOL_FLAGS_STUB_AND_RESOLVER )
				TrieUtils::parse_uleb128(p, end, other);
		}
	}

	void willInsertAs(const std::string& name) {
		// Symbols re-exported under the same name do not need an explict import name, delete it to save space
		if ((name == importName) ) {
			importName = "";
		}
	}
};

typedef Trie<ExportInfo> ExportInfoTrie;


// Used by accelerator tables in dyld shared cache
struct DylibIndex {
	uint32_t		index;

	DylibIndex() : index(0) {}
	DylibIndex(uint32_t i) : index(i) {}

	uint32_t encodedSize(void) {
		return TrieUtils::uleb128_size(index);
	}
	
	void appendToStream(std::vector<uint8_t>& out) {
        uint32_t nodeSize = TrieUtils::uleb128_size(index);
        out.push_back(nodeSize);
        TrieUtils::append_uleb128(index, out);
	}
	
	void loadData(const uint8_t* p, const uint8_t* const end) {
		uint64_t temp;
		TrieUtils::parse_uleb128(p, end, temp);
		index = (uint32_t)temp;
	}
	
	void willInsertAs(const std::string& name) {
	}
};
typedef Trie<DylibIndex> DylibIndexTrie;


#endif	// __TRIE__


