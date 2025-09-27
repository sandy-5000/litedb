#include "litedb/engine/buffer_manager.hpp"

namespace litedb::buffer_manager {


BufferManager::BufferManager() {}

BufferManager::~BufferManager() {
    for (uint32_t i = 0; i < litedb::constants::BUFFER_MANAGER_SIZE; ++i) {
        try {
            pages_[i].write();
        } catch (const std::exception& e) {
            std::cerr << "[BufferManager] Failed to write buffer page-" << i << ": " << e.what() << "\n";
        }
    }
}

litedb::page::Page* BufferManager::getEmptyPage(size_t page_id) {
    auto page = &pages_[page_id % litedb::constants::BUFFER_MANAGER_SIZE];
    return page;
}


}
