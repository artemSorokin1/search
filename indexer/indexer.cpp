#include "indexer.hpp"

namespace search {

DocId Indexer::addDocument(const std::string& title,
                           const std::string& description,
                           const std::string& url)
{
    // 1. Новый DocId — это текущий размер массива документов
    DocId id = static_cast<DocId>(documents_.size());

    // 2. Сохраняем документ
    StoredDocument doc;
    doc.title = title;
    doc.description = description;
    doc.url = url;
    documents_.push_back(std::move(doc));

    // 3. Склеиваем текст для индексации
    std::string text = title + " " + description;

    // 4. Отдаём в BooleanIndex на индексацию
    index_.addDocument(id, text);

    return id;
}

void Indexer::finalize() {
    index_.finalize();
}

} 
