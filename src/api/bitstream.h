// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#pragma once

class bitstream_reader_i {
public:
    virtual uint32_t get_next_bits(int len) = 0;
    virtual uint32_t read_next_bits(int len) = 0;
    virtual bool seek_pattern(uint32_t pattern, int len) = 0;
    virtual void skip_bits(int len) = 0;
};