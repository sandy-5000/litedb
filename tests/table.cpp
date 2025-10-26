#include <cstdint>
#include <iostream>

#include "tests/table.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/table/utils.hpp"
#include "litedb/table/operations.hpp"

void check_root_table(bool rev) {
    auto root_manager = litedb::engine::root_manager_;
    auto root_page = root_manager->get_root();

    root_manager->lock_unique();
    uint32_t root_table_page = root_manager->page_data.root_table_page;
    root_manager->unlock_unique();

    uint32_t page_count = litedb::g::pages_count;
    std::cout << "\n[ROOT_PAGE]: " << root_table_page << ", [PAGE_COUNT]: " << page_count << std::endl;

    litedb::table::utils::check_tree_links(root_table_page, page_count, rev);
}

void create_tables() {
    std::cout << "\n=========== [STARTED_INSERTS] ===========\n";

    std::string key = "table__";
    uint64_t success_cnt = 0, failed_cnt = 0;

    for (int i = 1; i <= 10000000; ++i) {
        auto nk = key + std::to_string(i);
        bool flag = litedb::table::root_table::create_table(nk);
        flag ? ++success_cnt : ++failed_cnt;
        if (i % 1000000 == 0) {
            std::cout << "[CREATE_TABLE] " << nk << " completed" << std::endl;
        }
    }

    std::cout << "\n[SUCCESS]: " << success_cnt << " [FAILED]: " << failed_cnt << "\n";
    std::cout << "========== [COMPLETED_INSERTS] ==========\n";
}

void find_tables() {
    std::cout << "\n=========== [STARTED_FINDS] ===========\n";

    std::string key = "table__";
    uint64_t success_cnt = 0, failed_cnt = 0, not_found_cnt = 0;

    for (int i = 1; i <= 10000000; ++i) {
        auto nk = key + std::to_string(i);
        std::string data = litedb::table::root_table::find_table(nk);
        if (data.size()) {
            uint64_t seq_number;
            std::memcpy(&seq_number, data.data() + 6 + nk.size(), sizeof(uint64_t));
            std::string table_name(data.data() + 4, data.data() + 4 + nk.size());
            if (seq_number + 1 == i && nk == table_name) {
                ++success_cnt;
            } else {
                ++not_found_cnt;
                // std::cout << "[FIND_TABLE] " << nk << " failed" << std::endl;
            }
        } else {
            ++failed_cnt;
            std::cout << "[FIND_TABLE] " << nk << " failed" << std::endl;
        }
        if (i % 1000000 == 0) {
            std::cout << "[FIND_TABLE] " << nk << " completed" << std::endl;
        }
    }

    std::cout << "\n[SUCCESS]: " << success_cnt << " [FAILED]: " << failed_cnt << " [NOT_FOUND]: " << not_found_cnt << "\n";
    std::cout << "========== [COMPLETED_FINDS] ==========\n";
}

void delete_tables() {
    std::cout << "\n=========== [STARTED_DELETES] ===========\n";

    std::string key = "table__";
    uint64_t success_cnt = 0, failed_cnt = 0;

    for (int i = 1; i <= 10000000; ++i) {
        auto nk = key + std::to_string(i);
        bool flag = litedb::table::root_table::drop_table(nk);
        flag ? ++success_cnt : ++failed_cnt;
        if (i % 1000000 == 0) {
            std::cout << "[DROP_TABLE] " << nk << " completed" << std::endl;
        }
        if (!flag) {
            std::cout << "[DROP_TABLE] *" << nk << " failed" << std::endl;
        }
    }

    std::cout << "\n[SUCCESS]: " << success_cnt << " [FAILED]: " << failed_cnt << "\n";
    std::cout << "========== [COMPLETED_DELETES] ==========\n";
}
