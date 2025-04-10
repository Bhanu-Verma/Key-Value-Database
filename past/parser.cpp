#include "parser.hpp"
#include <sstream>
#include <algorithm>


std::vector<std::string> DB::parse(const std::string& command){
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