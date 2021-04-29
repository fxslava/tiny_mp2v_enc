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
bool mp2v_decoder_c::decode() {
    return video_sequence_decoder.decode();
}

bool mp2v_sequence_decoder_c::decode() {
    return parse();
}

bool mp2v_sequence_decoder_c::parse_picture_data() {
    /* Decode sequence parameters*/
    mp2v_decoded_picture_c pic(m_bs, this);
    pic.block_count = block_count_tbl[m_sequence_extension.chroma_format];

    parse_picture_header(m_bs, pic.m_picture_header);
    parse_picture_coding_extension(m_bs, pic.m_picture_coding_extension);
    parse_extension_and_user_data(after_picture_coding_extension, &pic);
    pic.decode();

    m_pictures.push_back(pic);
    m_pictures_queue.push(&m_pictures.back());
    return true;
}

bool mp2v_decoded_picture_c::decode() {
    return parse_picture();
}

bool mp2v_decoded_picture_c::parse_slice() {
    mp2v_decoded_slice_c slice(m_bs, this);
    slice.init_slice();
    slice.decode();
    m_slices.push_back(slice);
    return true;
}

bool mp2v_decoded_slice_c::decode() {
    return parse_slice();
}

bool mp2v_decoded_slice_c::decode_blocks(mb_data_t& mb_data) {
    macroblocks.push_back(mb_data);
    return true;
}