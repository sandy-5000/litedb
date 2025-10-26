#include <cstdint>
#include <list>
#include <memory>
#include <iostream>

#include "tests/page_allocation.hpp"
#include "litedb/page/page.hpp"
#include "litedb/engine/store.hpp"

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
