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


#include <stdlib.h>
#include <stdio.h>

static int verbose = 0;
__attribute__((constructor)) static void initme(void) {
    verbose = getenv("STUB_VERBOSE") != NULL;
}
void* BXPatch4(void) { if (verbose) puts("STUB: BXPatch4 called"); return NULL; }
void* BXPatch5(void) { if (verbose) puts("STUB: BXPatch5 called"); return NULL; }
void* BXPatch5InPlace(void) { if (verbose) puts("STUB: BXPatch5InPlace called"); return NULL; }
void* BXPatchFile(void) { if (verbose) puts("STUB: BXPatchFile called"); return NULL; }
void* CachePatch(void) { if (verbose) puts("STUB: CachePatch called"); return NULL; }
void* CachePatchInPlace(void) { if (verbose) puts("STUB: CachePatchInPlace called"); return NULL; }
void* PCompressFilter(void) { if (verbose) puts("STUB: PCompressFilter called"); return NULL; }
void* PCompressGetDecoderDescription(void) { if (verbose) puts("STUB: PCompressGetDecoderDescription called"); return NULL; }
void* PCompressGetDecoderKey(void) { if (verbose) puts("STUB: PCompressGetDecoderKey called"); return NULL; }
void* PCompressGetEncoderDescription(void) { if (verbose) puts("STUB: PCompressGetEncoderDescription called"); return NULL; }
void* PCompressGetEncoderKey(void) { if (verbose) puts("STUB: PCompressGetEncoderKey called"); return NULL; }
void* PKBOMCopierBomSysFree(void) { if (verbose) puts("STUB: PKBOMCopierBomSysFree called"); return NULL; }
void* PKBOMCopierBomSysInit(void) { if (verbose) puts("STUB: PKBOMCopierBomSysInit called"); return NULL; }
void* PKInstallServiceClientInterface(void) { if (verbose) puts("STUB: PKInstallServiceClientInterface called"); return NULL; }
void* PKInstallServiceInterface(void) { if (verbose) puts("STUB: PKInstallServiceInterface called"); return NULL; }
void* PKLogTracedMessage(void) { if (verbose) puts("STUB: PKLogTracedMessage called"); return NULL; }
void* PKPayloadCopierBOMCopierCopyFileFinishedHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierCopyFileFinishedHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierCopyFileStartedHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierCopyFileStartedHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierCopyFileUpdateHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierCopyFileUpdateHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierFatalErrorHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierFatalErrorHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierFatalFileErrorHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierFatalFileErrorHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierFileConflictErrorHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierFileConflictErrorHandler called"); return NULL; }
void* PKPayloadCopierBOMCopierFileErrorHandler(void) { if (verbose) puts("STUB: PKPayloadCopierBOMCopierFileErrorHandler called"); return NULL; }
void* PKSIPCurrentProcessCanModifySystemIntegrityProtectionFiles(void) { if (verbose) puts("STUB: PKSIPCurrentProcessCanModifySystemIntegrityProtectionFiles called"); return NULL; }
void* PKTrustLevelToNSString(void) { if (verbose) puts("STUB: PKTrustLevelToNSString called"); return NULL; }
void* PackagePatch(void) { if (verbose) puts("STUB: PackagePatch called"); return NULL; }
void* ParallelArchiveExtract(void) { if (verbose) puts("STUB: ParallelArchiveExtract called"); return NULL; }
void* ParallelArchiveGenerateMSUBOM(void) { if (verbose) puts("STUB: ParallelArchiveGenerateMSUBOM called"); return NULL; }
void* ParallelArchiveOLDWriterAddEntry(void) { if (verbose) puts("STUB: ParallelArchiveOLDWriterAddEntry called"); return NULL; }
void* ParallelArchiveOLDWriterCreate(void) { if (verbose) puts("STUB: ParallelArchiveOLDWriterCreate called"); return NULL; }
void* ParallelArchiveOLDWriterDestroy(void) { if (verbose) puts("STUB: ParallelArchiveOLDWriterDestroy called"); return NULL; }
void* ParallelArchiveRead(void) { if (verbose) puts("STUB: ParallelArchiveRead called"); return NULL; }
void* ParallelArchiveSearch(void) { if (verbose) puts("STUB: ParallelArchiveSearch called"); return NULL; }
void* ParallelArchiveSort(void) { if (verbose) puts("STUB: ParallelArchiveSort called"); return NULL; }
void* ParallelArchiveWriteDirContents(void) { if (verbose) puts("STUB: ParallelArchiveWriteDirContents called"); return NULL; }
void* ParallelArchiveWriteEntryData(void) { if (verbose) puts("STUB: ParallelArchiveWriteEntryData called"); return NULL; }
void* ParallelArchiveWriteEntryHeader(void) { if (verbose) puts("STUB: ParallelArchiveWriteEntryHeader called"); return NULL; }
void* ParallelArchiveWriterCreate(void) { if (verbose) puts("STUB: ParallelArchiveWriterCreate called"); return NULL; }
void* ParallelArchiveWriterDestroy(void) { if (verbose) puts("STUB: ParallelArchiveWriterDestroy called"); return NULL; }
void* ParallelCompressionDecode(void) { if (verbose) puts("STUB: ParallelCompressionDecode called"); return NULL; }
void* ParallelCompressionEncode(void) { if (verbose) puts("STUB: ParallelCompressionEncode called"); return NULL; }
void* ParallelCompressionFileClose(void) { if (verbose) puts("STUB: ParallelCompressionFileClose called"); return NULL; }
void* ParallelCompressionFileOpen(void) { if (verbose) puts("STUB: ParallelCompressionFileOpen called"); return NULL; }
void* ParallelCompressionFileRead(void) { if (verbose) puts("STUB: ParallelCompressionFileRead called"); return NULL; }
void* ParallelCompressionFileSeek(void) { if (verbose) puts("STUB: ParallelCompressionFileSeek called"); return NULL; }
void* ParallelCompressionFileWrite(void) { if (verbose) puts("STUB: ParallelCompressionFileWrite called"); return NULL; }
void* ParallelPatchApplyPatches(void) { if (verbose) puts("STUB: ParallelPatchApplyPatches called"); return NULL; }
void* ParallelPatchExtractPayload(void) { if (verbose) puts("STUB: ParallelPatchExtractPayload called"); return NULL; }
void* ParallelPatchGetAssetsSize(void) { if (verbose) puts("STUB: ParallelPatchGetAssetsSize called"); return NULL; }
void* ParallelPatchIdentifyAssets(void) { if (verbose) puts("STUB: ParallelPatchIdentifyAssets called"); return NULL; }
void* ParallelPatchLinkFiles(void) { if (verbose) puts("STUB: ParallelPatchLinkFiles called"); return NULL; }
void* ParallelPatchRemoveFiles(void) { if (verbose) puts("STUB: ParallelPatchRemoveFiles called"); return NULL; }
void* Shove(void) { if (verbose) puts("STUB: Shove called"); return NULL; }
void* _ShoveOne(void) { if (verbose) puts("STUB: _ShoveOne called"); return NULL; }
void* _blockCompressionReadHandler(void) { if (verbose) puts("STUB: _blockCompressionReadHandler called"); return NULL; }
void* _blockCompressionWriteHandler(void) { if (verbose) puts("STUB: _blockCompressionWriteHandler called"); return NULL; }
void* _chunkNumberForChunkSizeAndResumptionOffset(void) { if (verbose) puts("STUB: _chunkNumberForChunkSizeAndResumptionOffset called"); return NULL; }
void* _parallelThreadCount(void) { if (verbose) puts("STUB: _parallelThreadCount called"); return NULL; }
void* compression_decode_buffer(void) { if (verbose) puts("STUB: compression_decode_buffer called"); return NULL; }
void* compression_decode_scratch_buffer_size(void) { if (verbose) puts("STUB: compression_decode_scratch_buffer_size called"); return NULL; }
void* compression_encode_buffer(void) { if (verbose) puts("STUB: compression_encode_buffer called"); return NULL; }
void* compression_encode_scratch_buffer_size(void) { if (verbose) puts("STUB: compression_encode_scratch_buffer_size called"); return NULL; }
void* compression_stream_build_shared_dict(void) { if (verbose) puts("STUB: compression_stream_build_shared_dict called"); return NULL; }
void* compression_stream_destroy(void) { if (verbose) puts("STUB: compression_stream_destroy called"); return NULL; }
void* compression_stream_get_state_size(void) { if (verbose) puts("STUB: compression_stream_get_state_size called"); return NULL; }
void* compression_stream_identify_algorithm(void) { if (verbose) puts("STUB: compression_stream_identify_algorithm called"); return NULL; }
void* compression_stream_init(void) { if (verbose) puts("STUB: compression_stream_init called"); return NULL; }
void* compression_stream_init_with_options(void) { if (verbose) puts("STUB: compression_stream_init_with_options called"); return NULL; }
void* compression_stream_is_state_valid(void) { if (verbose) puts("STUB: compression_stream_is_state_valid called"); return NULL; }
void* compression_stream_op_data_size(void) { if (verbose) puts("STUB: compression_stream_op_data_size called"); return NULL; }
void* compression_stream_process(void) { if (verbose) puts("STUB: compression_stream_process called"); return NULL; }
void* lz4_decode_asm(void) { if (verbose) puts("STUB: lz4_decode_asm called"); return NULL; }
void* lzvn_decode_buffer(void) { if (verbose) puts("STUB: lzvn_decode_buffer called"); return NULL; }
