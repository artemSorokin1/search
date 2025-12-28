#pragma once

#include <string>
#include <vector>

namespace search {

class Lematization {
public:
    Lematization() = default;

    std::string lemmatizeToken(const std::string& token) const;

    std::vector<std::string> lemmatizeTokens(const std::vector<std::string>& tokens) const;

private:
    bool isLatin(const std::string& s) const;
    std::string stemEnglish(const std::string& w) const;
    std::string stemRussian(const std::string& w) const;
};

}
