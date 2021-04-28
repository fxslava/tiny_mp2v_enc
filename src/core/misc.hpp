#include "bitstream.h"
#include "mp2v_vlc.h"
#include <stdint.h>

static uint8_t local_find_start_code(bitstream_reader_i* bs) {
    bs->seek_pattern(vlc_start_code.value, vlc_start_code.len);
    return (uint8_t)(bs->get_next_bits(32) & 0xff);
}

static uint8_t local_next_start_code(bitstream_reader_i* bs) {
    uint32_t full_start_code = bs->get_next_bits(32);
    return full_start_code & 0xff;
}

static uint8_t local_read_start_code(bitstream_reader_i* bs) {
    uint32_t full_start_code = bs->read_next_bits(32);
    return full_start_code & 0xff;
}

template <typename T, int count>
static void local_copy_array(bitstream_reader_i* bs, T* dst) {
    for (int i = 0; i < count; i++) {
        dst[i] = bs->read_next_bits(sizeof(T) * 8);
    }
}