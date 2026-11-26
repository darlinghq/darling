#include <stddef.h>

void *at_encoder_create(int output_type, int output_alpha,
                        int input_type, int input_alpha,
                        const void *background_color) {
    (void)output_type; (void)output_alpha;
    (void)input_type; (void)input_alpha;
    (void)background_color;
    return NULL;
}

int at_encoder_compress_texels(void *encoder, const void *src, void *dest,
                               float error_threshold, int flags) {
    (void)encoder; (void)src; (void)dest; (void)error_threshold; (void)flags;
    return -1;
}

int at_encoder_decompress_texels(void *encoder, const void *src, void *dest,
                                 int flags) {
    (void)encoder; (void)src; (void)dest; (void)flags;
    return -1;
}

unsigned long at_encoder_get_block_counts(void *encoder, unsigned long width,
                                          unsigned long height, unsigned long depth) {
    (void)encoder; (void)width; (void)height; (void)depth;
    return 0;
}
