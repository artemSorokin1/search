#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

using namespace std;

namespace search {

    class Tokenizer {
    public:
        vector<string> tokenize(const string & s);
        string to_lower_utf8(const string& s);
        string normalize(const string& s);

    };  

}
