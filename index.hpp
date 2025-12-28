#pragma once

#include <string>
#include <vector>

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

#include "mongo_client/mongo_client.hpp"
#include "indexer/indexer.hpp"
#include "mongo_utils.hpp"

namespace search {

inline void buildIndex(MongoClient& mongo,
                                Indexer& indexer,
                                const std::string& collection_name)
{
    auto coll = mongo.collection(collection_name);

    mongocxx::cursor cursor = coll.find({});

    for (auto&& doc : cursor) {
        bsoncxx::document::view view = doc;

        std::string title       = get_string_field(view, "title");
        std::string description = get_string_field(view, "description");
        std::string url         = get_string_field(view, "url");

        indexer.addDocument(title, description, url);
    }

    indexer.finalize();
}

}
