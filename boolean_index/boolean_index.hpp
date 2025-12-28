#pragma once

#include <string>
#include <vector>
#include <cstdint>

#include "../lematization/lematization.hpp"
#include "../tokenizer.hpp"

namespace search {

using DocId = std::uint32_t;
using PostingList = std::vector<DocId>;

class BooleanIndex {
public:
    explicit BooleanIndex(const std::string& index_file_path = "index.txt");

    void addDocument(DocId id, const std::string& text);

    void finalize();

    PostingList getPostings(const std::string& lemma) const;

    std::vector<DocId> andQuery(const std::vector<std::string>& terms) const;
    std::vector<DocId> orQuery(const std::vector<std::string>& terms) const;

private:
    std::string index_file_path_;

    std::vector<std::pair<std::string, DocId>> temp_pairs_;

    std::vector<std::pair<std::string, PostingList>> index_;

    Lematization lem_;
    Tokenizer tok_;

    void saveToFile() const;
    PostingList loadPostingsFromFile(const std::string& lemma) const;

    static PostingList intersect(const PostingList& a, const PostingList& b);
    static PostingList unify(const PostingList& a, const PostingList& b);
};

} // namespace search
