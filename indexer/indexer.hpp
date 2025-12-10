#pragma once

#include <string>
#include <vector>

#include "../boolean_index/boolean_index.hpp"  

namespace search {

struct StoredDocument {
    std::string title;
    std::string description;
    std::string url;
    // сюда при желании можно добавить mongoId, price и т.п.
};

class Indexer {
public:
    Indexer() = default;

    // Добавить один документ (например, считанный из Mongo)
    // Возвращает присвоенный DocId
    DocId addDocument(const std::string& title,
                      const std::string& description,
                      const std::string& url);

    // Завершить индексацию (отсортировать и очистить postings)
    void finalize();

    // Доступ к булевому индексу (только чтение)
    const BooleanIndex& getIndex() const { return index_; }

    // Получить документ по DocId
    const StoredDocument& getDocument(DocId id) const { return documents_.at(id); }

    // Количество документов
    std::size_t size() const { return documents_.size(); }

private:
    BooleanIndex index_;
    std::vector<StoredDocument> documents_;
};

} // namespace search
