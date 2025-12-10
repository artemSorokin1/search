#pragma once

#include <string>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

namespace search {

class MongoClient {
public:
    MongoClient(const std::string& uri = "mongodb://localhost:27017",
                const std::string& db_name = "db");

    mongocxx::database db();

    mongocxx::collection collection(const std::string& name);

    const std::string& db_name() const noexcept { return db_name_; }

    std::vector<std::string> get_all_descriptions(const std::string& collection_name);

private:
    mongocxx::client client_;
    std::string db_name_;
};

} 
