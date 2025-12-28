#include "indexer.hpp"

namespace search {

DocId Indexer::addDocument(const std::string& title,
                           const std::string& description,
                           const std::string& url)
{
    DocId id = static_cast<DocId>(documents_.size());

    StoredDocument doc;
    doc.title = title;
    doc.description = description;
    doc.url = url;
    documents_.push_back(std::move(doc));

    std::string text = title + " " + description;

    index_.addDocument(id, text);

    return id;
}

void Indexer::finalize() {
    index_.finalize();
}

}
