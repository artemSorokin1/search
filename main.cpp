#include <iostream>
#include "mongo_client/mongo_client.hpp"
#include "indexer/indexer.hpp"
#include "index.hpp"   
#include "tokenizer.hpp"

int main() {
    // 1. Создаём Mongo-клиент
    search::MongoClient mongo{"mongodb://localhost:27017", "db"};

    // 2. Создаём индексатор
    search::Indexer indexer;

    // 3. Строим индекс по коллекции, например "products"
    search::buildIndex(mongo, indexer, "items");

    std::cout << "Проиндексировано документов: " << indexer.size() << "\n";

    // 4. Пробуем поиск
    std::vector<std::string> query = {"датчик"};
    auto resultIds = indexer.getIndex().andQuery(query);

    search::Tokenizer t;
    auto res = t.tokenize("Датчик, частота вращения колеса, STELLOX 06-65305-SX (1 шт.)");
    for (auto & elem : res) {
        cout << elem << '\n';
    }

    std::cout << "Найдено документов: " << resultIds.size() << "\n";
    for (auto id : resultIds) {
        const auto& doc = indexer.getDocument(id);
        std::cout << "DocId: " << id << "\n";
        std::cout << "  Title: " << doc.title << "\n";
        std::cout << "  URL:   " << doc.url << "\n";
        std::cout << "--------------------\n";
    }

    return 0;
}
