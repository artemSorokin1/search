#pragma once

#include <string>
#include <vector>

#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>          // ВАЖНО: чтобы были k_utf8 и get_utf8

#include "mongo_client/mongo_client.hpp"
#include "indexer/indexer.hpp"        // или как у тебя называется Indexer

namespace search {

// Аккуратный helper: безопасно достаём строку из BSON-документа
inline std::string get_string_field(const bsoncxx::document::view& doc,
                                    const std::string& field_name)
{
    auto el = doc[field_name];
    if (!el) {
        return {};
    }

    if (el.type() != bsoncxx::type::k_string) {
        return {};
    }

    // value — это string_view-like (bsoncxx::stdx::string_view)
    auto sv = el.get_string().value;
    return std::string{sv.data(), sv.size()};
}

// Построение индекса из коллекции MongoDB
inline void buildIndex(MongoClient& mongo,
                                Indexer& indexer,
                                const std::string& collection_name)
{
    auto coll = mongo.collection(collection_name);

    mongocxx::cursor cursor = coll.find({});

    for (auto&& doc : cursor) {
        // doc здесь обычно bsoncxx::document::view_or_value
        // не вызываем doc.view(), просто приводим к view
        bsoncxx::document::view view = doc;

        std::string title       = get_string_field(view, "title");
        std::string description = get_string_field(view, "description");
        std::string url         = get_string_field(view, "url");

        // Если в коллекции нет title/url, можно оставить пустыми
        // или подставить что-то ещё

        indexer.addDocument(title, description, url);
    }

    indexer.finalize();
}

} // namespace search
