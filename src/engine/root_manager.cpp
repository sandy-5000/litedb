#include "litedb/engine/root_manager.hpp"

#include <mutex>
#include <iostream>

namespace litedb::engine::root_manager {

RootManager::RootManager() {
    max_batch = 1000;
    try {
        root.force_read(0);
        std::memcpy(&page_data, root.data_ + constants::PAGE_HEADER_SIZE, sizeof(root_data));
        uint32_t page_id = page_data.top_free_page;
        free_pages_.push_front(page_id);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("[RootManger] initialization failed: ") + e.what());
    }
}

RootManager::~RootManager() {
    save_state();
}

void RootManager::save_state() {
    std::unique_lock lock(mtx_);
    try {
        std::memcpy(root.data_ + constants::PAGE_HEADER_SIZE, &page_data, sizeof(root_data));
        root.write();
    } catch (const std::exception& e) {
        std::cerr << "[RootManger] Failed to write root-page: " << e.what() << "\n";
    }
    try {
        free_files_list_write();
    } catch (const std::exception& e) {
        std::cerr << "[RootManger] Failed to write free-pages-list: " << e.what() << "\n";
    }
    std::cout << "[RootManger] state_saved" << std::endl;
}

void RootManager::free_files_list_read() {
    uint32_t curr = free_pages_.back();
    while (curr != 0 && free_pages_.size() < max_batch) {
        uint32_t next;
        try {
            next = page::io::read_align<uint32_t>(curr, page::NEXT_PAGE_OFFSET);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("[RootManger] Failed to read free page chain: ") + e.what());
        }
        free_pages_.push_back(next);
        curr = next;
    }
}

void RootManager::free_files_list_write() {
    if (free_pages_.size() <= 1) {
        return;
    }
    while (free_pages_.size() > 1) {
        uint32_t next = free_pages_.back();
        free_pages_.pop_back();
        uint32_t curr = free_pages_.back();
        try {
            page::io::write_align<uint32_t>(curr, page::NEXT_PAGE_OFFSET, next);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                std::string("[RootManger] Failed to write free page ") + std::to_string(curr) + ": " + e.what()
            );
        }
    }
}

litedb::page::Page* RootManager::get_root() {
    return &root;
}

uint32_t RootManager::get_free_page() {
    std::unique_lock lock(mtx_);
    if (free_pages_.size() == 1) {
        if (free_pages_.front() == 0) {
            return g::pages_count++;
        } else {
            free_files_list_read();
        }
    }
    uint32_t id = free_pages_.front();
    free_pages_.pop_front();
    page_data.top_free_page = free_pages_.front();
    return id;
}

void RootManager::add_free_page(uint32_t page_id) {
    std::unique_lock lock(mtx_);
    free_pages_.push_front(page_id);
    page_data.top_free_page = page_id;
    if (free_pages_.size() >= max_batch - 10) {
        free_files_list_write();
    }
}

}