#pragma once

#include <string>

struct StoredDocument {
    std::string title;
    std::string description;
    std::string url;
    std::string mongo_doc_id;
};