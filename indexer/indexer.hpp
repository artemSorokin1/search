#pragma once

#include <string>
#include <vector>

#include "../boolean_index/boolean_index.hpp"  

namespace search {

struct StoredDocument {
    std::string title;
    std::string description;
    std::string url;
};

class Indexer {
public:
    Indexer() = default;

    DocId addDocument(const std::string& title,
                      const std::string& description,
                      const std::string& url);

    void finalize();

    const BooleanIndex& getIndex() const { return index_; }

    const StoredDocument& getDocument(DocId id) const { return documents_.at(id); }

    std::size_t size() const { return documents_.size(); }

private:
    BooleanIndex index_;
    std::vector<StoredDocument> documents_;
};

}
