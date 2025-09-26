#include <fstream>

#include "litedb/json.hpp"

namespace litedb::json {


void print_json(const std::string& jsonInput) {
    try {
        bsoncxx::document::value doc = bsoncxx::from_json(jsonInput);
        auto view = doc.view();

        // save in
        std::ofstream out("data.bson", std::ios::binary);
        if (!out) {
            std::cerr << "Cannot open file for writing\n";
        }
        out.write(reinterpret_cast<const char*>(view.data()), view.length());
        out.close();
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
