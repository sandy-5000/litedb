#include <cstdint>

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <list>
#include <memory>

#include "litedb/config.hpp"
#include "litedb/engine/store.hpp"
#include "litedb/engine/root_manager.hpp"
#include "litedb/engine/buffer_manager.hpp"
#include "litedb/table/data_types.hpp"
#include "litedb/table/compare.hpp"
#include "litedb/table/find.hpp"
#include "litedb/table/operations.hpp"
#include "litedb/table/utils.hpp"


void print_key(std::vector<uint8_t> &key) {
    for (auto i : key) {
        printf("%02X ", i);
    }
    printf("\n");
}

void append_double(std::vector<uint8_t> &key, double val) {
    key.push_back(litedb::table::TYPE_f64);
    size_t old_size = key.size();
    key.resize(old_size + sizeof(double));
    std::memcpy(key.data() + old_size, &val, sizeof(double));
}

void append_string(std::vector<uint8_t> &key, const std::string &str) {
    uint16_t str_size = static_cast<uint16_t>(str.size());
    key.push_back(litedb::table::TYPE_str);
    uint16_t old_size = key.size();
    key.resize(old_size + str.size());
    std::memcpy(key.data() + old_size, str.data(), str.size());
    key.push_back(0x00);
}

void append_bool(std::vector<uint8_t> &key, bool val) {
    key.push_back(litedb::table::TYPE_u8);
    key.push_back(val ? 0x01 : 0x00);
}

void append_int32(std::vector<uint8_t> &key, int32_t val) {
    key.push_back(litedb::table::TYPE_i32);
    size_t old_size = key.size();
    key.resize(old_size + sizeof(int32_t));
    std::memcpy(key.data() + old_size, &val, sizeof(int32_t));
}

void append_int64(std::vector<uint8_t> &key, int64_t val) {
    key.push_back(litedb::table::TYPE_i64);
    size_t old_size = key.size();
    key.resize(old_size + sizeof(int64_t));
    std::memcpy(key.data() + old_size, &val, sizeof(int64_t));
}

void append_key_type(std::vector<uint8_t> &key, uint8_t type, uint32_t size) {
    key.push_back(type);
    type &= 0x7f;
    if (type == 0x00 || type == 0x05) {
        key.resize(key.size() + size);
    } else if (type == 0x01) {
        key.resize(key.size() + 2);
    } else if (type == 0x02 || type == 0x06) {
        key.resize(key.size() + 4);
    } else if (type == 0x03) {
        key.resize(key.size() + 8);
    }
}

void finsh_key(std::vector<uint8_t> &key) {
    key.push_back(0x00);
    uint16_t total_size = static_cast<uint16_t>(key.size());
    std::memcpy(key.data(), &total_size, sizeof(uint16_t));
    litedb::table::utils::print_key(reinterpret_cast<uint8_t *>(key.data()));
}



