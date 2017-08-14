#include <compression.h>

size_t
compression_encode_scratch_buffer_size(compression_algorithm algorithm) {
    return 0;
}

size_t
compression_encode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm) {
    return 0;
}

size_t
compression_decode_scratch_buffer_size(compression_algorithm algorithm) {
    return 0;
}

size_t
compression_decode_buffer(uint8_t * __restrict dst_buffer, size_t dst_size,
                          const uint8_t * __restrict src_buffer, size_t src_size,
                          void * __restrict __nullable scratch_buffer,
                          compression_algorithm algorithm) {
    return 0;
}

compression_status
compression_stream_init(compression_stream * stream,
                        compression_stream_operation operation,
                        compression_algorithm algorithm) {
    return COMPRESSION_STATUS_ERROR;
}

compression_status
compression_stream_process(compression_stream * stream,
                           int flags) {
    return COMPRESSION_STATUS_ERROR;
}

compression_status
compression_stream_destroy(compression_stream * stream) {
    return COMPRESSION_STATUS_ERROR;
}

