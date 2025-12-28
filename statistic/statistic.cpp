#include "statistic.hpp"

namespace search {

double Stat::avg_token_length(const std::vector<std::string>& tokens) const {
    if (tokens.empty()) return 0.0;

    std::size_t len_sum = 0;
    for (const auto& token : tokens) {
        len_sum += token.size();
    }
    return static_cast<double>(len_sum) / static_cast<double>(tokens.size());
}

void Stat::dumpLemmasFromMongo(MongoClient& mongo,
                              const std::string& collection_name,
                              const std::string& out_path) const
{
    auto coll = mongo.collection(collection_name);
    mongocxx::cursor cursor = coll.find({});

    std::ofstream out(out_path);
    if (!out.is_open()) return;

    Tokenizer tok;
    Lematization lem;

    for (auto&& doc : cursor) {
        bsoncxx::document::view view = doc;

        std::string title = get_string_field(view, "title");
        std::string description = get_string_field(view, "description");

        std::string text = title + " " + description;

        auto tokens = tok.tokenize(text);
        auto lemmas = lem.lemmatizeTokens(tokens);

        // ВАЖНО: для закона Ципфа пишем ВСЕ вхождения, не unique
        for (const auto& l : lemmas) {
            if (!l.empty()) out << l << "\n";
        }
    }
}

} // namespace search