void compare_test() {
    using namespace litedb::table;

    int test_no = 0;

    // --- Test 1: identical strings ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x02, 0);

        append_string(key_a, "sandy-blaze");
        append_string(key_b, "sandy-blaze");

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (identical strings): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 2: different strings ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x03, 0);
        append_key_type(key_b, 0x02, 0);

        append_string(key_a, "abc");
        append_string(key_b, "xyz");

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (different strings): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 3: identical int32 ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x04, 0);
        append_key_type(key_b, 0x02, 0);

        append_int32(key_a, 42);
        append_int32(key_b, 42);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (identical int32): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 4: different int32 ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x02, 0);

        append_int32(key_a, 42);
        append_int32(key_b, 100);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (different int32): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 5: identical double ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x02, 0);
        append_key_type(key_b, 0x02, 0);

        append_double(key_a, 3.14159);
        append_double(key_b, 3.14159);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (identical double): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 6: different double ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x04, 0);
        append_key_type(key_b, 0x02, 0);

        append_double(key_a, 2.71828);
        append_double(key_b, 3.14159);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (different double): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 7: compound key (string + int32) identical ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x02, 0);

        append_string(key_a, "user1");
        append_int32(key_a, 42);

        append_string(key_b, "user1");
        append_int32(key_b, 42);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound string+int32 identical): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 8: compound key (string + int32) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x03, 0);
        append_key_type(key_b, 0x02, 0);

        append_string(key_a, "user1");
        append_int32(key_a, 42);

        append_string(key_b, "user1");
        append_int32(key_b, 43);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound string+int32 differs): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 9: compound key (double + int64) identical ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x04, 0);
        append_key_type(key_b, 0x02, 0);

        append_double(key_a, 1.23);
        append_int64(key_a, 9876543210);

        append_double(key_b, 1.23);
        append_int64(key_b, 9876543210);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound double+int64 identical): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 10: compound key (double + int64) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x02, 0);

        append_double(key_a, 1.23);
        append_int64(key_a, 9876543210);

        append_double(key_b, 2.34);
        append_int64(key_b, 9876543210);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound double+int64 differs): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 11: compound key (double + int64 + string) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x04, 0);
        append_key_type(key_b, 0x02, 0);

        append_double(key_a, 2.34);
        append_int64(key_a, 9876543210);
        append_string(key_a, "sandy-blaze");

        append_double(key_b, 2.34);
        append_int64(key_b, 9876543210);
        append_string(key_b, "sandy-blaze");

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound double+int64+string identical): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 12: compound key (double + int64 + string) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x82, 0);

        append_double(key_a, 2.34);
        append_int64(key_a, 9876543210);
        append_string(key_a, "sandy-blaze");

        append_double(key_b, 2.34);
        append_int64(key_b, 9876543210);
        append_string(key_b, "sandy-blaze");
        append_int64(key_b, 10);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), true);
        std::cout << "Test " << test_no << " (compound double+int64+string identical): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 13: compound key (double + int64 + string + string) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x82, 0);

        append_double(key_a, 2.34);
        append_int64(key_a, 9876543210);
        append_string(key_a, "sandy-blaze");
        append_string(key_a, "9000");

        append_double(key_b, 2.34);
        append_int64(key_b, 9876543210);
        append_string(key_b, "sandy-blaze");
        append_string(key_b, "9000");
        append_int64(key_b, 10);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), true);
        std::cout << "Test " << test_no << " (compound double+int64+string+string identical): " << static_cast<int>(res) << " (expected 0)\n";
    }

    // --- Test 14: compound key (double + int64 + string + string) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x82, 0);

        append_double(key_a, 2.34);
        append_int64(key_a, 9876543210);
        append_string(key_a, "sandy-blaze");
        append_string(key_a, "9000");

        append_double(key_b, 2.34);
        append_int64(key_b, 9876543210);
        append_string(key_b, "sandy-blaze");
        append_string(key_a, "9000");
        append_int64(key_b, 10);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound double+int64+string+string differ): " << static_cast<int>(res) << " (expected -1)\n";
    }

    // --- Test 15: compound key (mixed) differs ---
    {
        ++test_no;
        std::vector<uint8_t> key_a(2), key_b(2);
        append_key_type(key_a, 0x01, 0);
        append_key_type(key_b, 0x82, 0);

        append_double(key_a, 2.34);
        append_int32(key_a, 1000);
        append_string(key_a, "sandy-blaze");

        append_double(key_b, 2.34);
        append_int64(key_b, 10);
        append_int64(key_b, 10928734);

        finsh_key(key_a);
        finsh_key(key_b);

        int8_t res = compare::keys(key_a.data(), key_b.data(), false);
        std::cout << "Test " << test_no << " (compound mixed differ): " << static_cast<int>(res) << " (expected 1)\n";
    }
}

void test_page_allocations() {
    auto buffer = litedb::engine::buffer_manager_->get_main_buffer();

    int32_t no_of_pages = 100, to_free = 30, after_free = 10;
    std::list<uint32_t> pages_;

    for (int i = 0; i < no_of_pages; ++i) {
        uint32_t new_page_id = litedb::engine::root_manager_->get_free_page();
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(new_page_id);
        page->read_empty(new_page_id);
        page->header.record_count = 1;
        std::cout << new_page_id << std::endl;
        pages_.push_back(new_page_id);
    }

    for (int i = 1; i < to_free; ++i) {
        litedb::engine::root_manager_->add_free_page(pages_.front());
        pages_.pop_front();
    }

    std::cout << "Fetching pages after freeing" << std::endl;
    for (int i = 0; i < after_free; ++i) {
        uint32_t new_page_id = litedb::engine::root_manager_->get_free_page();
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(new_page_id);
        page->read_empty(new_page_id);
        page->header.record_count = 1;
        std::cout << new_page_id << std::endl;
    }
}

void create_tables() {
    std::string key = "table__";
    uint64_t success_cnt = 0, failed_cnt = 0;

    for (int i = 1; i <= 100000000; ++i) {
        auto nk = key + std::to_string(i);
        bool flag = litedb::table::root_table::create_table(nk);
        flag ? ++success_cnt : ++failed_cnt;
        if (flag && i % 1000000 == 0) {
            std::cout << "[CREATE_TABLE] " << nk << " success" << std::endl;
        }
    }

    auto root_manager = litedb::engine::root_manager_;
    uint32_t root_table_page = root_manager->page_data.root_table_page;

    std::cout << "\n[COMPLETED_INSERTS]\n";
    std::cout << "[SUCCESS]: " << success_cnt << " [FAILED]: " << failed_cnt << "\n\n";
    // litedb::table::utils::check_table(root_table_page);
}

int32_t main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "For storing needed a file path <file_name>.ldb" << std::endl;
        return 0;
    }

    std::string file_path = std::string(argv[1]);

    try {
        litedb::config::init_db_path(argv[1]);

        static litedb::engine::root_manager::RootManager rootManager;
        static litedb::engine::buffer_manager::BufferManager bufferManager;
        litedb::engine::root_manager_ = &rootManager;
        litedb::engine::buffer_manager_ = &bufferManager;

        litedb::config::print_hardware_config();
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    // compare_test();
    // test_page_allocations();
    // create_tables();

    for (int i = 4; i <= 4; ++i) {
        std::cout << "page: " << i << std::endl;
        auto buffer = litedb::engine::buffer_manager_->get_main_buffer();
        std::shared_ptr<litedb::page::Page> page = buffer->get_page(i);
        litedb::table::utils::print_slot_page(page);
    }

    return 0;
}