#include "lematization.hpp"

namespace search {

bool Lematization::isLatin(const std::string& s) const {
    if (s.empty()) return false;
    for (unsigned char c : s) {
        if (c < 128) {
            // только латинские буквы
            if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
                return false;
            }
        } else {
            // есть не-ASCII байты -> не чисто английское слово
            return false;
        }
    }
    return true;
}

std::string Lematization::stemEnglish(const std::string& w) const {
    std::string s = w;

    // очень простой стеммер: убираем типичные окончания
    if (s.size() > 4 && s.substr(s.size() - 3) == "ing") {
        s.erase(s.size() - 3);
    } else if (s.size() > 3 && s.substr(s.size() - 2) == "ed") {
        s.erase(s.size() - 2);
    } else if (s.size() > 3 && s.back() == 's') {
        s.pop_back();
    }

    return s;
}

std::string Lematization::stemRussian(const std::string& w) const {
    std::string s = w;

    // Упрощённый список русских окончаний (UTF-8).
    // Важно: сначала более длинные.
    static const char* suffixes[] = {
        // 6–4 "буквенные" суффиксы (в байтах может быть больше из-за UTF-8)
        "остью", "остям", "остях",
        "иями", "ьими",
        "шего", "щего", "шему", "щему", "шим", "щим", "шая", "щая", "шее", "щее",

        // глагольные суффиксы (инф. + формы)
        "овать", "ывать", "ивать",
        "ать", "ять", "ять", "еть", "ить",
        "ешь", "ешься", "ете", "етесь", "ем", "ете",
        "лся", "лась", "лись", "лось",
        "ешь", "ют", "ут", "ит", "ат", "ят",

        // прилагательные (полные и краткие)
        "ыми", "ими",
        "ого", "его",
        "ому", "ему",
        "ыми", "ими",
        "ых", "их",
        "ый", "ий", "ой",
        "ая", "яя",
        "ое", "ее",
        "ые", "ие",

        // существительные (склонение, множ. число)
        "ами", "ями",
        "ов", "ев", "ёв",
        "ям", "ам",
        "ях", "ах",
        "ую", "ью",
        "ии", "ии",
        "ия", "ья",
        "ей", "ой", "ей",
        "ье", "ие",

        // короткие окончания (одна "буква", но несколько байт)
        "у", "ю",
        "е", "ё",
        "а", "я",
        "ы", "и",
        "о"
    };
    const size_t suffixCount = sizeof(suffixes) / sizeof(suffixes[0]);

    for (size_t i = 0; i < suffixCount; ++i) {
        const std::string suf = suffixes[i];
        if (s.size() > suf.size() + 1 &&      // чтобы не снижать слово до 1 буквы
            s.size() >= suf.size() &&
            s.compare(s.size() - suf.size(), suf.size(), suf) == 0) {
            s.erase(s.size() - suf.size());
            break;
        }
    }

    return s;
}

std::string Lematization::lemmatizeToken(const std::string& token) const {
    if (token.empty()) return token;

    // чистый английский -> английский стеммер
    if (isLatin(token)) {
        return stemEnglish(token);
    }

    // всё остальное (русский/смешанное) -> русский стеммер
    return stemRussian(token);
}

std::vector<std::string> Lematization::lemmatizeTokens(
    const std::vector<std::string>& tokens
) const {
    std::vector<std::string> result;
    result.reserve(tokens.size());
    for (const auto& t : tokens) {
        result.push_back(lemmatizeToken(t));
    }
    return result;
}

} 
