#include <iostream>
#include "mongo_client/mongo_client.hpp"
#include "indexer/indexer.hpp"
#include "index.hpp"   
#include "tokenizer.hpp"
#include "statistic/statistic.hpp"

int main() {
    search::MongoClient mongo{"mongodb://localhost:27017", "db"};

    search::Indexer indexer;
    search::Stat stat;

    stat.dumpLemmasFromMongo(mongo, "items", "lemmas.txt");
    search::buildIndex(mongo, indexer, "items");

    std::cout << "Проиндексировано документов: " << indexer.size() << "\n";

    std::vector<std::string> query = {"ruler"};
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
