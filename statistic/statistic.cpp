#include "statistic.hpp"

namespace search {

double search::Stat::avg_token_length(std::vector<std::string> tokens) {
    int len_sum = 0;
    for (auto & token : tokens) {
        len_sum += (int)token.size();
    }

    return (double) len_sum / (double)(int)tokens.size();
}

}