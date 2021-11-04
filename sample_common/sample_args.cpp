// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.

#include "sample_args.h"

args_parser::args_parser(std::vector<arg_desc_t> args_desc) {
    for (auto& desc : args_desc) {
        m_args_map[desc.key] = desc;
    }
}

args_parser::args_parser(std::vector<arg_desc_t> args_desc, int argc, char* argv[]) {
    for (auto& desc : args_desc) {
        m_args_map[desc.key] = desc;
    }
    parse(argc, argv);
}

void args_parser::parse(int argc, char* argv[]) {
    for (int arg = 0; arg < argc; arg++) {
        std::string key = argv[arg];
        auto &desc = m_args_map[key];
        if (desc.type != ARG_TYPE_NONE) {
            char* value = argv[++arg];

            switch (desc.type) {
            case ARG_TYPE_INT:
                *((int*)desc.value) = std::stoi(value);
                break;
            case ARG_TYPE_TEXT:
                *((std::string**)desc.value) = new std::string(value);
                break;
            }
        }
    }
}

args_parser::~args_parser() {
    for (auto& it : m_args_map) {
        if (it.second.type == ARG_TYPE_TEXT) {
            std::string** val = static_cast<std::string**>(it.second.value);
            if (*val && val) delete *val;
        }
    }
}