#include "bitstream_writer.h"
#include <stdint.h>

template <typename T, int count>
static void local_write_array(bitstream_writer_c* bs, T* dst) {
    for (int i = 0; i < count; i++) {
        bs->write_bits(dst[i], sizeof(T) * 8);
    }
}
