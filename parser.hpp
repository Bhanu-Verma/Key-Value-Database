#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <string_view>

namespace DB{
    std::vector<std::string> parse(const std::string& command);
};

#endif