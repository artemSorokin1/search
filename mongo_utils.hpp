#pragma once

#include <string>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

namespace search {

inline std::string get_string_field(const bsoncxx::document::view& doc,
                                    const std::string& field_name)
{
    auto el = doc[field_name];
    if (!el) return {};
    if (el.type() != bsoncxx::type::k_string) return {};
    auto sv = el.get_string().value;
    return std::string{sv.data(), sv.size()};
}

} // namespace search
