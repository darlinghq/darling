/*
This file is part of Darling.

Copyright (C) 2017 Lubos Dolezel
    
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

void* BXPatch4(void);
void* BXPatch5(void);
void* BXPatch5InPlace(void);
void* BXPatchFile(void);
void* CachePatch(void);
void* CachePatchInPlace(void);
void* PCompressFilter(void);
void* PCompressGetDecoderDescription(void);
void* PCompressGetDecoderKey(void);
void* PCompressGetEncoderDescription(void);
void* PCompressGetEncoderKey(void);
void* PKBOMCopierBomSysFree(void);
void* PKBOMCopierBomSysInit(void);
void* PKInstallServiceClientInterface(void);
void* PKInstallServiceInterface(void);
void* PKLogTracedMessage(void);
void* PKPayloadCopierBOMCopierCopyFileFinishedHandler(void);
void* PKPayloadCopierBOMCopierCopyFileStartedHandler(void);
void* PKPayloadCopierBOMCopierCopyFileUpdateHandler(void);
void* PKPayloadCopierBOMCopierFatalErrorHandler(void);
void* PKPayloadCopierBOMCopierFatalFileErrorHandler(void);
void* PKPayloadCopierBOMCopierFileConflictErrorHandler(void);
void* PKPayloadCopierBOMCopierFileErrorHandler(void);
void* PKSIPCurrentProcessCanModifySystemIntegrityProtectionFiles(void);
void* PKTrustLevelToNSString(void);
void* PackagePatch(void);
void* ParallelArchiveExtract(void);
void* ParallelArchiveGenerateMSUBOM(void);
void* ParallelArchiveOLDWriterAddEntry(void);
void* ParallelArchiveOLDWriterCreate(void);
void* ParallelArchiveOLDWriterDestroy(void);
void* ParallelArchiveRead(void);
void* ParallelArchiveSearch(void);
void* ParallelArchiveSort(void);
void* ParallelArchiveWriteDirContents(void);
void* ParallelArchiveWriteEntryData(void);
void* ParallelArchiveWriteEntryHeader(void);
void* ParallelArchiveWriterCreate(void);
void* ParallelArchiveWriterDestroy(void);
void* ParallelCompressionDecode(void);
void* ParallelCompressionEncode(void);
void* ParallelCompressionFileClose(void);
void* ParallelCompressionFileOpen(void);
void* ParallelCompressionFileRead(void);
void* ParallelCompressionFileSeek(void);
void* ParallelCompressionFileWrite(void);
void* ParallelPatchApplyPatches(void);
void* ParallelPatchExtractPayload(void);
void* ParallelPatchGetAssetsSize(void);
void* ParallelPatchIdentifyAssets(void);
void* ParallelPatchLinkFiles(void);
void* ParallelPatchRemoveFiles(void);
void* Shove(void);
void* _ShoveOne(void);
void* _blockCompressionReadHandler(void);
void* _blockCompressionWriteHandler(void);
void* _chunkNumberForChunkSizeAndResumptionOffset(void);
void* _parallelThreadCount(void);
void* compression_decode_buffer(void);
void* compression_decode_scratch_buffer_size(void);
void* compression_encode_buffer(void);
void* compression_encode_scratch_buffer_size(void);
void* compression_stream_build_shared_dict(void);
void* compression_stream_destroy(void);
void* compression_stream_get_state_size(void);
void* compression_stream_identify_algorithm(void);
void* compression_stream_init(void);
void* compression_stream_init_with_options(void);
void* compression_stream_is_state_valid(void);
void* compression_stream_op_data_size(void);
void* compression_stream_process(void);
void* lz4_decode_asm(void);
void* lzvn_decode_buffer(void);
