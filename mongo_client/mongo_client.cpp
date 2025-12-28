#include "mongo_client.hpp"
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

namespace {

mongocxx::instance global_mongo_instance{};

}

namespace search {

MongoClient::MongoClient(const std::string& uri, const std::string& db_name)
    : client_{mongocxx::uri{uri}},
      db_name_{db_name}
{
}

mongocxx::database MongoClient::db() {
    return client_[db_name_];
}

mongocxx::collection MongoClient::collection(const std::string& name) {
    return client_[db_name_][name];
}

}

