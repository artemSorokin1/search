#include "tokenizer.hpp"

using namespace std;

namespace search {

vector<string> Tokenizer::tokenize(const string & s) {
    string tmp = to_lower_utf8(s);

    tmp = normalize(tmp);

    vector<std::string> tokens;
    istringstream iss(tmp);
    string token;
    while (iss >> token) {
        if (token.size() >= 2) {
            tokens.push_back(token);
        }
    }

    return tokens;
}

std::string Tokenizer::to_lower_utf8(const std::string& s) {
    std::string res;
    res.reserve(s.size());

    for (size_t i = 0; i < s.size();) {
        unsigned char c = s[i];

        if (c < 128) {
            res.push_back(std::tolower(c));
            i++;
            continue;
        }

        if ((c == 0xD0 || c == 0xD1) && i + 1 < s.size()) {
            unsigned char c2 = s[i + 1];

            if (c == 0xD0 && c2 >= 0x90 && c2 <= 0x9F) {     
                res.push_back(0xD0);
                res.push_back(c2 + 0x20);
            }
            else if (c == 0xD0 && c2 >= 0xA0 && c2 <= 0xAF) {
                res.push_back(0xD1);
                res.push_back(c2 - 0x20);
            }
            else if (c == 0xD0 && c2 == 0x81) { 
                res.push_back(0xD1);
                res.push_back(0x91);
            }
            else {
                res.push_back(c);
                res.push_back(c2);
            }

            i += 2;
            continue;
        }

        res.push_back(c);
        i++;
    }

    return res;
}


string Tokenizer::normalize(const string & s) {
    string res = s;
    for (char &ch : res) {
        unsigned char c = static_cast<unsigned char>(ch);
        if (c < 128) {
            if (!std::isalnum(c) && c != '-') {
                ch = ' ';
            }
        }
    }
    return res;
}

}
