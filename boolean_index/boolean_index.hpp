#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

#include "../lematization/lematization.hpp" 
#include "../tokenizer.hpp"  // твой класс лемматизации
// tokenize() объяви где-то отдельно и подключи нужный заголовок
// std::vector<std::string> tokenize(const std::string& text);

namespace search {

using DocId = std::uint32_t;
using PostingList = std::vector<DocId>;

class BooleanIndex {
public:
    BooleanIndex() = default;

    // Добавить документ по его внутреннему DocId и тексту (title + description)
    void addDocument(DocId id, const std::string& text);

    // Завершить построение индекса (отсортировать списки, удалить дубликаты)
    void finalize();

    // Получить postings-list для леммы (nullptr, если терм не встречался)
    const PostingList* getPostings(const std::string& lemma) const;

    // Простейший AND-запрос: все термины должны встречаться в документе
    std::vector<DocId> andQuery(const std::vector<std::string>& terms) const;

    // Простейший OR-запрос: хотя бы один термин встречается в документе
    std::vector<DocId> orQuery(const std::vector<std::string>& terms) const;

private:
    std::unordered_map<std::string, PostingList> index_;
    Lematization lem_;
    Tokenizer tok_;

    static PostingList intersect(const PostingList& a, const PostingList& b);
    static PostingList unify(const PostingList& a, const PostingList& b);
};

} // namespace search
