#ifndef MACBINARY_H
#define MACBINARY_H
#include <stdint.h>
#include "../util/MSB.h"

// http://hackipedia.org/File%20formats/Containers/MacBinary%20(archive)/MacBinary%20III%20file%20format.html

#pragma pack(1)
struct MacBinaryHeader
{
	uint8_t old_ver;
	uint8_t filename_len;
	char filename[63];
	MSB<uint32_t> file_type, file_creator;
	uint8_t orig_finder_flags;
	uint8_t zero;
	MSB<uint16_t> vertical_pos, horizontal_pos;
	MSB<uint16_t> win_folder_id;
	uint8_t _protected, zero2;
	MSB<uint32_t> data_len, resource_len;
	MSB<uint32_t> created, last_modified;
	MSB<uint16_t> getinfo_comment_len;
	uint8_t finder_flags;
	MSB<uint32_t> signature; // 'mBIN'
	uint8_t script, ext_finder_flags, unused[8];
	MSB<uint32_t> total_len; // unused
	MSB<uint16_t> sec_header_len;
	uint8_t version_write, version_read;
	MSB<uint16_t> crc;
	char padding[2]; // pad to 128 bytes
};
#pragma pack()

static_assert(sizeof(MacBinaryHeader) == 128, "Invalid header length");

#ifdef FOR_MACBINARY
enum Command { ExtractDF, ExtractRF, PrintName };

void printHelp();
Command getCommand(const char* cmd);
void printName(std::ifstream& file, const MacBinaryHeader& hdr);
void extractRF(std::ifstream& file, const MacBinaryHeader& hdr, const char* outName);
void extractDF(std::ifstream& file, const MacBinaryHeader& hdr, const char* outName);
void extract(std::ifstream& file, uint32_t offset, uint32_t len, std::string outName);
void checkHeader(const MacBinaryHeader& hdr);
#endif

#endif

