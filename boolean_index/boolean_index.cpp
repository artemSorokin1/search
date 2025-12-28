#include "boolean_index.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>

namespace search {

BooleanIndex::BooleanIndex(const std::string& index_file_path)
    : index_file_path_(index_file_path) {}

void BooleanIndex::addDocument(DocId id, const std::string& text) {
    auto tokens = tok_.tokenize(text);
    auto lemmas = lem_.lemmatizeTokens(tokens);

    std::sort(lemmas.begin(), lemmas.end());
    lemmas.erase(std::unique(lemmas.begin(), lemmas.end()), lemmas.end());

    for (auto& l : lemmas) {
        if (!l.empty()) {
            temp_pairs_.emplace_back(std::move(l), id);
        }
    }
}

void BooleanIndex::finalize() {
    index_.clear();
    if (temp_pairs_.empty()) {
        saveToFile();
        return;
    }

    std::sort(temp_pairs_.begin(), temp_pairs_.end(),
              [](const auto& A, const auto& B) {
                  if (A.first != B.first) return A.first < B.first;
                  return A.second < B.second;
              });

    temp_pairs_.erase(std::unique(temp_pairs_.begin(), temp_pairs_.end()),
                      temp_pairs_.end());

    for (const auto& [lemma, id] : temp_pairs_) {
        if (index_.empty() || index_.back().first != lemma) {
            index_.push_back({lemma, PostingList{}});
        }
        index_.back().second.push_back(id);
    }

    saveToFile();

    temp_pairs_.clear();
}

void BooleanIndex::saveToFile() const {
    std::ofstream out(index_file_path_);
    if (!out.is_open()) return;

    for (const auto& [lemma, postings] : index_) {
        out << lemma;
        for (DocId id : postings) {
            out << " " << id;
        }
        out << "\n";
    }
}

PostingList BooleanIndex::getPostings(const std::string& lemma) const {
    if (!index_.empty()) {
        auto it = std::lower_bound(
            index_.begin(), index_.end(), lemma,
            [](const auto& entry, const std::string& key) {
                return entry.first < key;
            });

        if (it != index_.end() && it->first == lemma) {
            return it->second;
        }
        return {};
    }

    return loadPostingsFromFile(lemma);
}

PostingList BooleanIndex::loadPostingsFromFile(const std::string& lemma) const {
    std::ifstream in(index_file_path_);
    if (!in.is_open()) {
        return {};
    }

    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::istringstream iss(line);
        std::string cur;
        iss >> cur;

        if (cur == lemma) {
            PostingList postings;
            DocId id;
            while (iss >> id) postings.push_back(id);
            return postings;
        }
        if (cur > lemma) break;
    }
    return {};
}

PostingList BooleanIndex::intersect(const PostingList& a, const PostingList& b) {
    PostingList res;
    res.reserve(std::min(a.size(), b.size()));

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) { res.push_back(a[i]); ++i; ++j; }
        else if (a[i] < b[j]) ++i;
        else ++j;
    }
    return res;
}

PostingList BooleanIndex::unify(const PostingList& a, const PostingList& b) {
    PostingList res;
    res.reserve(a.size() + b.size());

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) { res.push_back(a[i]); ++i; ++j; }
        else if (a[i] < b[j]) { res.push_back(a[i]); ++i; }
        else { res.push_back(b[j]); ++j; }
    }
    while (i < a.size()) res.push_back(a[i++]);
    while (j < b.size()) res.push_back(b[j++]);
    return res;
}

std::vector<DocId> BooleanIndex::andQuery(const std::vector<std::string>& terms) const {
    if (terms.empty()) return {};

    std::vector<std::string> lemmas;
    lemmas.reserve(terms.size());
    for (const auto& t : terms) lemmas.push_back(lem_.lemmatizeToken(t));

    PostingList tmp;
    bool first = true;

    for (const auto& lemma : lemmas) {
        PostingList pl = getPostings(lemma);
        if (pl.empty()) return {};

        if (first) { tmp = std::move(pl); first = false; }
        else { tmp = intersect(tmp, pl); }

        if (tmp.empty()) return {};
    }
    return tmp;
}

std::vector<DocId> BooleanIndex::orQuery(const std::vector<std::string>& terms) const {
    if (terms.empty()) return {};

    std::vector<std::string> lemmas;
    lemmas.reserve(terms.size());
    for (const auto& t : terms) lemmas.push_back(lem_.lemmatizeToken(t));

    PostingList result;
    bool hasAny = false;

    for (const auto& lemma : lemmas) {
        PostingList pl = getPostings(lemma);
        if (pl.empty()) continue;

        if (!hasAny) { result = std::move(pl); hasAny = true; }
        else { result = unify(result, pl); }
    }
    return hasAny ? result : PostingList{};
}

}
