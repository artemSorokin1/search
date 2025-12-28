#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

#include "../mongo_client/mongo_client.hpp"
#include "../lematization/lematization.hpp"
#include "../tokenizer.hpp"
#include "../mongo_utils.hpp"

namespace search {

class Stat {
public:
    double avg_token_length(const std::vector<std::string>& tokens) const;

    void dumpLemmasFromMongo(MongoClient& mongo,
                             const std::string& collection_name,
                             const std::string& out_path) const;
};

} // namespace search
