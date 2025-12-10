#include "mongo_client.hpp"
#include <mongocxx/instance.hpp>
#include <mongocxx/uri.hpp>

namespace {

// Глобальный singleton instance, создаётся один раз при старте программы
mongocxx::instance global_mongo_instance{};

} // namespace

namespace search {

MongoClient::MongoClient(const std::string& uri, const std::string& db_name)
    : client_{mongocxx::uri{uri}},
      db_name_{db_name}
{
    // больше ничего не нужно, instance уже создан выше
}

mongocxx::database MongoClient::db() {
    return client_[db_name_];
}

mongocxx::collection MongoClient::collection(const std::string& name) {
    return client_[db_name_][name];
}

} // namespace search

