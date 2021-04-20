// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include <vector>
#include "mp2v_hdr.h"
#include "api/bitstream.h"

enum extension_after_code_e {
    after_sequence_extension = 0,
    after_group_of_picture_header,
    after_picture_coding_extension
};

class video_sequence_c {
public:
    std::vector<video_sequence_t> context;
public:
    video_sequence_c(bitstream_reader_i* bitstream) : 
        m_bs(bitstream), 
        context(1) {
        m_sequence_header = &context[0].sequence_header;
        m_sequence_extension = &context[0].sequence_extension;
    };
    bool parse();
    bool parse_sequence_header();
    bool parse_sequence_extension();
    bool parse_extension_and_user_data(extension_after_code_e after_code);
    bool parse_extension_data(extension_after_code_e after_code);

private:
    bitstream_reader_i* m_bs;
    sequence_header_t* m_sequence_header;
    sequence_extension_t* m_sequence_extension;
};

class mp2v_parser_c {
public:
    mp2v_parser_c(bitstream_reader_i* bitstream) : 
        m_bs(bitstream), 
        m_video_sequence(bitstream) {};
    bool parse();

private:
    bitstream_reader_i* m_bs;
    video_sequence_c m_video_sequence;
};