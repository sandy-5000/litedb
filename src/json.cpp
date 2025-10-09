#include <fstream>

#include "litedb/json.hpp"

namespace litedb::json {


void print_json(const std::string& jsonInput) {
    try {
        bsoncxx::document::value doc = bsoncxx::from_json(jsonInput);
        auto view = doc.view();

        std::string type, table_name;

        if (auto elem = view["type"]; elem && elem.type() == bsoncxx::type::k_string) {
            type = std::string(elem.get_string().value);
            if (auto elem = view["table_name"]; elem && elem.type() == bsoncxx::type::k_string && type == "create") {
                std::cout << "call create" << std::endl;
                table_name = std::string(elem.get_string().value);
                litedb::table::utils::root_table::create_table(table_name);
            }
        }

        // save in
        // std::ofstream out("data.bson", std::ios::binary);
        // if (!out) {
        //     std::cerr << "Cannot open file for writing\n";
        // }
        // out.write(reinterpret_cast<const char*>(view.data()), view.length());
        // out.close();
        // ----------

        std::cout << "{\n";
        for (auto element : view) {
            print_element<bsoncxx::document::element>(element, "  ");
        }
        std::cout << "}\n";
    } catch (const std::exception &e) {
        std::cout << jsonInput << "\n";
        std::cerr << "Error parsing JSON: " << e.what() << "\n";
    }
}


}
