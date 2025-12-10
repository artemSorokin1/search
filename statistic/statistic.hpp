#pragma once
#include <vector>
#include <string>

namespace search {

    class Stat {
    public:
        double avg_token_length(std::vector<std::string> tokens);
    };

}