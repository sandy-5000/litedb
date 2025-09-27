#include "litedb/engine/buffer_manager.hpp"

namespace litedb::buffer_manager {


BufferManager::BufferManager(size_t capacity) : capacity_(capacity) {
    cache_.setCapacity(capacity);
}

BufferManager::~BufferManager() {
    auto page_list = cache_.get_list();
    for (auto page : page_list) {
        try {
            page.second->write();
        } catch (const std::exception& e) {
            std::cerr << "[BufferManager] Failed to write buffer page-" << page.first << ": " << e.what() << "\n";
        }
    }
}

std::shared_ptr<litedb::page::Page> BufferManager::getPage(size_t page_id) {
    auto cachedPageOpt = cache_.get(page_id);
    if (cachedPageOpt) {
        return *cachedPageOpt;
    }
    std::shared_ptr<litedb::page::Page> p = std::make_shared<litedb::page::Page>();
    cache_.set(page_id, p);
    return p;
}


}
