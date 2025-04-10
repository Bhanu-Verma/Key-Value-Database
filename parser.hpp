#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <string>
#include <string_view>
#include <sstream>
#include <algorithm>

namespace DB{
    inline std::vector<std::string> parse(const std::string& command){
        std::stringstream commandStream{ command };
        std::vector<std::string> result{};
        std::string curr{};
        while (commandStream >> curr)
        {
            result.push_back(curr);
        }
        // Convert the first string to lowercase 
        std::transform(result[0].begin(), result[0].end(), result[0].begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }
};

#endif