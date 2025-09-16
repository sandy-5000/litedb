#include "litedb/buffer_manager.hpp"

namespace litedb::buffer_manager {


BufferManager::BufferManager() {
    // std::cout << "Buffer Manager created" << std::endl;
}

litedb::page::Page* BufferManager::getPage(size_t page_id) {
    auto page = &pages_[page_id % litedb::constants::BUFFER_MANAGER_SIZE];
    int32_t byte_count = page->read(page_id);
    return page;
}

}
