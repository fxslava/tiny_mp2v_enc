#include "decoder.h"

frame_c::frame_c(int width, int height, int chroma_format) {
    m_stride = (width + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
    m_width = width;
    m_height = height;

    switch (chroma_format) {
    case chroma_format_420:
        m_chroma_stride = ((m_stride >> 1) + CACHE_LINE - 1) & ~(CACHE_LINE - 1);
        m_chroma_width = m_width >> 1;
        m_chroma_height = m_height >> 1;
        break;
    case chroma_format_422:
        m_chroma_stride = m_stride;
        m_chroma_width = m_width;
        m_chroma_height = m_height >> 1;
        break;
    case chroma_format_444:
        m_chroma_stride = m_stride;
        m_chroma_width = m_width;
        m_chroma_height = m_height;
        break;
    }

    luma_plane = new uint8_t[m_height * m_stride];
    chroma_planes[0] = new uint8_t[m_chroma_height * m_chroma_stride];
    chroma_planes[1] = new uint8_t[m_chroma_height * m_chroma_stride];
}

frame_c::~frame_c() {
    if (luma_plane) delete[] luma_plane;
    if (chroma_planes[0]) delete[] chroma_planes[0];
    if (chroma_planes[1]) delete[] chroma_planes[1];
}

bool mp2v_decoder_c::decoder_init(decoder_config_t* config) {
    int pool_size = config->frames_pool_size;
    int width = config->width;
    int height = config->height;
    int chroma_format = config->chroma_format;

    for (int i = 0; i < pool_size; i++)
        frames_pool.push_back(new frame_c(width, height, chroma_format));

    return true;
}

bool mp2v_decoder_c::decode_slice(slice_c& slice) {
    for (auto& mb : slice.macroblocks)
        for (int i = 0; i < slice.block_count; i++) {
            int16_t QF[64];
        }
    return true;
}

bool mp2v_decoder_c::decode(picture_c* pic) {
    for (auto& slice : pic->m_slices)
        decode_slice(slice);
    return true;
}