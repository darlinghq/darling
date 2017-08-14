#ifndef __COMPRESSION_H__
#define __COMPRESSION_H__

#include <stdint.h>
#include <os/base.h>
#include <sys/types.h>
#include <Availability.h>

#ifdef __cplusplus
extern "C" {
#endif

#if __has_feature(assume_nonnull)
    _Pragma("clang assume_nonnull begin")
#else
#   define __nullable
#endif

typedef enum {

    COMPRESSION_LZ4     = 0x100,
    COMPRESSION_ZLIB    = 0x205,
    COMPRESSION_LZMA    = 0x306,
    COMPRESSION_LZ4_RAW = 0x101,
    COMPRESSION_LZFSE    = 0x801,

} compression_algorithm;

extern size_t
compression_encode_scratch_buffer_size(compression_algorithm algorithm)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

extern size_t
compression_encode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

extern size_t
compression_decode_scratch_buffer_size(compression_algorithm algorithm)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

extern size_t
compression_decode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

typedef struct {
    uint8_t       * dst_ptr;
    size_t          dst_size;
    const uint8_t * src_ptr;
    size_t          src_size;
    void          * __nullable state;

} compression_stream;

typedef enum {
    COMPRESSION_STREAM_ENCODE = 0,
    COMPRESSION_STREAM_DECODE = 1,
  
} compression_stream_operation;

typedef enum {
    COMPRESSION_STREAM_FINALIZE = 0x0001,
} compression_stream_flags;

typedef enum {

    COMPRESSION_STATUS_OK     = 0,
    COMPRESSION_STATUS_ERROR  = -1,
    COMPRESSION_STATUS_END    = 1,
  
} compression_status;


extern compression_status
compression_stream_init(compression_stream * stream,
                        compression_stream_operation operation,
                        compression_algorithm algorithm)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

extern compression_status
compression_stream_process(compression_stream * stream,
                           int flags)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);

extern compression_status
compression_stream_destroy(compression_stream * stream)
__OSX_AVAILABLE_STARTING(__MAC_10_11, __IPHONE_9_0);
  
#if __has_feature(assume_nonnull)
  _Pragma("clang assume_nonnull end")
#endif
#ifdef __cplusplus
}
#endif

#endif

