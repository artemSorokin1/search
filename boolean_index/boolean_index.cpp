#include "boolean_index.hpp"
#include <algorithm>
#include <unordered_set>

namespace search {

void BooleanIndex::addDocument(DocId id, const std::string& text) {
    // 1. Токенизация
    auto tokens = tok_.tokenize(text);

    // 2. Лемматизация
    auto lemmas = lem_.lemmatizeTokens(tokens);

    // 3. Для булева индекса достаточно факта наличия терма в документе,
    //    поэтому внутри документа делаем множество уникальных лемм.
    std::unordered_set<std::string> uniqueLemma(lemmas.begin(), lemmas.end());

    // 4. Добавляем DocId в postings-list каждой леммы
    for (const auto& l : uniqueLemma) {
        index_[l].push_back(id);
    }
}

void BooleanIndex::finalize() {
    // Для каждой леммы:
    // - сортируем список docId
    // - удаляем возможные дубликаты (на всякий случай)
    for (auto& [lemma, postings] : index_) {
        std::sort(postings.begin(), postings.end());
        postings.erase(std::unique(postings.begin(), postings.end()), postings.end());
    }
}

const PostingList* BooleanIndex::getPostings(const std::string& lemma) const {
    auto it = index_.find(lemma);
    if (it == index_.end()) return nullptr;
    return &it->second;
}

// Вспомогательная функция: пересечение двух отсортированных списков
PostingList BooleanIndex::intersect(const PostingList& a, const PostingList& b) {
    PostingList res;
    res.reserve(std::min(a.size(), b.size()));

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            res.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    return res;
}

// Вспомогательная функция: объединение двух отсортированных списков
PostingList BooleanIndex::unify(const PostingList& a, const PostingList& b) {
    PostingList res;
    res.reserve(a.size() + b.size());

    std::size_t i = 0, j = 0;
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            res.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            res.push_back(a[i]);
            ++i;
        } else {
            res.push_back(b[j]);
            ++j;
        }
    }

    while (i < a.size()) {
        res.push_back(a[i++]);
    }
    while (j < b.size()) {
        res.push_back(b[j++]);
    }
    return res;
}

// Простейший AND-запрос: все термины должны присутствовать в документе
std::vector<DocId> BooleanIndex::andQuery(const std::vector<std::string>& terms) const {
    if (terms.empty()) return {};

    // 1. Лемматизируем термы запроса
    std::vector<std::string> lemmas;
    lemmas.reserve(terms.size());
    for (const auto& t : terms) {
        lemmas.push_back(lem_.lemmatizeToken(t));
    }

    // 2. Берём postings первого терма как старт
    const PostingList* current = nullptr;
    PostingList tmpResult;

    for (const auto& lemma : lemmas) {
        const PostingList* pl = getPostings(lemma);
        if (!pl) {
            // Если хотя бы один терм не встречается вообще,
            // результат AND-поиска пустой
            return {};
        }

        if (!current) {
            current = pl;
            tmpResult = *pl; // копия
        } else {
            tmpResult = intersect(tmpResult, *pl);
        }

        if (tmpResult.empty()) {
            return {};
        }
    }

    return tmpResult;
}

// Простейший OR-запрос: хотя бы один терм встречается в документе
std::vector<DocId> BooleanIndex::orQuery(const std::vector<std::string>& terms) const {
    if (terms.empty()) return {};

    std::vector<std::string> lemmas;
    lemmas.reserve(terms.size());
    for (const auto& t : terms) {
        lemmas.push_back(lem_.lemmatizeToken(t));
    }

    PostingList result;

    bool hasAny = false;
    for (const auto& lemma : lemmas) {
        const PostingList* pl = getPostings(lemma);
        if (!pl) continue;

        if (!hasAny) {
            result = *pl;
            hasAny = true;
        } else {
            result = unify(result, *pl);
        }
    }

    if (!hasAny) return {};
    return result;
}

} // namespace search
