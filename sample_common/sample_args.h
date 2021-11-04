// Copyright © 2021 Vladislav Ovchinnikov. All rights reserved.
#include <string>
#include <vector>
#include <map>

enum arg_type_e {
    ARG_TYPE_NONE = 0,
    ARG_TYPE_INT,
    ARG_TYPE_TEXT
};

struct arg_desc_t {
    std::string key;
    std::string desc;
    arg_type_e  type;
    void*       value;
};

class args_parser {
public:
    args_parser(std::vector<arg_desc_t> args_desc, int argc, char* argv[]);
    args_parser(std::vector<arg_desc_t> args_desc);
    ~args_parser();
    void parse(int argc, char* argv[]);

private:
    std::map<std::string, arg_desc_t> m_args_map;
};