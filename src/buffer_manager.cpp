#include "litedb/buffer_manager.hpp"

namespace litedb::buffer_manager {


BufferManager::BufferManager() {
    max_batch = 1000;
    try {
        root.forceRead(0);
        uint32_t page_id = root.getLeftMostChild();
        free_pages_.push_front(page_id);
    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("BufferManager initialization failed: ") + e.what());
    }
}

BufferManager::~BufferManager() {
    std::unique_lock lock(mtx_);
    try {
        root.write();
    } catch (const std::exception& e) {
        std::cerr << "[BufferManager] Failed to write root-page: " << e.what() << "\n";
    }
    for (uint32_t i = 0; i < litedb::constants::BUFFER_MANAGER_SIZE; ++i) {
        try {
            pages_[i].write();
        } catch (const std::exception& e) {
            std::cerr << "[BufferManager] Failed to write buffer page-" << i << ": " << e.what() << "\n";
        }
    }
    try {
        free_files_list_write();
    } catch (const std::exception& e) {
        std::cerr << "[BufferManager] Failed to write free-pages-list: " << e.what() << "\n";
    }
}

litedb::page::Page* BufferManager::getRoot() {
    return &root;
}

litedb::page::Page* BufferManager::getEmptyPage(size_t page_id) {
    auto page = &pages_[page_id % litedb::constants::BUFFER_MANAGER_SIZE];
    return page;
}

uint32_t BufferManager::getFreePage() {
    std::unique_lock lock(mtx_);
    if (free_pages_.size() == 1) {
        if (free_pages_.front() == 0) {
            return litedb::g::pages_count++;
        } else {
            free_files_list_read();
        }
    }
    uint32_t id = free_pages_.front();
    free_pages_.pop_front();
    root.setLeftMostChild(free_pages_.front());
    return id;
}

void BufferManager::addFreePage(uint32_t page_id) {
    std::unique_lock lock(mtx_);
    free_pages_.push_front(page_id);
    root.setLeftMostChild(page_id);
    if (free_pages_.size() >= max_batch - 10) {
        free_files_list_write();
    }
}

void BufferManager::free_files_list_read() {
    uint32_t curr = free_pages_.back();
    while (curr != 0 && free_pages_.size() < max_batch) {
        uint32_t next;
        try {
            next = litedb::page::io::read_align<uint32_t>(curr, litedb::page::NEXT_PAGE_OFFSET);
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string("Failed to read free page chain: ") + e.what());
        }
        free_pages_.push_back(next);
        curr = next;
    }
}

void BufferManager::free_files_list_write() {
    if (free_pages_.size() <= 1) {
        return;
    }
    while (free_pages_.size() > 1) {
        uint32_t next = free_pages_.back();
        free_pages_.pop_back();
        uint32_t curr = free_pages_.back();
        try {
            litedb::page::io::write_align<uint32_t>(curr, litedb::page::NEXT_PAGE_OFFSET, next);
        } catch (const std::exception& e) {
            throw std::runtime_error(
                std::string("Failed to write free page ") + std::to_string(curr) + ": " + e.what()
            );
        }
    }
}


}
