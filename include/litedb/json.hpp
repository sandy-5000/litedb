#pragma once

#include <iostream>
#include <string>
#include <bsoncxx/json.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/types.hpp>

#include "litedb/tables/operations.hpp"

using namespace bsoncxx;

namespace litedb::json {


void print_json(const std::string& jsonInput);

template <typename T>
void print_element(const T& element, const std::string& padding = "") {
    auto type = element.type();
    std::string key = std::string(element.key());

    std::cout << padding;
    if (!key.empty()) {
        std::cout << "\"" << key << "\": ";
    }

    switch (type) {
        case bsoncxx::type::k_double:
            std::cout << element.get_double() << "\n";
            break;
        case bsoncxx::type::k_string:
            std::cout << "\"" << std::string(element.get_string().value) << "\"\n";
            break;
        case bsoncxx::type::k_bool:
            std::cout << (element.get_bool() ? "true" : "false") << "\n";
            break;
        case bsoncxx::type::k_int32:
            std::cout << element.get_int32() << "\n";
            break;
        case bsoncxx::type::k_int64:
            std::cout << element.get_int64() << "\n";
            break;
        case bsoncxx::type::k_null:
            std::cout << "null\n";
            break;
        case bsoncxx::type::k_document: {
            std::cout << "{\n";
            for (auto sub_elem : element.get_document().value)
                print_element<bsoncxx::document::element>(sub_elem, padding + "  ");
            std::cout << padding << "}\n";
            break;
        }
        case bsoncxx::type::k_array: {
            std::cout << "[\n";
            for (auto item : element.get_array().value) {
                print_element<bsoncxx::array::element>(item, padding + "  ");
            }
            std::cout << padding << "]\n";
            break;
        }
        default:
            std::cout << "[unknown type]\n";
    }
}


}